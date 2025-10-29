#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "thread_pool.h"
#include <string>
#include <map>
#include <memory>

class HttpServer {
public:
    explicit HttpServer(int port = 8080, size_t num_threads = std::thread::hardware_concurrency());
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    void start();

    void stop();

    bool is_running() const { return running_; }

private:
    void handle_client(int client_socket);

    struct HttpRequest {
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    HttpRequest parse_request(const std::string& request_data);

    std::string generate_response(const HttpRequest& request);

    int port_;
    int server_socket_;
    bool running_;

    std::unique_ptr<ThreadPool> thread_pool_;
};

#endif // HTTP_SERVER_H
