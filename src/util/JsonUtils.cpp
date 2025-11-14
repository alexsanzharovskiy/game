#include "util/JsonUtils.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace util {

std::string ReadFileToString(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static std::string Trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::vector<double> ParseDoubleArray(const std::string& jsonText) {
    std::vector<double> result;
    auto start = jsonText.find('[');
    auto end = jsonText.rfind(']');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        throw std::runtime_error("Invalid JSON array");
    }
    std::string arr = jsonText.substr(start + 1, end - start - 1);

    std::string token;
    std::stringstream ss(arr);
    while (std::getline(ss, token, ',')) {
        token = Trim(token);
        if (!token.empty()) {
            result.push_back(std::stod(token));
        }
    }
    return result;
}

std::vector<double> ParsePayoutsFromWinRanges(const std::string& jsonText) {
    std::vector<double> payouts;
    std::string key = "\"payout\"";
    size_t pos = 0;
    while (true) {
        pos = jsonText.find(key, pos);
        if (pos == std::string::npos) break;
        pos += key.size();
        pos = jsonText.find(':', pos);
        if (pos == std::string::npos) break;
        ++pos;
        std::string number;
        while (pos < jsonText.size()) {
            char c = jsonText[pos];
            if (c == ',' || c == '}' || c == '\n' || c == '\r') break;
            number.push_back(c);
            ++pos;
        }
        number = Trim(number);
        if (!number.empty()) {
            payouts.push_back(std::stod(number));
        }
    }
    return payouts;
}

} // namespace util
