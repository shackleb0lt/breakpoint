#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Print commands before executing them (helpful for debugging)
# set -x

# 1. Create directory if it doesn't exist
mkdir -p build

VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
VCPKG_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

if [[ ! -f "$VCPKG_TOOLCHAIN" ]]; then
    echo "Error: vcpkg toolchain not found at $VCPKG_TOOLCHAIN"
    exit 1
fi

if command -v ninja &> /dev/null; then
    CMAKE_GENERATOR="Ninja"
    BUILD_CMD="ninja -C build"
elif command -v make &> /dev/null; then
    CMAKE_GENERATOR="Unix Makefiles"
    BUILD_CMD="make -C build"
else
    echo "Neither ninja nor make found."
    exit 1
fi


cmake -S . -B build -G $CMAKE_GENERATOR \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN"

$BUILD_CMD

# 4. Set permissions
# echo "--- Applying capabilities ---"
# sudo setcap CAP_SYS_PTRACE=+eip bkpt

cd build
ctest