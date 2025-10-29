# Multi-stage build for optimized image size

# Build stage
FROM gcc:12 AS builder

# Install CMake
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/

# Build the application
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . && \
    cmake --install . --prefix /app/install

# Runtime stage
FROM debian:bullseye-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 httpserver

# Copy binary from builder
COPY --from=builder /app/install/bin/http-server /usr/local/bin/http-server

# Switch to non-root user
USER httpserver

# Expose port
EXPOSE 8080

# Run the server
ENTRYPOINT ["/usr/local/bin/http-server"]
CMD ["8080"]
