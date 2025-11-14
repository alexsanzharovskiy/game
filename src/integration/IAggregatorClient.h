#pragma once
#include <string>
#include <optional>

enum class TxStatus {
    SUCCESS,
    FAILED
};

struct StartSessionResponse {
    std::string sessionId;
    double balance{0.0};
};

struct DebitRequest {
    std::string roundId;
    std::string betTxId;
    double amount{0.0};
    std::string sessionId;
    std::string playerId;
};

struct DebitResponse {
    TxStatus status{TxStatus::FAILED};
    double newBalance{0.0};
    std::string errorCode;
};

struct WinRequest {
    std::string roundId;
    std::string winTxId;
    double amount{0.0};
    std::string sessionId;
    std::string playerId;
};

struct WinResponse {
    TxStatus status{TxStatus::FAILED};
    double newBalance{0.0};
    std::string errorCode;
};

class IAggregatorClient {
public:
    virtual ~IAggregatorClient() = default;

    virtual std::optional<StartSessionResponse> StartSession(const std::string& token) = 0;
    virtual std::optional<DebitResponse> Debit(const DebitRequest& req) = 0;
    virtual std::optional<WinResponse> Win(const WinRequest& req) = 0;
    virtual void EndSession(const std::string& sessionId) = 0;
};
