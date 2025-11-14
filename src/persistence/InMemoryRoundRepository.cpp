#include "persistence/InMemoryRoundRepository.h"

void InMemoryRoundRepository::Save(const RoundResult& result) {
    rounds_[result.roundId] = result;
    lastBySession_[result.sessionId] = result.roundId;
}

std::optional<RoundResult> InMemoryRoundRepository::FindById(const std::string& roundId) {
    auto it = rounds_.find(roundId);
    if (it == rounds_.end()) return std::nullopt;
    return it->second;
}

std::optional<RoundResult> InMemoryRoundRepository::FindLastBySession(std::uint64_t sessionId) {
    auto it = lastBySession_.find(sessionId);
    if (it == lastBySession_.end()) return std::nullopt;
    return FindById(it->second);
}

void InMemoryRoundRepository::Update(const RoundResult& result) {
    rounds_[result.roundId] = result;
    lastBySession_[result.sessionId] = result.roundId;
}

std::optional<RoundResult> InMemoryRoundRepository::FindUnfinishedBySession(std::uint64_t sessionId) {
    auto last = FindLastBySession(sessionId);
    if (!last) return std::nullopt;
    if (last->status == RoundStatus::COMPLETED || last->status == RoundStatus::CANCELLED) {
        return std::nullopt;
    }
    return last;
}
