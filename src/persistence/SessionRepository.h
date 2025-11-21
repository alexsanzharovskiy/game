#pragma once
#include "core/model/Session.h"
#include <optional>
#include <string>

class SessionRepository {
public:
    virtual ~SessionRepository() = default;
    virtual Session Create(const Session& s) = 0;
    virtual std::optional<Session> FindById(std::uint64_t id) = 0;
    virtual std::optional<Session> FindByExternalId(const std::string& externalId) = 0;
    virtual std::optional<Session> FindActiveByPlayerAndOperator(const std::string& playerId,
                                                                 std::int64_t operatorId) = 0;
    virtual void Update(const Session& s) = 0;
};
