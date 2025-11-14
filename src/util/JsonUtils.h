#pragma once
#include <string>
#include <vector>

namespace util {

std::vector<double> ParseDoubleArray(const std::string& jsonText);
std::vector<double> ParsePayoutsFromWinRanges(const std::string& jsonText);
std::string ReadFileToString(const std::string& path);

} // namespace util
