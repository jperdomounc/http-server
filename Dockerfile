
FROM gcc:12 AS builder


RUN apt-get update && apt-get install -y \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app


COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/


RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . && \
    cmake --install . --prefix /app/install


FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -u 1000 httpserver

COPY --from=builder /app/install/bin/http-server /usr/local/bin/http-server

USER httpserver

EXPOSE 8080

ENTRYPOINT ["/usr/local/bin/http-server"]
CMD ["8080"]
