#include "core/math/GameMathConfig.h"
#include "util/JsonUtils.h"
#include <stdexcept>

GameMathConfig GameMathConfig::LoadFromFiles(const std::string& winRangesPath,
                                             const std::string& probabilitiesPath) {
    GameMathConfig cfg;

    auto winRangesText = util::ReadFileToString(winRangesPath);
    auto payouts = util::ParsePayoutsFromWinRanges(winRangesText);

    auto probsText = util::ReadFileToString(probabilitiesPath);
    auto probs = util::ParseDoubleArray(probsText);

    if (payouts.empty() || probs.empty()) {
        throw std::runtime_error("Empty payouts or probabilities");
    }
    if (payouts.size() != probs.size()) {
        throw std::runtime_error("Payouts and probabilities size mismatch");
    }

    cfg.combinations_.reserve(payouts.size());
    for (double p : payouts) {
        cfg.combinations_.push_back(WinCombination{p});
    }
    cfg.probabilities_ = std::move(probs);
    return cfg;
}
