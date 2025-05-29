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

# Copy build configuration
cp ../Config.mk .
cp ../Makefile .

echo "Build files collected in build directory" 