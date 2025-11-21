#pragma once
#include "integration/IAggregatorClient.h"
#include "session/SessionManager.h"
#include <string>

class BalanceController {
public:
    BalanceController(SessionManager& sessionManager,
                      IAggregatorClient& aggregator)
        : sessionManager_(sessionManager),
          aggregator_(aggregator) {}

    // body: {"operator_id":1,"player_id":"p1","session_id":123}
    std::string GetBalance(const std::string& bodyJson);

private:
    SessionManager& sessionManager_;
    IAggregatorClient& aggregator_;
};
