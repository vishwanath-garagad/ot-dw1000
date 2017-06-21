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
if [ -d "$OT_ROOT/build" -o -d "$OT_ROOT/output" ]; then
    make clean -f examples/Makefile-nrf52840-dw1000
    find . -type f -name "Makefile.in" -delete
fi

# Run bootstrap
./bootstrap

# Build OT CLI and NCP Application for DW1000 Radio and Nordic NRF52840 as Platform
# ENALE_DW1000=1 defines the macro "OPENTHREAD_ENABLE_DW1000_RADIO"
# COAP=1 enables all the COAP related functionalites
# this macro is used to invoke all the DW1000 related apis for compilation.
COAP=1 ENABLE_DW1000=1 make -f $OT_ROOT/examples/Makefile-nrf52840-dw1000

# Create hex files for output binary files
arm-none-eabi-objcopy -O ihex $OT_ROOT/output/nrf52840/bin/ot-cli-ftd $OT_ROOT/output/nrf52840/bin/ot-cli-ftd.hex
arm-none-eabi-objcopy -O ihex $OT_ROOT/output/nrf52840/bin/ot-ncp-ftd $OT_ROOT/output/nrf52840/bin/ot-ncp-ftd.hex

# Toplevel Directory
cd -
