#pragma once
#include "core/math/MathEngine.h"
#include "persistence/RoundRepository.h"
#include "integration/IAggregatorClient.h"
#include "session/SessionManager.h"
#include <optional>
#include <string>

class RoundService {
public:
    RoundService(MathEngine& mathEngine,
                 RoundRepository& roundRepo,
                 SessionManager& sessionManager,
                 IAggregatorClient& aggregator);

    // Шаг 1 — /game/play: BET на агрегаторе + генерация раунда, БЕЗ WIN
    RoundResult PlayRound(std::uint64_t sessionInternalId,
                          double betAmount,
                          const std::string& currency,
                          const std::string& reelsJson = {});

    // Шаг 2 — /game/finish: WIN на агрегаторе по уже существующему round_id
RoundResult FinishRound(const std::string& playerId,
                        const std::string& roundId);

    // Автовосстановление незавершённого раунда при session_start
    std::optional<RoundResult> ResumeUnfinishedRound(std::uint64_t sessionInternalId);

    std::optional<RoundResult> FindUnfinishedRoundForPlayer(const std::string& playerId);


private:
    MathEngine& mathEngine_;
    RoundRepository& roundRepo_;
    SessionManager& sessionManager_;
    IAggregatorClient& aggregator_;

    std::string GenerateId(const std::string& prefix, std::uint64_t sessionId);
    RoundResult DoFinishRound(std::uint64_t sessionInternalId, RoundResult round);
};
