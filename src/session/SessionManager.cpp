#include "session/SessionManager.h"

SessionManager::SessionManager(SessionRepository& repo)
    : repo_(repo) {}

Session SessionManager::StartSession(const std::string& token,
                                     const std::string& playerId,
                                     const std::string& externalSessionId,
                                     double initialBalance) {
    Session s;
    s.token = token;
    s.playerId = playerId;
    s.externalSessionId = externalSessionId;
    s.balance = initialBalance;
    s.isActive = true;
    return repo_.Create(s);
}

std::optional<Session> SessionManager::GetSession(std::uint64_t internalId) {
    return repo_.FindById(internalId);
}

std::optional<Session> SessionManager::GetByExternalSessionId(const std::string& externalId) {
    return repo_.FindByExternalId(externalId);
}

void SessionManager::UpdateBalance(std::uint64_t internalId, double newBalance) {
    auto s = repo_.FindById(internalId);
    if (!s) return;
    s->balance = newBalance;
    repo_.Update(*s);
}

void SessionManager::SetLastRound(std::uint64_t internalId, const std::string& roundId) {
    auto s = repo_.FindById(internalId);
    if (!s) return;
    s->lastRoundId = roundId;
    repo_.Update(*s);
}

void SessionManager::ClearLastRound(std::uint64_t internalId) {
    auto s = repo_.FindById(internalId);
    if (!s) return;
    s->lastRoundId.reset();
    repo_.Update(*s);
}

void SessionManager::EndSession(std::uint64_t internalId) {
    auto s = repo_.FindById(internalId);
    if (!s) return;
    s->isActive = false;
    repo_.Update(*s);
}
