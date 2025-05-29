#!/bin/bash

# Install required dependencies (for Debian/Ubuntu-based systems)
if command -v apt-get &> /dev/null; then
    echo "Installing dependencies for Debian/Ubuntu..."
    sudo apt-get update && sudo apt-get install -y git make gcc-arm-none-eabi
elif command -v dnf &> /dev/null; then
    echo "Installing dependencies for Fedora..."
    sudo dnf install -y git make arm-none-eabi-gcc-cs
else
    echo "Please install the following packages manually:"
    echo "- git"
    echo "- make"
    echo "- gcc-arm-none-eabi (ARM cross-compiler)"
fi

# Initialize and update submodules if needed
if [ ! -d "circle-stdlib" ] || [ ! -d "Synth_Dexed" ] || [ ! -d "CMSIS_5" ]; then
    echo "Initializing submodules..."
    git submodule update --init --recursive
fi

# Build the project
cd src
make

# Check if kernel7l.img was created
if [ -f "kernel7l.img" ]; then
    echo "Build successful! kernel7l.img has been created for Raspberry Pi 4"
else
    echo "Build failed or kernel7l.img was not created"
    exit 1
fi

# Create temporary directories
mkdir -p sdcard
mkdir -p kernels

# Copy kernel image
cp ./src/kernel*.img ./kernels/

# Copy boot files
cp -r ./circle-stdlib/libs/circle/boot/* sdcard/
rm -rf sdcard/config*.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux

# Copy configuration files
cp ./src/config.txt ./src/minidexed.ini ./src/performance.ini sdcard/

# Add cmdline.txt with USB speed setting
echo "usbspeed=full" > sdcard/cmdline.txt

# Copy kernel images to sdcard
cp ./kernels/* sdcard/ || true

# Get SysEx files
./getsysex.sh

# Create the zip file with date stamp
zip -r MiniDexed_build_$(date +%Y-%m-%d).zip sdcard/* kernels/*

# Cleanup temporary directories
rm -rf sdcard kernels

echo "Build files have been collected into MiniDexed_build_$(date +%Y-%m-%d).zip" 