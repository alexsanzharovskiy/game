#include "util/Logger.h"
#include "core/rng/SystemRng.h"
#include "core/math/GameMathConfig.h"
#include "core/math/MathEngine.h"
#include "persistence/InMemorySessionRepository.h"
#include "persistence/InMemoryRoundRepository.h"
#include "persistence/InMemoryTransactionRepository.h"
#include "integration/DummyAggregatorClient.h"
#include "session/SessionManager.h"
#include "session/RoundService.h"
#include "api/HttpServer.h"
#include "api/SessionController.h"
#include "api/GameController.h"
#include "api/BalanceController.h"
#include <filesystem>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    try {
      std::filesystem::path exePath = std::filesystem::canonical(argv[0]);
      std::filesystem::path baseDir = exePath.parent_path(); // папка с бинарём

      // если storage лежит рядом с бинарём:
      // auto storageDir = baseDir / "storage";

      // если storage лежит на уровень выше (как у тебя сейчас):
      auto storageDir = baseDir.parent_path() / "storage";

      std::string winRangesPath = (storageDir / "win_ranges.json").string();
      std::string probabilitiesPath = (storageDir / "rtp_probabilities.json").string();

        GameMathConfig cfg = GameMathConfig::LoadFromFiles(winRangesPath, probabilitiesPath);
        SystemRng rng;
        MathEngine math(cfg, rng);

        InMemorySessionRepository sessionRepo;
        InMemoryRoundRepository roundRepo;
        InMemoryTransactionRepository txRepo;
        DummyAggregatorClient aggregator;
        SessionManager sessionManager(sessionRepo);
        RoundService roundService(math, roundRepo, txRepo, sessionManager, aggregator);

        HttpServer server;
        SessionController sessionController(sessionManager, aggregator, roundService);
        GameController gameController(roundService, sessionManager);
        BalanceController balanceController(sessionManager, aggregator);

        server.RegisterHandler("/session/start",
            [&sessionController](const std::string& body) {
                return sessionController.StartSession(body);
            });

        server.RegisterHandler("/session/end",
            [&sessionController](const std::string& body) {
                return sessionController.EndSession(body);
            });

        // 1) /game/play — BET + генерация раунда, без WIN
        server.RegisterHandler("/game/play",
            [&gameController](const std::string& body) {
                return gameController.Play(body);
            });

        // 2) /game/finish — WIN + финализация раунда
        server.RegisterHandler("/game/finish",
            [&gameController](const std::string& body) {
                return gameController.Finish(body);
            });

        server.RegisterHandler("/balance",
            [&balanceController](const std::string& body) {
                return balanceController.GetBalance(body);
            });

        server.Start(8080);

        return 0;
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("Fatal error: ") + ex.what());
        return 1;
    }
}
