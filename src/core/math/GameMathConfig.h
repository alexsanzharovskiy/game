#pragma once
#include <vector>
#include <string>

struct WinCombination {
    double payout{0.0};
};

class GameMathConfig {
public:
    static GameMathConfig LoadFromFiles(const std::string& winRangesPath,
                                        const std::string& probabilitiesPath);

    const std::vector<WinCombination>& GetCombinations() const { return combinations_; }
    const std::vector<double>& GetProbabilities() const { return probabilities_; }

private:
    std::vector<WinCombination> combinations_;
    std::vector<double> probabilities_;
};
