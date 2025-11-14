#pragma once
#include <string>
#include "session/SessionManager.h"
#include "integration/IAggregatorClient.h"
#include "session/RoundService.h"

class SessionController {
public:
    SessionController(SessionManager& sessionManager,
                      IAggregatorClient& aggregator,
                      RoundService& roundService)
        : sessionManager_(sessionManager),
          aggregator_(aggregator),
          roundService_(roundService) {}

    // body: {"token":"...","player_id":"..."}
    std::string StartSession(const std::string& bodyJson);
    // body: {"session_id":1}
    std::string EndSession(const std::string& bodyJson);

private:
    SessionManager& sessionManager_;
    IAggregatorClient& aggregator_;
    RoundService& roundService_;
};
