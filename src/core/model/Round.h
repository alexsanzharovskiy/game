#pragma once
#include <string>
#include <cstdint>

enum class RoundStatus {
    BET_FINISHED,   // BET прошёл, win рассчитан, WIN ещё не отправлялся
    WIN_PENDING,    // WIN отправляли, но не получили успешного подтверждения (можно ретраить)
    COMPLETED,      // раунд полностью завершён (WIN проведён, баланс обновлён)
    CANCELLED       // ставка не прошла / раунд отменён
};

struct RoundResult {
    std::string roundId;
    std::string playerId;   // для удобства локального хранения
    std::string currency;   // валюта (например, "USD", "EUR", "USDT")
    std::string reelsJson;  // JSON с барабанами/символами (пока можно оставлять пустым)

    std::uint64_t sessionId{0};
    std::int64_t operatorId{0};
    double betAmount{0.0};
    double winAmount{0.0};
    RoundStatus status{RoundStatus::BET_FINISHED};
};
