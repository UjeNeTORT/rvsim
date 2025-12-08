FROM ubuntu:24.04

RUN apt-get update &&                                              \
    apt-get install -y lsb-release wget software-properties-common \
                       build-essential cmake ninja-build git       \
                       zlib1g-dev libzstd-dev curl libedit-dev     \
                       libcurl4-openssl-dev libgtest-dev

RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 21

RUN apt-get update && \
    apt-get install -y llvm-21-dev clang-21 gcc-riscv64-unknown-elf

ENV CC=clang-21
ENV CXX=clang++-21
ENV LLVM_DIR=/usr/lib/llvm-21/cmake

WORKDIR /rv32
