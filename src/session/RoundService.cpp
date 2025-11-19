#include "session/RoundService.h"
#include "util/Logger.h"
#include <stdexcept>

RoundService::RoundService(MathEngine& mathEngine,
                           RoundRepository& roundRepo,
                           SessionManager& sessionManager,
                           IAggregatorClient& aggregator)
    : mathEngine_(mathEngine),
      roundRepo_(roundRepo),
      sessionManager_(sessionManager),
      aggregator_(aggregator) {}

std::string RoundService::GenerateId(const std::string& prefix, std::uint64_t sessionId) {
    static std::uint64_t counter = 1;
    return prefix + "_" + std::to_string(sessionId) + "_" + std::to_string(counter++);
}

// Шаг 1: /game/play
RoundResult RoundService::PlayRound(std::uint64_t sessionInternalId,
                                    double betAmount,
                                    const std::string& currency,
                                    const std::string& reelsJson) {
    auto sessionOpt = sessionManager_.GetSession(sessionInternalId);
    if (!sessionOpt) {
        throw std::runtime_error("Session not found");
    }
    auto session = *sessionOpt;
    if (!session.isActive) {
        throw std::runtime_error("Session is not active");
    }

    // Не пускаем новый раунд, если есть незавершённый
    auto unfinished = roundRepo_.FindUnfinishedBySession(session.internalId);
    if (unfinished) {
        throw std::runtime_error("Unfinished round exists; complete it before starting a new one");
    }

    // Генерируем идентификаторы
    std::string roundId = GenerateId("round", session.internalId);
    std::string betTxId = GenerateId("bet", session.internalId);

    // 1) BET (DEBIT) на агрегаторе
    DebitRequest dreq;
    dreq.roundId   = roundId;
    dreq.betTxId   = betTxId;
    dreq.amount    = betAmount;
    dreq.sessionId = session.externalSessionId;
    dreq.playerId  = session.playerId;

    auto debitResp = aggregator_.Debit(dreq);
    if (!debitResp || debitResp->status != TxStatus::SUCCESS) {
        util::Logger::Warn("PlayRound: debit failed for round " + roundId);
        throw std::runtime_error("Failed to debit bet amount: " +
                                 (debitResp ? debitResp->errorCode : "NO_RESPONSE"));
    }

    // Обновляем баланс по ставке
    sessionManager_.UpdateBalance(session.internalId, debitResp->newBalance);

    // 2) Генерируем результат раунда (winAmount) — без WIN
    double winAmount = mathEngine_.CalculateWin(betAmount);

    RoundResult result;
    result.roundId   = roundId;
    result.betTxId   = betTxId;
    result.winTxId   = ""; // пока не было WIN
    result.sessionId = session.internalId;
    result.playerId  = session.playerId;
    result.currency  = currency;
    result.reelsJson = reelsJson; // пока может быть пустой
    result.betAmount = betAmount;
    result.winAmount = winAmount;
    result.status    = RoundStatus::BET_CONFIRMED; // BET прошёл, WIN ещё не делали

    roundRepo_.Save(result);
    sessionManager_.SetLastRound(session.internalId, result.roundId);

    util::Logger::Info("PlayRound: created round " + roundId +
                       " bet=" + std::to_string(betAmount) +
                       " win=" + std::to_string(winAmount));

    // ВАЖНО: WIN ЕЩЁ НЕ ДЕЛАЛИ, баланс по win не изменён
    return result;
}

// Внутренняя общая логика для FinishRound и ResumeUnfinishedRound
RoundResult RoundService::DoFinishRound(std::uint64_t sessionInternalId, RoundResult round) {
    auto sessionOpt = sessionManager_.GetSession(sessionInternalId);
    if (!sessionOpt) {
        throw std::runtime_error("Session not found");
    }
    auto session = *sessionOpt;

    // Если уже COMPLETED или CANCELLED — ничего делать не нужно
    if (round.status == RoundStatus::COMPLETED || round.status == RoundStatus::CANCELLED) {
        return round;
    }

    // Если winAmount == 0, то WIN вообще не нужен — просто помечаем COMPLETED
    if (round.winAmount <= 0.0) {
        round.status = RoundStatus::COMPLETED;
        roundRepo_.Update(round);
        sessionManager_.ClearLastRound(session.internalId);
        util::Logger::Info("DoFinishRound: round " + round.roundId + " completed with zero win");
        return round;
    }

    // Нужен WIN: если ещё нет winTxId — генерируем
    if (round.winTxId.empty()) {
        round.winTxId = GenerateId("win", round.sessionId);
    }

    WinRequest wreq;
    wreq.roundId   = round.roundId;
    wreq.winTxId   = round.winTxId;
    wreq.amount    = round.winAmount;
    wreq.sessionId = session.externalSessionId;
    wreq.playerId  = session.playerId;

    auto winResp = aggregator_.Win(wreq);
    if (!winResp || winResp->status != TxStatus::SUCCESS) {
        round.status = RoundStatus::WIN_PENDING;
        roundRepo_.Update(round);
        util::Logger::Warn("DoFinishRound: WIN still pending for round " + round.roundId +
                           ", error=" + (winResp ? winResp->errorCode : "NO_RESPONSE"));
        return round; // остаётся незавершённым, можно будет ретраить
    }

    // WIN успешен
    sessionManager_.UpdateBalance(session.internalId, winResp->newBalance);
    round.status = RoundStatus::COMPLETED;
    roundRepo_.Update(round);
    sessionManager_.ClearLastRound(session.internalId);

    util::Logger::Info("DoFinishRound: round " + round.roundId +
                       " COMPLETED, win=" + std::to_string(round.winAmount) +
                       ", new balance=" + std::to_string(winResp->newBalance));
    return round;
}

// Шаг 2: /game/finish
RoundResult RoundService::FinishRound(const std::string& playerId,
                                      const std::string& roundId) {
    auto existing = roundRepo_.FindById(roundId);
    if (!existing) {
        throw std::runtime_error("Round not found");
    }

    RoundResult round = *existing;

    // Проверяем, что раунд реально принадлежит этому игроку
    if (round.playerId != playerId) {
        throw std::runtime_error("Round does not belong to this player");
    }

    // Если уже COMPLETED или CANCELLED — ничего делать не нужно
    if (round.status == RoundStatus::COMPLETED || round.status == RoundStatus::CANCELLED) {
        return round;
    }

    // Внутри RoundResult.sessionId лежит internalId сессии, в которой был BET
    std::uint64_t sessionInternalId = round.sessionId;

    return DoFinishRound(sessionInternalId, round);
}


// Автовосстановление при session_start
std::optional<RoundResult> RoundService::ResumeUnfinishedRound(std::uint64_t sessionInternalId) {
    auto unfinished = roundRepo_.FindUnfinishedBySession(sessionInternalId);
    if (!unfinished) return std::nullopt;

    RoundResult round = *unfinished;
    try {
        RoundResult finished = DoFinishRound(sessionInternalId, round);
        return finished;
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("ResumeUnfinishedRound error: ") + ex.what());
        return round; // возвращаем как есть
    }
}

std::optional<RoundResult> RoundService::FindUnfinishedRoundForPlayer(const std::string& playerId) {
    return roundRepo_.FindUnfinishedByPlayer(playerId);
}

