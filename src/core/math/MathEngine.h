#pragma once
#include "core/math/GameMathConfig.h"
#include "core/rng/IRng.h"
#include <vector>

class MathEngine {
public:
    MathEngine(const GameMathConfig& config, IRng& rng);

    double CalculateWin(double betAmount);

private:
    const GameMathConfig& config_;
    IRng& rng_;
    std::vector<double> cumulative_;

    void BuildCumulative();
    std::size_t ChooseIndex();
    double RandomDouble01();
};
