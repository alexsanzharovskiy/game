#include "persistence/InMemorySessionRepository.h"

Session InMemorySessionRepository::Create(const Session& s) {
    Session copy = s;
    copy.internalId = nextId_++;
    byId_[copy.internalId] = copy;
    byExternal_[copy.externalSessionId] = copy.internalId;
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

void InMemorySessionRepository::Update(const Session& s) {
    byId_[s.internalId] = s;
    byExternal_[s.externalSessionId] = s.internalId;
}
