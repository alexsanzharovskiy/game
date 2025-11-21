#pragma once
#include "persistence/TransactionRepository.h"
#include <unordered_map>
#include <vector>

class InMemoryTransactionRepository : public TransactionRepository {
public:
    void Save(const Transaction& tx) override;
    void Update(const Transaction& tx) override;
    std::optional<Transaction> FindById(const std::string& id) override;
    std::vector<Transaction> FindByRound(const std::string& roundId) override;

private:
    std::unordered_map<std::string, Transaction> byId_;
    std::unordered_map<std::string, std::vector<std::string>> byRound_;
};
