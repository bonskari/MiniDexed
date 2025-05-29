#!/bin/bash

# Set RPI version if not set
if [ -z "${RPI}" ]; then
    echo "RPI version not set, defaulting to 4"
    export RPI=4
fi

# Build boot files first
echo "Building boot files..."
cd ./circle-stdlib/libs/circle/boot
make
if [ "${RPI}" -gt 2 ]
then
    make armstub64
fi
cd -

# Create base directory for SD card contents
mkdir -p sdcard

# Copy boot files with exact names
cp ./circle-stdlib/libs/circle/boot/bootcode.bin sdcard/
cp ./circle-stdlib/libs/circle/boot/start.elf sdcard/
cp ./circle-stdlib/libs/circle/boot/start_cd.elf sdcard/
cp ./circle-stdlib/libs/circle/boot/start4.elf sdcard/
cp ./circle-stdlib/libs/circle/boot/start4cd.elf sdcard/
cp ./circle-stdlib/libs/circle/boot/fixup.dat sdcard/
cp ./circle-stdlib/libs/circle/boot/fixup_cd.dat sdcard/
cp ./circle-stdlib/libs/circle/boot/fixup4.dat sdcard/
cp ./circle-stdlib/libs/circle/boot/fixup4cd.dat sdcard/

# Copy DTB files
cp ./circle-stdlib/libs/circle/boot/bcm2710-rpi-zero-2-w.dtb sdcard/
cp ./circle-stdlib/libs/circle/boot/bcm2711-rpi-4-b.dtb sdcard/
cp ./circle-stdlib/libs/circle/boot/bcm2711-rpi-400.dtb sdcard/
cp ./circle-stdlib/libs/circle/boot/bcm2711-rpi-cm4.dtb sdcard/
cp ./circle-stdlib/libs/circle/boot/bcm2712-rpi-5-b.dtb sdcard/

# Copy ARM stub
cp ./circle-stdlib/libs/circle/boot/armstub8-rpi4.bin sdcard/

# Copy kernel images
cp ./src/kernel*.img sdcard/

# Copy configuration files
cp ./src/config.txt sdcard/
cp ./src/minidexed.ini sdcard/
cp ./src/performance.ini sdcard/

# Create cmdline.txt
echo "usbspeed=full" > sdcard/cmdline.txt

# Copy LICENCE file
cp ./circle-stdlib/libs/circle/boot/LICENCE.broadcom sdcard/

# Create performance directory and copy files
mkdir -p sdcard/performance
# Note: Add any performance-specific files here if needed

# Create the zip file with date stamp
zip -r MiniDexed_build_$(date +%Y-%m-%d).zip sdcard/*

# Cleanup
rm -rf sdcard

echo "Build files have been collected into MiniDexed_build_$(date +%Y-%m-%d).zip" 