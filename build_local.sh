#!/bin/bash
set -e

# Default to RPi 4 if not specified
RPI=${RPI:-4}
echo "Building for Raspberry Pi ${RPI}"

# Create necessary directories
mkdir -p kernels sdcard

# Recursively pull git submodules
git submodule update --init --recursive

# Get particular git commits of the submodules
sh -ex submod.sh

# Install appropriate toolchain based on RPi version
if [ "${RPI}" -gt 2 ]; then
    # For RPi 3 and 4 (64-bit)
    if [ ! -f "gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz" ]; then
        wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
    fi
    tar xf gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
else
    # For RPi 1 and 2 (32-bit)
    if [ ! -f "gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz" ]; then
        wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz
    fi
    tar xf gcc-arm-10.3-2021.07-x86_64-arm-none-eabi.tar.xz
fi

# Add toolchain to PATH
export PATH=$(readlink -f ./gcc-*/bin/):$PATH

# Build dependencies and MiniDexed
./build.sh
cp ./src/kernel*.img ./kernels/

# Build Raspberry Pi boot files
cd ./circle-stdlib/libs/circle/boot
make
if [ "${RPI}" -gt 2 ]; then
    make armstub64
fi
cd -

# Prepare SD card contents
cd sdcard
../getsysex.sh
cd ..

# Copy boot files and configurations
cp -r ./circle-stdlib/libs/circle/boot/* sdcard/
rm -rf sdcard/config*.txt sdcard/README sdcard/Makefile sdcard/armstub sdcard/COPYING.linux
cp ./src/config.txt ./src/minidexed.ini ./src/*img ./src/performance.ini sdcard/
echo "usbspeed=full" > sdcard/cmdline.txt

# Copy kernel images
cd sdcard
cp ../kernels/* . || true

# Create dated zip file
zip -r ../MiniDexed_$(date +%Y-%m-%d).zip *
cd ..

echo "Build completed successfully!"
echo "The build files are available in MiniDexed_$(date +%Y-%m-%d).zip" 