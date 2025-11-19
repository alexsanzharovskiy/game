#include "api/SessionController.h"
#include "util/Logger.h"
#include <sstream>

static std::string ExtractField(const std::string& json, const std::string& field) {
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

std::string SessionController::StartSession(const std::string& bodyJson) {
    try {
        std::string token    = ExtractField(bodyJson, "token");
        std::string playerId = ExtractField(bodyJson, "player_id");
        if (token.empty() || playerId.empty()) {
            return R"({"status":"ERROR","error_code":"TOKEN_AND_PLAYER_ID_REQUIRED"})";
        }

        auto resp = aggregator_.StartSession(token);
        if (!resp) {
            return R"({"status":"ERROR","error_code":"FAILED_TO_START_SESSION"})";
        }

        // создаём НОВУЮ сессию для игрока
        Session s = sessionManager_.StartSession(token, playerId, resp->sessionId, resp->balance);

        // ищем незавершённый раунд по player_id
        auto unfinished = roundService_.FindUnfinishedRoundForPlayer(playerId);

        std::ostringstream out;
        out << "{"
            << "\"status\":\"OK\","
            << "\"session_id\":" << s.internalId << ","
            << "\"external_session_id\":\"" << s.externalSessionId << "\","
            << "\"balance\":" << s.balance;

        if (unfinished) {
            out << ",\"resumed_round\":{"
                << "\"round_id\":\"" << unfinished->roundId << "\","
                << "\"bet_amount\":" << unfinished->betAmount << ","
                << "\"win_amount\":" << unfinished->winAmount << ","
                << "\"currency\":\"" << unfinished->currency << "\""
                << "}";
        } else {
            out << ",\"resumed_round\":null";
        }

        out << "}";
        return out.str();
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("StartSession error: ") + ex.what());
        return R"({"status":"ERROR","error_code":"INTERNAL_ERROR"})";
    }
}



std::string SessionController::EndSession(const std::string& bodyJson) {
    try {
        std::string sidStr = ExtractField(bodyJson, "session_id");
        if (sidStr.empty()) {
            return R"({"status":"ERROR","error_code":"SESSION_ID_REQUIRED"})";
        }
        std::uint64_t sid = std::stoull(sidStr);
        auto sOpt = sessionManager_.GetSession(sid);
        if (!sOpt) {
            return R"({"status":"ERROR","error_code":"SESSION_NOT_FOUND"})";
        }
        aggregator_.EndSession(sOpt->externalSessionId);
        sessionManager_.EndSession(sid);
        return R"({"status":"OK"})";
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("EndSession error: ") + ex.what());
        return R"({"status":"ERROR","error_code":"INTERNAL_ERROR"})";
    }
}
