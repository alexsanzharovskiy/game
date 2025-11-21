#include "persistence/InMemoryTransactionRepository.h"

void InMemoryTransactionRepository::Save(const Transaction& tx) {
    byId_[tx.id] = tx;
    byRound_[tx.roundId].push_back(tx.id);
}

void InMemoryTransactionRepository::Update(const Transaction& tx) {
    auto it = byId_.find(tx.id);
    if (it == byId_.end()) return;
    it->second = tx;
}

std::optional<Transaction> InMemoryTransactionRepository::FindById(const std::string& id) {
    auto it = byId_.find(id);
    if (it == byId_.end()) return std::nullopt;
    return it->second;
}

std::vector<Transaction> InMemoryTransactionRepository::FindByRound(const std::string& roundId) {
    std::vector<Transaction> result;
    auto it = byRound_.find(roundId);
    if (it == byRound_.end()) return result;
    result.reserve(it->second.size());
    for (const auto& txId : it->second) {
        auto byIdIt = byId_.find(txId);
        if (byIdIt != byId_.end()) {
            result.push_back(byIdIt->second);
        }
    }
    return result;
}
