#pragma once
#include <string>
#include <cstdint>
#include <optional>

enum class TransactionType {
    BET,
    WIN,
    CANCEL
};

enum class TransactionStatus {
    CREATED,
    PENDING,
    SUCCESS,
    FAILED
};

struct Transaction {
    std::string id;
    TransactionType type{TransactionType::BET};
    TransactionStatus status{TransactionStatus::PENDING};
    double amount{0.0};

    std::string roundId;
    std::uint64_t sessionId{0};
    std::string playerId;
    std::int64_t operatorId{0};

    // Внешний ID/ошибка от агрегатора
    std::string externalError;
};
