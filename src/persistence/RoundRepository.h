#pragma once
#include "core/model/Round.h"
#include <optional>
#include <string>
#include <vector>

class RoundRepository {
public:
    virtual ~RoundRepository() = default;
    virtual void Save(const RoundResult& result) = 0;
    virtual std::optional<RoundResult> FindById(const std::string& roundId) = 0;
    virtual std::optional<RoundResult> FindLastBySession(std::uint64_t sessionId) = 0;
    virtual void Update(const RoundResult& result) = 0;
    virtual std::optional<RoundResult> FindUnfinishedBySession(std::uint64_t sessionId) = 0;

    // 🔥 поиск незавершённого раунда по игроку и оператору
    virtual std::optional<RoundResult> FindUnfinishedByPlayer(const std::string& playerId,
                                                              std::int64_t operatorId) = 0;
};
