#include "core/math/MathEngine.h"
#include <stdexcept>
#include <limits>

MathEngine::MathEngine(const GameMathConfig& config, IRng& rng)
    : config_(config), rng_(rng) {
    BuildCumulative();
}

void MathEngine::BuildCumulative() {
    const auto& probs = config_.GetProbabilities();
    cumulative_.clear();
    cumulative_.reserve(probs.size());
    double sum = 0.0;
    for (double p : probs) {
        sum += p;
        cumulative_.push_back(sum);
    }
    if (sum <= 0.0) {
        throw std::runtime_error("Total probability sum <= 0");
    }
    for (double& v : cumulative_) {
        v /= sum;
    }
}

double MathEngine::RandomDouble01() {
    std::uint64_t v = rng_.Next();
    const double denom = static_cast<double>(std::numeric_limits<std::uint64_t>::max());
    return static_cast<double>(v) / denom;
}

std::size_t MathEngine::ChooseIndex() {
    double r = RandomDouble01();
    const std::size_t n = cumulative_.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (r < cumulative_[i]) {
            return i;
        }
    }
    return n - 1;
}

double MathEngine::CalculateWin(double betAmount) {
    if (betAmount <= 0.0) {
        throw std::runtime_error("Bet amount must be positive");
    }
    std::size_t idx = ChooseIndex();
    const auto& combs = config_.GetCombinations();
    if (idx >= combs.size()) {
        throw std::runtime_error("Chosen index out of range");
    }
    double payout = combs[idx].payout;
    double winAmount = payout * betAmount;
    return winAmount;
}
