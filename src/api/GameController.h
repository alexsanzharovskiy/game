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

    // /game/play — {"session_id":1,"player_id":"p1","amount":10,"currency":"USD"}
    std::string Play(const std::string& bodyJson);

    // /game/finish — {"player_id":"p1","round_id":"round_1_1"}
    std::string Finish(const std::string& bodyJson);

private:
    RoundService& roundService_;
    SessionManager& sessionManager_;
};

