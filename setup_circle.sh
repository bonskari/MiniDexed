#!/bin/bash

# Exit on error
set -e

# Set toolchain prefix
export TOOLPREFIX=aarch64-none-elf-

# Clone Circle and its dependencies if not already present
if [ ! -d "circle-stdlib" ]; then
    git clone https://github.com/smuehlst/circle-stdlib.git
    cd circle-stdlib
    git submodule update --init --recursive
    cd ..
fi

# Clone CMSIS if not already present
if [ ! -d "CMSIS_5" ]; then
    git clone https://github.com/ARM-software/CMSIS_5.git
fi

# Clone Dexed synth if not already present
if [ ! -d "Synth_Dexed" ]; then
    git clone https://codeberg.org/dcoredump/Synth_Dexed.git
fi

# Configure circle-stdlib
cd circle-stdlib

# Create Config.mk with correct settings
cat > Config.mk << EOF
AARCH = 64
RASPPI = 4
PREFIX = ${TOOLPREFIX}
EOF

./configure --raspberrypi 4
make
cd ..

echo "Circle environment setup complete!" 