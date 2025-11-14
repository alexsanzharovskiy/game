#pragma once
#include <string>
#include "session/RoundService.h"
#include "session/SessionManager.h"

class GameController {
public:
    GameController(RoundService& roundService,
                   SessionManager& sessionManager)
        : roundService_(roundService),
          sessionManager_(sessionManager) {}

    // body: {"session_id":1,"amount":10}
    std::string Play(const std::string& bodyJson);

private:
    RoundService& roundService_;
    SessionManager& sessionManager_;
};
