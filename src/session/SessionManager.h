#pragma once
#include "core/model/Session.h"
#include "persistence/SessionRepository.h"
#include <optional>

class SessionManager {
public:
    explicit SessionManager(SessionRepository& repo);

    Session StartSession(const std::string& token,
                         const std::string& playerId,
                         const std::string& externalSessionId,
                         double initialBalance);

    std::optional<Session> GetSession(std::uint64_t internalId);
    std::optional<Session> GetByExternalSessionId(const std::string& externalId);

    void UpdateBalance(std::uint64_t internalId, double newBalance);
    void SetLastRound(std::uint64_t internalId, const std::string& roundId);
    void ClearLastRound(std::uint64_t internalId);
    void EndSession(std::uint64_t internalId);

private:
    SessionRepository& repo_;
};
