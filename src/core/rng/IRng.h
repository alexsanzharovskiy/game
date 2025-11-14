#pragma once
#include <cstdint>

class IRng {
public:
    virtual ~IRng() = default;
    virtual std::uint64_t Next() = 0;
};
