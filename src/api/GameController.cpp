#include "api/GameController.h"
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

// 1) /game/play — BET + генерация раунда, без WIN
std::string GameController::Play(const std::string& bodyJson) {
    try {
        std::string sidStr   = ExtractField(bodyJson, "session_id");
        std::string playerId = ExtractField(bodyJson, "player_id");
        std::string amountStr= ExtractField(bodyJson, "amount");
        std::string currency = ExtractField(bodyJson, "currency");
        std::string operatorStr = ExtractField(bodyJson, "operator_id");

        if (sidStr.empty() || playerId.empty() || amountStr.empty() || operatorStr.empty()) {
            return R"({"status":"ERROR","error_code":"SESSION_ID_PLAYER_ID_OPERATOR_ID_AMOUNT_REQUIRED"})";
        }
        if (currency.empty()) {
            currency = "USD"; // дефолт, если не передали
        }

        std::uint64_t sid = std::stoull(sidStr);
        double amount = std::stod(amountStr);
        std::int64_t operatorId = std::stoll(operatorStr);

        // Можно (опционально) проверить, что playerId совпадает с тем, что в сессии
        auto sOpt = sessionManager_.GetSession(sid);
        if (!sOpt) {
            return R"({"status":"ERROR","error_code":"SESSION_NOT_FOUND"})";
        }
        if (sOpt->playerId != playerId) {
            return R"({"status":"ERROR","error_code":"PLAYER_ID_MISMATCH"})";
        }
        if (sOpt->operatorId != operatorId) {
            return R"({"status":"ERROR","error_code":"OPERATOR_ID_MISMATCH"})";
        }
        if (!sOpt->isActive) {
            return R"({"status":"ERROR","error_code":"SESSION_NOT_ACTIVE"})";
        }

        RoundResult result = roundService_.PlayRound(sid, amount, currency);

        // Баланс пока без учёта WIN — только после WIN/finish
        auto sessAfter = sessionManager_.GetSession(sid);
        double balance = sessAfter ? sessAfter->balance : 0.0;

        std::ostringstream out;
        out << "{"
            << "\"status\":\"OK\","
            << "\"session_id\":" << sid << ","
            << "\"player_id\":\"" << playerId << "\","
            << "\"round_id\":\"" << result.roundId << "\","
            << "\"bet_amount\":" << result.betAmount << ","
            << "\"win_amount\":" << result.winAmount << ","
            << "\"currency\":\"" << result.currency << "\","
            << "\"balance\":" << balance << ","
            << "\"operator_id\":" << result.operatorId << ","
            << "\"round_status\":\""
            << (result.status == RoundStatus::COMPLETED ? "COMPLETED" :
                result.status == RoundStatus::WIN_PENDING ? "WIN_PENDING" :
                result.status == RoundStatus::BET_FINISHED ? "BET_FINISHED" : "CANCELLED")
            << "\""
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

// 2) /game/finish — WIN + финализация раунда
std::string GameController::Finish(const std::string& bodyJson) {
    try {
        std::string playerId = ExtractField(bodyJson, "player_id");
        std::string roundId  = ExtractField(bodyJson, "round_id");
        std::string operatorStr = ExtractField(bodyJson, "operator_id");

        if (playerId.empty() || roundId.empty() || operatorStr.empty()) {
            return R"({"status":"ERROR","error_code":"PLAYER_ID_ROUND_ID_OPERATOR_ID_REQUIRED"})";
        }
        std::int64_t operatorId = std::stoll(operatorStr);

        // Завершаем раунд чисто по player_id + round_id
        RoundResult result = roundService_.FinishRound(playerId, roundId, operatorId);

        // После WIN баланс обновился в той сессии, где был BET.
        // Найдём актуальный баланс по internal sessionId, который хранится в round.sessionId.
        auto sessOpt = sessionManager_.GetSession(result.sessionId);
        double balance = sessOpt ? sessOpt->balance : 0.0;
        if (!sessOpt || sessOpt->operatorId != operatorId || result.operatorId != operatorId) {
            return R"({"status":"ERROR","error_code":"OPERATOR_ID_MISMATCH"})";
        }

        std::ostringstream out;
        out << "{"
            << "\"status\":\"OK\","
            << "\"player_id\":\"" << playerId << "\","
            << "\"round_id\":\"" << result.roundId << "\","
            << "\"bet_amount\":" << result.betAmount << ","
            << "\"win_amount\":" << result.winAmount << ","
            << "\"currency\":\"" << result.currency << "\","
            << "\"balance\":" << balance << ","
            << "\"operator_id\":" << result.operatorId << ","
            << "\"round_status\":\""
            << (result.status == RoundStatus::COMPLETED ? "COMPLETED" :
                result.status == RoundStatus::WIN_PENDING ? "WIN_PENDING" :
                result.status == RoundStatus::BET_FINISHED ? "BET_FINISHED" : "CANCELLED")
            << "\""
            << "}";
        return out.str();
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("Finish error: ") + ex.what());
        std::ostringstream out;
        out << "{"
            << "\"status\":\"ERROR\","
            << "\"error_code\":\"" << ex.what() << "\""
            << "}";
        return out.str();
    }
}
