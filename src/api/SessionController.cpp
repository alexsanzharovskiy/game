#include "api/SessionController.h"
#include "util/Logger.h"
#include <sstream>
#include <cstdint>

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
        std::string operatorStr = ExtractField(bodyJson, "operator_id");
        if (token.empty() || playerId.empty() || operatorStr.empty()) {
            return R"({"status":"ERROR","error_code":"TOKEN_PLAYER_ID_OPERATOR_ID_REQUIRED"})";
        }
        std::int64_t operatorId = std::stoll(operatorStr);

        auto resp = aggregator_.StartSession(token, operatorId);
        if (!resp) {
            return R"({"status":"ERROR","error_code":"FAILED_TO_START_SESSION"})";
        }

        // Проверка активной сессии для player+operator
        auto activeSession = sessionManager_.GetActiveByPlayerAndOperator(playerId, operatorId);
        if (activeSession) {
            // попытка закрыть незавершённый раунд и сессию
            roundService_.ResumeUnfinishedRound(activeSession->internalId);
            aggregator_.EndSession(activeSession->externalSessionId);
            sessionManager_.EndSession(activeSession->internalId);
        }

        // создаём НОВУЮ сессию для игрока
        Session s = sessionManager_.StartSession(token, playerId, resp->sessionId, operatorId, resp->balance);

        // ищем незавершённый раунд по player_id
        auto unfinished = roundService_.FindUnfinishedRoundForPlayer(playerId, operatorId);

        std::ostringstream out;
        out << "{"
            << "\"status\":\"OK\","
            << "\"session_id\":" << s.internalId << ","
            << "\"external_session_id\":\"" << s.externalSessionId << "\","
            << "\"operator_id\":" << s.operatorId << ","
            << "\"balance\":" << s.balance;

        if (unfinished) {
            out << ",\"resumed_round\":{"
                << "\"round_id\":\"" << unfinished->roundId << "\","
                << "\"bet_amount\":" << unfinished->betAmount << ","
                << "\"win_amount\":" << unfinished->winAmount << ","
                << "\"currency\":\"" << unfinished->currency << "\","
                << "\"operator_id\":" << unfinished->operatorId
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
        std::string operatorStr = ExtractField(bodyJson, "operator_id");
        if (sidStr.empty() || operatorStr.empty()) {
            return R"({"status":"ERROR","error_code":"SESSION_ID_AND_OPERATOR_ID_REQUIRED"})";
        }
        std::uint64_t sid = std::stoull(sidStr);
        std::int64_t operatorId = std::stoll(operatorStr);
        auto sOpt = sessionManager_.GetSession(sid);
        if (!sOpt) {
            return R"({"status":"ERROR","error_code":"SESSION_NOT_FOUND"})";
        }
        if (sOpt->operatorId != operatorId) {
            return R"({"status":"ERROR","error_code":"OPERATOR_ID_MISMATCH"})";
        }
        aggregator_.EndSession(sOpt->externalSessionId);
        sessionManager_.EndSession(sid);
        return R"({"status":"OK"})";
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("EndSession error: ") + ex.what());
        return R"({"status":"ERROR","error_code":"INTERNAL_ERROR"})";
    }
}
