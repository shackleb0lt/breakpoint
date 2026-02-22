#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Print commands before executing them (helpful for debugging)
# set -x

# 1. Create directory if it doesn't exist
mkdir -p build

# 2. Only run CMake if the cache doesn't exist
if [ ! -f "build/CMakeCache.txt" ]; then

    cmake -S . -B build -G "Ninja" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_TOOLCHAIN_FILE=/home/qubit/vcpkg/scripts/buildsystems/vcpkg.cmake 
else
    echo "--- CMake cache found, skipping configuration ---"
fi

ninja -C build

# 4. Set permissions
# echo "--- Applying capabilities ---"
# sudo setcap CAP_SYS_PTRACE=+eip bkpt

cd build
ctest