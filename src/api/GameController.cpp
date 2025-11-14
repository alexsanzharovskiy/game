#include "api/GameController.h"
#include "util/Logger.h"
#include <sstream>

static std::string ExtractFieldSimple(const std::string& json, const std::string& field) {
    std::string key = "\"" + field + "\"";
    auto pos = json.find(key);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\"')) ++pos;
    std::string value;
    while (pos < json.size() && json[pos] != '\"' && json[pos] != ',' && json[pos] != '}') {
        value.push_back(json[pos]);
        ++pos;
    }
    return value;
}

std::string GameController::Play(const std::string& bodyJson) {
    try {
        std::string sidStr = ExtractFieldSimple(bodyJson, "session_id");
        std::string betStr = ExtractFieldSimple(bodyJson, "amount");
        if (sidStr.empty() || betStr.empty()) {
            return R"({"status":"ERROR","error_code":"SESSION_ID_AND_AMOUNT_REQUIRED"})";
        }
        std::uint64_t sid = std::stoull(sidStr);
        double bet = std::stod(betStr);

        RoundResult result = roundService_.PlayRound(sid, bet);

        auto sOpt = sessionManager_.GetSession(sid);
        double balance = sOpt ? sOpt->balance : 0.0;

        std::ostringstream out;
        out << "{"
            << "\"status\":\"OK\","
            << "\"round\":{"
            <<   "\"round_id\":\"" << result.roundId << "\","
            <<   "\"bet_amount\":" << result.betAmount << ","
            <<   "\"win_amount\":" << result.winAmount << ","
            <<   "\"balance\":" << balance
            << "}"
            << "}";
        return out.str();
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("Play error: ") + ex.what());
        std::ostringstream out;
        out << "{"
            << "\"status\":\"ERROR\","
            << "\"error_code\":\"" << ex.what() << "\""
            << "}";
        return out.str();
    }
}
