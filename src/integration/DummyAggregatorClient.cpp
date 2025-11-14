#include "integration/DummyAggregatorClient.h"
#include "util/Logger.h"

std::optional<StartSessionResponse> DummyAggregatorClient::StartSession(const std::string& token) {
    StartSessionResponse resp;
    resp.sessionId = "ext_" + token;
    resp.balance = 1000.0;
    balances_[resp.sessionId] = resp.balance;
    util::Logger::Info("DummyAggregator: StartSession for token=" + token);
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
                       " from " + req.sessionId + " for round " + req.roundId);
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
                       " to " + req.sessionId + " for round " + req.roundId);
    return resp;
}

void DummyAggregatorClient::EndSession(const std::string& sessionId) {
    util::Logger::Info("DummyAggregator: EndSession " + sessionId);
}
