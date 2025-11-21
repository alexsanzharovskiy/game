#pragma once
#include <string>
#include <optional>
#include <cstdint>

struct Session {
    std::uint64_t internalId{0};
    std::string externalSessionId;
    std::string token;
    std::string playerId;
    std::int64_t operatorId{0};
    double balance{0.0};
    std::optional<std::string> lastRoundId;
    bool isActive{true};
};
