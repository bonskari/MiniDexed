export RPI=4

# Get particular git commits of the submodules
sh -ex submod.sh

# Download and install the correct toolchain for Raspberry Pi 4
TOOLCHAIN_FILE="gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz"
TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/${TOOLCHAIN_FILE}"

if [ ! -f "${TOOLCHAIN_FILE}" ]; then
    echo "Downloading toolchain..."
    wget "${TOOLCHAIN_URL}"
fi

if [ ! -d "gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf" ]; then
    echo "Extracting toolchain..."
    tar xf "${TOOLCHAIN_FILE}"
fi

# Add toolchain to PATH
export PATH=$PATH:$(pwd)/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf/bin

# Build the project
./build.sh
