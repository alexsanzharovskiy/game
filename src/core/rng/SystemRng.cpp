#include "core/rng/SystemRng.h"
#include <chrono>

SystemRng::SystemRng() {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    engine_.seed(static_cast<std::mt19937_64::result_type>(seed));
}

std::uint64_t SystemRng::Next() {
    return engine_();
}
