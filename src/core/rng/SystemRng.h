#pragma once
#include "core/rng/IRng.h"
#include <random>

class SystemRng : public IRng {
public:
    SystemRng();
    std::uint64_t Next() override;

private:
    std::mt19937_64 engine_;
};
