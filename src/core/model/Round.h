#pragma once
#include <string>
#include <cstdint>

enum class RoundStatus {
    BET_CONFIRMED,  // BET прошёл, раунд сгенерирован, WIN ещё не отправлялся/не подтверждён
    WIN_PENDING,    // WIN отправляли, но не получили успешного подтверждения (можно ретраить)
    COMPLETED,      // раунд полностью завершён (WIN проведён, баланс обновлён)
    CANCELLED       // ставка не прошла / раунд отменён
};

struct RoundResult {
    std::string roundId;
    std::string betTxId;    // id транзакции BET (debit)
    std::string winTxId;    // id транзакции WIN (credit), может быть пустым до WIN
    std::string playerId;   // для удобства локального хранения
    std::string currency;   // валюта (например, "USD", "EUR", "USDT")
    std::string reelsJson;  // JSON с барабанами/символами (пока можно оставлять пустым)

    std::uint64_t sessionId{0};
    double betAmount{0.0};
    double winAmount{0.0};
    RoundStatus status{RoundStatus::BET_CONFIRMED};
};
