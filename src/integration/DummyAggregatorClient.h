#pragma once
#include "integration/IAggregatorClient.h"
#include <unordered_map>

class DummyAggregatorClient : public IAggregatorClient {
public:
    std::optional<StartSessionResponse> StartSession(const std::string& token) override;
    std::optional<DebitResponse> Debit(const DebitRequest& req) override;
    std::optional<WinResponse> Win(const WinRequest& req) override;
    void EndSession(const std::string& sessionId) override;

private:
    std::unordered_map<std::string, double> balances_;
};
