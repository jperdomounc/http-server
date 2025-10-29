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

    // Delete copy constructor and assignment operator
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    // Start the server
    void start();

    // Stop the server
    void stop();

    // Check if server is running
    bool is_running() const { return running_; }

private:
    // Handle client connection
    void handle_client(int client_socket);

    // Parse HTTP request
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    // Parse request from raw data
    HttpRequest parse_request(const std::string& request_data);

    // Generate HTTP response
    std::string generate_response(const HttpRequest& request);

    // Server configuration
    int port_;
    int server_socket_;
    bool running_;

    // Thread pool for handling connections
    std::unique_ptr<ThreadPool> thread_pool_;
};

#endif // HTTP_SERVER_H
