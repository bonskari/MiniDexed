#!/bin/bash

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