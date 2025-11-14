#pragma once
#include <functional>
#include <string>
#include <unordered_map>

class HttpServer {
public:
    using Handler = std::function<std::string(const std::string& body)>;

    void RegisterHandler(const std::string& path, Handler handler) {
        handlers_[path] = std::move(handler);
    }

    void Start(int port);

private:
    std::unordered_map<std::string, Handler> handlers_;
};
