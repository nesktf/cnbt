#!/usr/bin/env sh

cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCNBT_BUILD_TESTS=1 && \
make -C build -j$(nproc) &&  \
./build/test/cnbt_test
