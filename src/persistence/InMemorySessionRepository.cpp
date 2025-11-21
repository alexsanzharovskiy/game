#include "persistence/InMemorySessionRepository.h"

Session InMemorySessionRepository::Create(const Session& s) {
    Session copy = s;
    copy.internalId = nextId_++;
    byId_[copy.internalId] = copy;
    byExternal_[copy.externalSessionId] = copy.internalId;
    if (copy.isActive) {
        activeByPlayerOperator_[copy.playerId + "#" + std::to_string(copy.operatorId)] = copy.internalId;
    }
    return copy;
}

std::optional<Session> InMemorySessionRepository::FindById(std::uint64_t id) {
    auto it = byId_.find(id);
    if (it == byId_.end()) return std::nullopt;
    return it->second;
}

std::optional<Session> InMemorySessionRepository::FindByExternalId(const std::string& externalId) {
    auto it = byExternal_.find(externalId);
    if (it == byExternal_.end()) return std::nullopt;
    return FindById(it->second);
}

std::optional<Session> InMemorySessionRepository::FindActiveByPlayerAndOperator(const std::string& playerId,
                                                                                std::int64_t operatorId) {
    auto it = activeByPlayerOperator_.find(playerId + "#" + std::to_string(operatorId));
    if (it == activeByPlayerOperator_.end()) return std::nullopt;
    return FindById(it->second);
}

void InMemorySessionRepository::Update(const Session& s) {
    byId_[s.internalId] = s;
    byExternal_[s.externalSessionId] = s.internalId;
    auto key = s.playerId + "#" + std::to_string(s.operatorId);
    if (s.isActive) {
        activeByPlayerOperator_[key] = s.internalId;
    } else {
        activeByPlayerOperator_.erase(key);
    }
}
