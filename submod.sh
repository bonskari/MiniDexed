#!/bin/bash
set -ex
#
# Update top-level modules as a baseline
git submodule update --init --recursive
#
# Use fixed master branch of circle-stdlib then re-update
cd circle-stdlib/
git checkout 3bd135d
git submodule update --init --recursive
cd -
#
# Optional update submodules explicitly
cd circle-stdlib/libs/circle
git checkout tags/Step49
cd -
cd circle-stdlib/libs/circle-newlib
#git checkout develop
cd -
#
# Use fixed master branch of Synth_Dexed
cd Synth_Dexed/
git checkout 65d8383ad5
cd -
