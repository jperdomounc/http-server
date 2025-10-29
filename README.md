# Multithreaded HTTP Server

A lightweight, multithreaded HTTP server built in C++ with thread pool architecture for efficient concurrent client connection handling.

## Features

- **Thread Pool Architecture**: Efficiently handles multiple concurrent connections using a thread pool pattern
- **Lightweight**: Minimal dependencies and optimized for performance
- **Docker Support**: Fully containerized for easy cross-platform deployment
- **CMake Build System**: Modern build configuration with CMake
- **Graceful Shutdown**: Handles SIGINT and SIGTERM signals for clean server shutdown
- **Configurable**: Adjustable port and thread pool size via command-line arguments

## Project Structure

```
http-server/
├── CMakeLists.txt          # CMake build configuration
├── Dockerfile              # Multi-stage Docker build
├── .dockerignore           # Docker ignore rules
├── include/                # Header files
│   ├── http_server.h       # HTTP server class
│   └── thread_pool.h       # Thread pool implementation
└── src/                    # Source files
    ├── main.cpp            # Application entry point
    ├── http_server.cpp     # HTTP server implementation
    └── thread_pool.cpp     # Thread pool implementation
```

## Requirements

### Native Build
- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.15 or higher
- POSIX-compliant system (Linux, macOS, Unix)

### Docker Build
- Docker 20.10 or higher

## Building

### Option 1: Native Build with CMake

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build .

# Run
./http-server [port] [num_threads]
```

### Option 2: Docker Build

```bash
# Build the Docker image
docker build -t http-server .

# Run the container
docker run -p 8080:8080 http-server

# Run with custom port
docker run -p 3000:3000 http-server 3000

# Run with custom port and thread count
docker run -p 8080:8080 http-server 8080 8
```

## Usage

### Command Line Arguments

```bash
./http-server [port] [num_threads]
```

- `port` (optional): Port number to bind the server (default: 8080)
- `num_threads` (optional): Number of worker threads in the pool (default: hardware concurrency)

### Examples

```bash
# Run with default settings (port 8080, auto-detect CPU cores)
./http-server

# Run on port 3000
./http-server 3000

# Run on port 8080 with 4 worker threads
./http-server 8080 4
```

## Endpoints

The server provides the following endpoints:

- `GET /` or `GET /index.html` - Home page with server information
- `GET /about` - About page describing server features
- `GET /status` - Server status page showing port and thread count

All other paths return a 404 Not Found response.

## Testing

Test the server using curl, a web browser, or any HTTP client:

```bash
# Test with curl
curl http://localhost:8080/

# Test with wget
wget -O - http://localhost:8080/about

# Test with ApacheBench for load testing
ab -n 1000 -c 10 http://localhost:8080/
```

## Architecture

### Thread Pool
The thread pool manages a fixed number of worker threads that process incoming connections concurrently. This design:
- Prevents thread creation overhead for each connection
- Limits resource consumption
- Provides predictable performance under load

### HTTP Server
The HTTP server:
- Uses non-blocking socket operations
- Parses HTTP/1.1 requests
- Routes requests to appropriate handlers
- Generates proper HTTP responses with headers

## Development

### Adding New Routes

To add new routes, modify `src/http_server.cpp` in the `generate_response()` method:

```cpp
else if (request.path == "/your-route") {
    std::string body = "Your response content";
    response << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: text/plain\r\n"
            << "Content-Length: " << body.length() << "\r\n"
            << "Connection: close\r\n"
            << "\r\n"
            << body;
}
```

### Rebuilding

After making changes:

```bash
# Native build
cd build
cmake --build .

# Docker build
docker build -t http-server .
```

## Performance Considerations

- The server uses a thread pool sized to the number of available CPU cores by default
- Connection handling is efficient with minimal memory allocation
- The multi-stage Docker build produces a slim runtime image (~80MB)
- Socket options are configured for address reuse and optimal performance

## Stopping the Server

Press `Ctrl+C` in the terminal where the server is running, or send a SIGTERM signal:

```bash
kill <pid>
```

The server will gracefully shut down, completing in-flight requests and cleaning up resources.

## License

MIT License - feel free to use this project for learning or production purposes.

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests.
