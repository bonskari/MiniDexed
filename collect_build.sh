#!/bin/bash

# Exit on error
set -e

# Create build directory if it doesn't exist
mkdir -p build

# Copy necessary files
cp -r src/* build/
cp -r circle-stdlib/libs/fatfs build/
cp -r circle-stdlib/libs/circle build/
cp -r CMSIS_5/CMSIS build/
cp -r Synth_Dexed/src build/dexed

# Create symlinks for dependencies
cd build
ln -sf circle/include circle_include
ln -sf dexed/src dexed_src
ln -sf CMSIS/Core/Include cmsis_include

# Create Config.mk for Raspberry Pi 4
cat > Config.mk << EOF
AARCH = 64
RASPPI = 4
PREFIX = aarch64-none-elf-
EOF

# Copy Makefile
cp ../Makefile .

echo "Build files collected in build directory" 