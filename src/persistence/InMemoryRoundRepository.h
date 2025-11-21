#pragma once
#include "persistence/RoundRepository.h"
#include <unordered_map>

class InMemoryRoundRepository : public RoundRepository {
public:
    void Save(const RoundResult& result) override;
    std::optional<RoundResult> FindById(const std::string& roundId) override;
    std::optional<RoundResult> FindLastBySession(std::uint64_t sessionId) override;
    void Update(const RoundResult& result) override;
    std::optional<RoundResult> FindUnfinishedBySession(std::uint64_t sessionId) override;
    std::optional<RoundResult> FindUnfinishedByPlayer(const std::string& playerId,
                                                      std::int64_t operatorId) override; // 🔥

private:
    std::unordered_map<std::string, RoundResult> rounds_;
    std::unordered_map<std::uint64_t, std::string> lastBySession_;
};
