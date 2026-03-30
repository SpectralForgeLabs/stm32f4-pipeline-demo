FROM debian:12.13-slim

ARG UNITY_VERSION=v2.6.1

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    g++ \
    cmake \
    ninja-build \
    make \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 --branch ${UNITY_VERSION} \
    https://github.com/ThrowTheSwitch/Unity.git /opt/unity


RUN groupadd -g 1000 tester && \
    useradd -u 1000 -g 1000 -m tester
USER tester

ENV UNITY_DIR=/opt/unity

WORKDIR /home/tester