FROM ubuntu:latest as build

RUN apt-get update && \
    apt-get install -y \
    g++ \
    libtbb-dev \
    cmake \ 
    protobuf-compiler

WORKDIR /transport_catalogue

COPY src/ ./src/

WORKDIR /transport_catalogue/build

RUN cmake ../src && \
    cmake --build .

FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y \
    g++ \
    libtbb-dev \
    cmake \ 
    protobuf-compiler

COPY --from=build \
    ./transport_catalogue/build/transport_catalogue \
    ./app/

WORKDIR /app
