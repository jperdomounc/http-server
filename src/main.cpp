#include "http_server.h"
#include <iostream>
#include <csignal>
#include <memory>
#include <cstdlib>

std::unique_ptr<HttpServer> server;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    size_t num_threads = std::thread::hardware_concurrency();

    if (argc > 1) {
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Invalid port number. Using default port 8080." << std::endl;
            port = 8080;
        }
    }

    if (argc > 2) {
        num_threads = std::atoi(argv[2]);
        if (num_threads <= 0) {
            std::cerr << "Invalid number of threads. Using hardware concurrency." << std::endl;
            num_threads = std::thread::hardware_concurrency();
        }
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        server = std::make_unique<HttpServer>(port, num_threads);

        std::cout << "Starting HTTP server..." << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;

        server->start();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
