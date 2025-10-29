#include "http_server.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

HttpServer::HttpServer(int port, size_t num_threads)
    : port_(port), server_socket_(-1), running_(false) {
    thread_pool_ = std::make_unique<ThreadPool>(num_threads);

    // Ignore SIGPIPE to prevent server crash when client disconnects
    signal(SIGPIPE, SIG_IGN);
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    if (running_) {
        std::cerr << "Server is already running" << std::endl;
        return;
    }

    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_socket_);
        throw std::runtime_error("Failed to set socket options");
    }

    // Bind socket to port
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_socket_);
        throw std::runtime_error("Failed to bind socket to port " + std::to_string(port_));
    }

    // Listen for connections
    if (listen(server_socket_, 128) < 0) {
        close(server_socket_);
        throw std::runtime_error("Failed to listen on socket");
    }

    running_ = true;
    std::cout << "HTTP Server started on port " << port_ << " with "
              << thread_pool_->size() << " worker threads" << std::endl;

    // Accept connections
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            if (running_) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }

        // Submit client handling to thread pool
        try {
            thread_pool_->enqueue([this, client_socket]() {
                this->handle_client(client_socket);
            });
        } catch (const std::exception& e) {
            std::cerr << "Failed to enqueue client handler: " << e.what() << std::endl;
            close(client_socket);
        }
    }
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }

    if (thread_pool_) {
        thread_pool_->shutdown();
    }

    std::cout << "Server stopped" << std::endl;
}

void HttpServer::handle_client(int client_socket) {
    const size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    std::string request_data;

    // Read request
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        request_data = buffer;

        // Parse and process request
        try {
            HttpRequest request = parse_request(request_data);
            std::string response = generate_response(request);

            // Send response
            send(client_socket, response.c_str(), response.length(), 0);
        } catch (const std::exception& e) {
            std::cerr << "Error processing request: " << e.what() << std::endl;

            // Send error response
            std::string error_response =
                "HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 11\r\n"
                "\r\n"
                "Bad Request";
            send(client_socket, error_response.c_str(), error_response.length(), 0);
        }
    }

    close(client_socket);
}

HttpServer::HttpRequest HttpServer::parse_request(const std::string& request_data) {
    HttpRequest request;
    std::istringstream stream(request_data);
    std::string line;

    // Parse request line
    if (std::getline(stream, line)) {
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path >> request.version;
    }

    // Parse headers
    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            break;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            // Trim leading whitespace from value
            size_t start = value.find_first_not_of(" \t");
            if (start != std::string::npos) {
                value = value.substr(start);
            }

            request.headers[key] = value;
        }
    }

    // Read body (if any)
    std::string body_content;
    while (std::getline(stream, line)) {
        body_content += line;
    }
    request.body = body_content;

    return request;
}

std::string HttpServer::generate_response(const HttpRequest& request) {
    std::ostringstream response;

    // Log the request
    std::cout << request.method << " " << request.path << " " << request.version << std::endl;

    // Simple routing
    if (request.method == "GET") {
        if (request.path == "/" || request.path == "/index.html") {
            std::string body =
                "<!DOCTYPE html>\n"
                "<html>\n"
                "<head><title>Multithreaded HTTP Server</title></head>\n"
                "<body>\n"
                "<h1>Welcome to the Multithreaded HTTP Server</h1>\n"
                "<p>This is a lightweight HTTP server built with C++ and thread pool architecture.</p>\n"
                "<ul>\n"
                "<li><a href=\"/\">Home</a></li>\n"
                "<li><a href=\"/about\">About</a></li>\n"
                "<li><a href=\"/status\">Server Status</a></li>\n"
                "</ul>\n"
                "</body>\n"
                "</html>";

            response << "HTTP/1.1 200 OK\r\n"
                    << "Content-Type: text/html\r\n"
                    << "Content-Length: " << body.length() << "\r\n"
                    << "Connection: close\r\n"
                    << "\r\n"
                    << body;
        }
        else if (request.path == "/about") {
            std::string body =
                "<!DOCTYPE html>\n"
                "<html>\n"
                "<head><title>About - HTTP Server</title></head>\n"
                "<body>\n"
                "<h1>About This Server</h1>\n"
                "<p>This is a multithreaded HTTP server implementation in C++.</p>\n"
                "<p>Features:</p>\n"
                "<ul>\n"
                "<li>Thread pool architecture for handling concurrent connections</li>\n"
                "<li>Lightweight and efficient</li>\n"
                "<li>Dockerized for easy deployment</li>\n"
                "<li>CMake build system</li>\n"
                "</ul>\n"
                "<p><a href=\"/\">Back to Home</a></p>\n"
                "</body>\n"
                "</html>";

            response << "HTTP/1.1 200 OK\r\n"
                    << "Content-Type: text/html\r\n"
                    << "Content-Length: " << body.length() << "\r\n"
                    << "Connection: close\r\n"
                    << "\r\n"
                    << body;
        }
        else if (request.path == "/status") {
            std::string body =
                "<!DOCTYPE html>\n"
                "<html>\n"
                "<head><title>Server Status</title></head>\n"
                "<body>\n"
                "<h1>Server Status</h1>\n"
                "<p>Server is running on port " + std::to_string(port_) + "</p>\n"
                "<p>Worker threads: " + std::to_string(thread_pool_->size()) + "</p>\n"
                "<p><a href=\"/\">Back to Home</a></p>\n"
                "</body>\n"
                "</html>";

            response << "HTTP/1.1 200 OK\r\n"
                    << "Content-Type: text/html\r\n"
                    << "Content-Length: " << body.length() << "\r\n"
                    << "Connection: close\r\n"
                    << "\r\n"
                    << body;
        }
        else {
            std::string body =
                "<!DOCTYPE html>\n"
                "<html>\n"
                "<head><title>404 Not Found</title></head>\n"
                "<body>\n"
                "<h1>404 - Not Found</h1>\n"
                "<p>The requested page was not found.</p>\n"
                "<p><a href=\"/\">Back to Home</a></p>\n"
                "</body>\n"
                "</html>";

            response << "HTTP/1.1 404 Not Found\r\n"
                    << "Content-Type: text/html\r\n"
                    << "Content-Length: " << body.length() << "\r\n"
                    << "Connection: close\r\n"
                    << "\r\n"
                    << body;
        }
    }
    else {
        std::string body = "Method Not Allowed";
        response << "HTTP/1.1 405 Method Not Allowed\r\n"
                << "Content-Type: text/plain\r\n"
                << "Content-Length: " << body.length() << "\r\n"
                << "Connection: close\r\n"
                << "\r\n"
                << body;
    }

    return response.str();
}
