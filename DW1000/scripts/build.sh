#!/bin/bash
# ------------------------------------------------------------------------------------------------------------------
# @file     build.sh
# @brief    This script file is used to build the OT CLI and NCP Application
#           for DW1000 Radio and Nordic NRF52840 as MCU
#
# @attention
#
# Copyright 2017 (c) DecaWave Ltd, Dublin, Ireland.
#
# All rights reserved.
#

# Toplevel Directory
cd $OT_ROOT

# Remove the Output and Build related files
if [ -d "$OT_ROOT/output" ]; then
    rm -rf output/
fi
if [ -d "$OT_ROOT/build" ]; then
    rm -rf build/
fi

# Run bootstrap
./bootstrap

# Run configure
./configure

# Cleans up Makefile.in files
make distclean

# Build OT CLI and NCP Application for DW1000 Radio and Nordic NRF52840 as Platform
#ENALE_DW1000=1 defines the macro "OPENTHREAD_ENABLE_DW1000_RADIO"
#this macro is used to invoke all the DW1000 related apis for compilation.
ENABLE_DW1000=1 make -f $OT_ROOT/examples/Makefile-nrf52840-dw1000

# Create hex files for output binary files
arm-none-eabi-objcopy -O ihex $OT_ROOT/output/bin/arm-none-eabi-ot-cli-ftd $OT_ROOT/output/bin/arm-none-eabi-ot-cli-ftd.hex
arm-none-eabi-objcopy -O ihex $OT_ROOT/output/bin/arm-none-eabi-ot-ncp $OT_ROOT/output/bin/arm-none-eabi-ot-ncp.hex

# Toplevel Directory
cd -
