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

std::optional<RoundResult> RoundService::ResumeUnfinishedRound(std::uint64_t sessionInternalId) {
    auto unfinished = roundRepo_.FindUnfinishedBySession(sessionInternalId);
    if (!unfinished) return std::nullopt;

    auto sessionOpt = sessionManager_.GetSession(sessionInternalId);
    if (!sessionOpt) {
        util::Logger::Warn("ResumeUnfinishedRound: session not found");
        return std::nullopt;
    }
    auto session = *sessionOpt;

    RoundResult round = *unfinished;

    if (round.status == RoundStatus::BET_CONFIRMED || round.status == RoundStatus::WIN_PENDING) {
        if (round.winAmount > 0.0 && round.winTxId.empty()) {
            round.winTxId = GenerateId("win", round.sessionId);
        }

        if (round.winAmount > 0.0) {
            WinRequest wreq;
            wreq.roundId = round.roundId;
            wreq.winTxId = round.winTxId;
            wreq.amount = round.winAmount;
            wreq.sessionId = session.externalSessionId;
            wreq.playerId = session.playerId;

            auto winResp = aggregator_.Win(wreq);
            if (!winResp || winResp->status != TxStatus::SUCCESS) {
                round.status = RoundStatus::WIN_PENDING;
                roundRepo_.Update(round);
                util::Logger::Warn("ResumeUnfinishedRound: WIN still pending for round " + round.roundId);
                return round;
            }
            sessionManager_.UpdateBalance(session.internalId, winResp->newBalance);
        }

        round.status = RoundStatus::COMPLETED;
        roundRepo_.Update(round);
        sessionManager_.ClearLastRound(session.internalId);
        util::Logger::Info("ResumeUnfinishedRound: completed round " + round.roundId);
    }

    return round;
}

RoundResult RoundService::PlayRound(std::uint64_t sessionInternalId, double betAmount) {
    auto sessionOpt = sessionManager_.GetSession(sessionInternalId);
    if (!sessionOpt) {
        throw std::runtime_error("Session not found");
    }
    auto session = *sessionOpt;
    if (!session.isActive) {
        throw std::runtime_error("Session is not active");
    }

    auto unfinished = roundRepo_.FindUnfinishedBySession(session.internalId);
    if (unfinished) {
        throw std::runtime_error("Unfinished round exists; complete it before starting a new one");
    }

    std::string roundId = GenerateId("round", session.internalId);
    std::string betTxId = GenerateId("bet", session.internalId);

    DebitRequest dreq;
    dreq.roundId = roundId;
    dreq.betTxId = betTxId;
    dreq.amount = betAmount;
    dreq.sessionId = session.externalSessionId;
    dreq.playerId = session.playerId;

    auto debitResp = aggregator_.Debit(dreq);
    if (!debitResp || debitResp->status != TxStatus::SUCCESS) {
        util::Logger::Warn("PlayRound: debit failed for round " + roundId);
        throw std::runtime_error("Failed to debit bet amount: " +
                                 (debitResp ? debitResp->errorCode : "NO_RESPONSE"));
    }
    sessionManager_.UpdateBalance(session.internalId, debitResp->newBalance);

    double winAmount = mathEngine_.CalculateWin(betAmount);

    RoundResult result;
    result.roundId = roundId;
    result.betTxId = betTxId;
    result.sessionId = session.internalId;
    result.betAmount = betAmount;
    result.winAmount = winAmount;
    result.status = RoundStatus::BET_CONFIRMED;

    if (winAmount > 0.0) {
        result.winTxId = GenerateId("win", session.internalId);

        WinRequest wreq;
        wreq.roundId = roundId;
        wreq.winTxId = result.winTxId;
        wreq.amount = winAmount;
        wreq.sessionId = session.externalSessionId;
        wreq.playerId = session.playerId;

        auto winResp = aggregator_.Win(wreq);
        if (!winResp || winResp->status != TxStatus::SUCCESS) {
            result.status = RoundStatus::WIN_PENDING;
            roundRepo_.Save(result);
            sessionManager_.SetLastRound(session.internalId, result.roundId);
            util::Logger::Warn("PlayRound: WIN pending for round " + roundId);
            throw std::runtime_error("Win not confirmed: " +
                                     (winResp ? winResp->errorCode : "NO_RESPONSE"));
        }
        sessionManager_.UpdateBalance(session.internalId, winResp->newBalance);
    }

    result.status = RoundStatus::COMPLETED;
    roundRepo_.Save(result);
    sessionManager_.ClearLastRound(session.internalId);
    auto updatedSession = sessionManager_.GetSession(session.internalId);
    if (updatedSession) {
        util::Logger::Info("PlayRound completed. New balance: " + std::to_string(updatedSession->balance));
    }

    return result;
}
