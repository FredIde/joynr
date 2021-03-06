FROM joynr-cpp-base:latest

###################################################
# install gcovr for code coverage reports
###################################################
RUN dnf update -y \
    && dnf install -y \
    python-pip \
    && dnf clean all \
    && pip install gcovr

###################################################
# install lcov
###################################################
RUN dnf update -y \
    && dnf install -y \
    lcov \
    && dnf clean all

###################################################
# install boost
###################################################
RUN dnf update -y \
    && dnf install -y \
    boost \
    boost-devel \
    && dnf clean all

###################################################
# install rapidjson
###################################################

RUN cd /opt \
    && git clone https://github.com/miloyip/rapidjson.git rapidjson \
    && cd rapidjson \
    && git checkout v1.1.0 \
    && mkdir build \
    && cd build \
    && cmake -DRAPIDJSON_BUILD_DOC=OFF \
    -DRAPIDJSON_BUILD_EXAMPLES=OFF \
    -DRAPIDJSON_BUILD_TESTS=OFF \
    -DRAPIDJSON_BUILD_THIRDPARTY_GTEST=OFF .. \
    && make install -j"$(nproc)" \
    && cd /opt \
    && rm -rf rapidjson

###################################################
# install muesli
###################################################

RUN cd /opt \
    && git clone https://github.com/bmwcarit/muesli.git \
    && cd muesli \
    && git checkout 0.3.1 \
    && mkdir build \
    && cd build \
    && cmake -DBUILD_MUESLI_TESTS=Off -DUSE_PLATFORM_RAPIDJSON=On .. \
    && make install -j"$(nproc)" \
    && cd /opt \
    && rm -rf muesli

###################################################
# install googletest & googlemock
###################################################

RUN cd /opt \
    && git clone https://github.com/google/googletest.git \
    && cd googletest \
    && git checkout ddb8012e \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make install -j"$(nproc)" \
    && cd /opt/ \
    && rm -rf googletest
