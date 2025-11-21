#pragma once
#include "core/model/Transaction.h"
#include <optional>
#include <string>
#include <vector>

class TransactionRepository {
public:
    virtual ~TransactionRepository() = default;

    virtual void Save(const Transaction& tx) = 0;
    virtual void Update(const Transaction& tx) = 0;
    virtual std::optional<Transaction> FindById(const std::string& id) = 0;
    virtual std::vector<Transaction> FindByRound(const std::string& roundId) = 0;
};
