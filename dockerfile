FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials and debugging tools
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    gdb \
    valgrind \
    strace \
    ltrace \
    net-tools \
    iputils-ping \
    curl \
    telnet \
    vim \
    && rm -rf /var/lib/apt/lists/*


RUN curl -fsSL https://opencode.ai/install | bash

# Set working directory
WORKDIR /webserv

# Default to bash
CMD ["/bin/bash"]
