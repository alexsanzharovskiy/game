#include "api/BalanceController.h"
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

std::string BalanceController::GetBalance(const std::string& bodyJson) {
    try {
        std::string operatorStr = ExtractField(bodyJson, "operator_id");
        std::string playerId = ExtractField(bodyJson, "player_id");
        std::string sessionStr = ExtractField(bodyJson, "session_id");

        if (operatorStr.empty() || playerId.empty() || sessionStr.empty()) {
            return R"({"status":"ERROR","error_code":"OPERATOR_ID_PLAYER_ID_SESSION_ID_REQUIRED"})";
        }

        std::int64_t operatorId = std::stoll(operatorStr);
        std::uint64_t sessionId = std::stoull(sessionStr);

        auto sOpt = sessionManager_.GetSession(sessionId);
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

        auto resp = aggregator_.Balance(sOpt->externalSessionId, playerId, operatorId);
        if (!resp || resp->status != TxStatus::SUCCESS) {
            return R"({"status":"ERROR","error_code":"FAILED_TO_FETCH_BALANCE"})";
        }

        // обновим локальный баланс
        sessionManager_.UpdateBalance(sessionId, resp->balance);

        std::ostringstream out;
        out << "{"
            << "\"status\":\"OK\","
            << "\"currency\":\"" << (resp->currency.empty() ? "USD" : resp->currency) << "\","
            << "\"balance\":" << resp->balance
            << "}";
        return out.str();
    } catch (const std::exception& ex) {
        util::Logger::Error(std::string("Balance error: ") + ex.what());
        return R"({"status":"ERROR","error_code":"INTERNAL_ERROR"})";
    }
}
