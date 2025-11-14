#pragma once
#include "persistence/SessionRepository.h"
#include <unordered_map>

class InMemorySessionRepository : public SessionRepository {
public:
    Session Create(const Session& s) override;
    std::optional<Session> FindById(std::uint64_t id) override;
    std::optional<Session> FindByExternalId(const std::string& externalId) override;
    void Update(const Session& s) override;

private:
    std::unordered_map<std::uint64_t, Session> byId_;
    std::unordered_map<std::string, std::uint64_t> byExternal_;
    std::uint64_t nextId_{1};
};
