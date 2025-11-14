#pragma once
#include <string>
#include <cstdint>

enum class RoundStatus {
    BET_CONFIRMED,
    WIN_PENDING,
    COMPLETED,
    CANCELLED
};

struct RoundResult {
    std::string roundId;
    std::string betTxId;
    std::string winTxId;

    std::uint64_t sessionId{0};
    double betAmount{0.0};
    double winAmount{0.0};
    RoundStatus status{RoundStatus::BET_CONFIRMED};
};
