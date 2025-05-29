#!/bin/bash

# Exit on error
set -e

# Set the correct toolchain prefix
export TOOLPREFIX="aarch64-none-elf-"

# Check if CIRCLE_STDLIB_DIR is set
if [ -z "$CIRCLE_STDLIB_DIR" ]; then
    echo "CIRCLE_STDLIB_DIR is not set. Setting it to ../circle-stdlib"
    export CIRCLE_STDLIB_DIR="../circle-stdlib"
fi

# Create and enter the build directory
mkdir -p build
cd build

# Clone circle-stdlib if it doesn't exist
if [ ! -d "circle-stdlib" ]; then
    echo "Cloning circle-stdlib..."
    git clone https://github.com/smuehlst/circle-stdlib.git
    cd circle-stdlib
else
    cd circle-stdlib
    echo "Updating circle-stdlib..."
    git pull
fi

# Initialize and update submodules
git submodule update --init --recursive

# Configure for Raspberry Pi 4
echo "Configuring for Raspberry Pi 4..."
./configure --raspberrypi 4

# Create a Config.mk file with the correct toolchain prefix
echo "Creating Config.mk with toolchain settings..."
cat > Config.mk << EOF
AARCH = 64
RASPPI = 4
PREFIX = ${TOOLPREFIX}
EOF

# Build circle-stdlib
echo "Building circle-stdlib..."
make -j$(nproc)

cd ..

# Clone CMSIS if it doesn't exist
if [ ! -d "CMSIS_5" ]; then
    echo "Cloning CMSIS..."
    git clone https://github.com/ARM-software/CMSIS_5.git
fi

# Clone Synth_Dexed if it doesn't exist
if [ ! -d "Synth_Dexed" ]; then
    echo "Cloning Synth_Dexed..."
    git clone https://codeberg.org/dcoredump/Synth_Dexed.git
fi

echo "Setup complete! You can now build MiniDexed."
echo "Make sure to set these environment variables:"
echo "export CIRCLE_STDLIB_DIR=$(pwd)/circle-stdlib"
echo "export SYNTH_DEXED_DIR=$(pwd)/Synth_Dexed/src"
echo "export CMSIS_DIR=$(pwd)/CMSIS_5/CMSIS" 