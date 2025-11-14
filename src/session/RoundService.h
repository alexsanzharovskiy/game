#pragma once
#include "core/math/MathEngine.h"
#include "persistence/RoundRepository.h"
#include "integration/IAggregatorClient.h"
#include "session/SessionManager.h"
#include <optional>

class RoundService {
public:
    RoundService(MathEngine& mathEngine,
                 RoundRepository& roundRepo,
                 SessionManager& sessionManager,
                 IAggregatorClient& aggregator);

    RoundResult PlayRound(std::uint64_t sessionInternalId, double betAmount);
    std::optional<RoundResult> ResumeUnfinishedRound(std::uint64_t sessionInternalId);

private:
    MathEngine& mathEngine_;
    RoundRepository& roundRepo_;
    SessionManager& sessionManager_;
    IAggregatorClient& aggregator_;

    std::string GenerateId(const std::string& prefix, std::uint64_t sessionId);
};
