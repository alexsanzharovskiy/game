#include "integration/DummyAggregatorClient.h"
#include "util/Logger.h"

std::optional<StartSessionResponse> DummyAggregatorClient::StartSession(const std::string& token,
                                                                        std::int64_t operatorId) {
    StartSessionResponse resp;
    resp.sessionId = "ext_" + token;
    resp.balance = 1000.0;
    resp.operatorId = operatorId;
    balances_[resp.sessionId] = resp.balance;
    util::Logger::Info("DummyAggregator: StartSession for token=" + token +
                       " operator=" + std::to_string(operatorId));
    return resp;
}

std::optional<DebitResponse> DummyAggregatorClient::Debit(const DebitRequest& req) {
    DebitResponse resp;
    auto it = balances_.find(req.sessionId);
    if (it == balances_.end()) {
        resp.status = TxStatus::FAILED;
        resp.errorCode = "SESSION_NOT_FOUND";
        return resp;
    }
    if (it->second < req.amount) {
        resp.status = TxStatus::FAILED;
        resp.errorCode = "INSUFFICIENT_FUNDS";
        resp.newBalance = it->second;
        return resp;
    }
    it->second -= req.amount;
    resp.status = TxStatus::SUCCESS;
    resp.newBalance = it->second;
    util::Logger::Info("DummyAggregator: Debit " + std::to_string(req.amount) +
                       " from " + req.sessionId + " for round " + req.roundId +
                       " operator=" + std::to_string(req.operatorId));
    return resp;
}

std::optional<WinResponse> DummyAggregatorClient::Win(const WinRequest& req) {
    WinResponse resp;
    auto it = balances_.find(req.sessionId);
    if (it == balances_.end()) {
        resp.status = TxStatus::FAILED;
        resp.errorCode = "SESSION_NOT_FOUND";
        return resp;
    }
    it->second += req.amount;
    resp.status = TxStatus::SUCCESS;
    resp.newBalance = it->second;
    util::Logger::Info("DummyAggregator: Win " + std::to_string(req.amount) +
                       " to " + req.sessionId + " for round " + req.roundId +
                       " operator=" + std::to_string(req.operatorId));
    return resp;
}

void DummyAggregatorClient::EndSession(const std::string& sessionId) {
    util::Logger::Info("DummyAggregator: EndSession " + sessionId);
}

std::optional<BalanceResponse> DummyAggregatorClient::Balance(const std::string& sessionId,
                                                             const std::string& playerId,
                                                             std::int64_t operatorId) {
    BalanceResponse resp;
    auto it = balances_.find(sessionId);
    if (it == balances_.end()) {
        resp.status = TxStatus::FAILED;
        resp.errorCode = "SESSION_NOT_FOUND";
        return resp;
    }
    resp.status = TxStatus::SUCCESS;
    resp.balance = it->second;
    resp.currency = "USD";
    util::Logger::Info("DummyAggregator: Balance session=" + sessionId +
                       " player=" + playerId +
                       " operator=" + std::to_string(operatorId) +
                       " balance=" + std::to_string(resp.balance));
    return resp;
}
