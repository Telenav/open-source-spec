FROM ubuntu:xenial

# System tools
RUN apt-get update
RUN apt-get install -yq unzip \
                        curl \
                        wget \
                        git

# Dependencies
RUN apt-get install -y \
    g++ \
    cmake \
    libboost-all-dev \
    libevent-dev \
    libdouble-conversion-dev \
    libgoogle-glog-dev \
    libgflags-dev \
    libiberty-dev \
    liblz4-dev \
    liblzma-dev \
    libsnappy-dev \
    make \
    zlib1g-dev \
    binutils-dev \
    libjemalloc-dev \
    libssl-dev \
    pkg-config \
    libsodium-dev

# Enable advanced debugging functionality
RUN apt-get install -y \
    libunwind8-dev \
    libelf-dev \
    libdwarf-dev

WORKDIR /code
RUN wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz && \
    tar zxf release-1.8.0.tar.gz && \
    rm -f release-1.8.0.tar.gz && \
    cd googletest-release-1.8.0 && \
    cmake . && \
    make && \
    make install

WORKDIR /code
RUN git clone https://github.com/facebook/folly.git
RUN cd folly && \
    mkdir build_ubuntu && \
    cd build_ubuntu && \
    cmake .. &&\
    make -j $(nproc) &&\
    make install

WORKDIR /code
RUN git clone https://github.com/facebookincubator/fizz.git
RUN cd fizz/fizz && \
    mkdir build_ubuntu && \
    cd build_ubuntu && \
    cmake .. &&\
    make -j $(nproc) &&\
    make install

WORKDIR /code
RUN git clone https://github.com/facebook/wangle.git
RUN cd wangle/wangle && \
    mkdir build_ubuntu && \
    cd build_ubuntu && \
    cmake .. &&\
    make -j $(nproc) &&\
    make install

WORKDIR /code
RUN git clone https://github.com/facebook/proxygen.git

WORKDIR /
