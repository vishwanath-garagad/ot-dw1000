#!/bin/bash
#******************************************************************#
# Author     : Nishal
# Date       : 2nd June 2017
# File       : setup.sh
# Version    : 0.1
# Description: This script does the setup for the system to work
#              with DW1000 examples with OpenThread stack
#
#******************************************************************#
#Initialising the necessary variables required for this script
DIR=$(pwd)
OT_COMMIT_ID=915261b36b256234ef6b40f6cbad69f036851385

CLI_TOOL_URL=https://www.nordicsemi.com/eng/nordic/download_resource/58852/21/81185512/94917
CLI_TOOL_FILE=nRF5x-Command-Line-Tools_9_3_1_Linux-x86_64.tar

PATCH_FILE=ot-dw1000.patch

WPAN_URL=https://github.com/openthread/wpantund/archive/master.zip
WPAN_FILE=wpantund-master.zip
WPAN_DIR=wpantund-master


echo ""
echo "<SCRIPT_LOG> Current directory $DIR"

if [ "$#" -ne  "0" -a "$1" == "INITIAL" ]; then
    echo "<SCRIPT_LOG> Setting up the dependencies for project..."
    #Install dependent librarires
    echo "<SCRIPT_LOG> Installing dependent librarires..."
    sudo apt-get install lib32z1 lib32ncurses5 lib32bz2-1.0

    #Install GCC ARM Embedded tool chain
    echo "<SCRIPT_LOG> Installing GCC ARM Embedded tool chain..."
    sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
    sudo apt-get update
    sudo apt-get install gcc-arm-embedded

    #Install wpantund
    echo "<SCRIPT_LOG> Installing wpantund..."
    sudo apt-get install dbus libreadline
    sudo apt-get install gcc g++ libdbus-1-dev libboost-dev libreadline-dev
    sudo apt-get install libtool autoconf autoconf-archive

    mkdir tools
    echo "<SCRIPT_LOG> Downloading package for wpantund..."
    #Download the package if not available
    if [ -f $DIR/tools/$WPAN_FILE ]; then
       echo "<SCRIPT_LOG> $WPAN_FILE already exists..."
    else
       cd tools
       echo "<SCRIPT_LOG> Downloading $WPAN_FILE ..."
       wget -O $WPAN_FILE $WPAN_URL
       cd ..
    fi

    #Extract the file if not done
    if [ -d $DIR/tools/$WPAN_DIR ]; then
       echo "<SCRIPT_LOG> $WPAN_DIR already exists..."
       echo "<SCRIPT_LOG> Extract skipped as directory already exists."
    else
       cd tools
       echo "<SCRIPT_LOG> Extracting $WPAN_FILE ..."
       unzip $WPAN_FILE
       cd $WPAN_DIR
       ./bootstrap.sh
       ./configure --sysconfdir=/etc
       make
       sudo make install
       cd ../..
    fi

    #Installing Nordic Command Line tools, download the package
    if [ -f $DIR/tools/$CLI_TOOL_FILE ]; then
        echo "<SCRIPT_LOG> $CLI_TOOL_FILE already exists..."
    else
        cd tools
        echo "<SCRIPT_LOG> Downloading $CLI_TOOL_FILE ..."
        wget -O $CLI_TOOL_FILE $CLI_TOOL_URL
        cd ..
    fi

else
    echo "<SCRIPT_LOG> Setting up the dependencies for project not required..."
fi

#To be called in case of INITIAL or UPDATE
if [ "$#" -ne  "0" ]; then
if [ "$1" == "INITIAL" -o "$1" == "UPDATE" ]; then

    #clone the OT repo and respective commit id
    echo "<SCRIPT_LOG> Cloning OpenThread repository..."
    if [ -d "$OT_ROOT" ]; then
        echo "<SCRIPT_LOG> $OT_ROOT already exists."
        rm -rf $OT_ROOT
    fi
    echo "<SCRIPT_LOG> Cloning OpenThread(OT) code to $OT_ROOT directory."
    git clone https://github.com/openthread/openthread.git $OT_ROOT
    cd $OT_ROOT
    git checkout $OT_COMMIT_ID

    #Enable execute permission for the files
    chmod a+x third_party/nlbuild-autotools/repo/autoconf/ltmain.sh
    cd -

    #Installing Nordic Command Line tools
    #Extract the file if not preset for Nordic Command line tools
    if [ -d "$OT_ROOT" ]; then
        if [ -d "$OT_ROOT/../nrfjprog" -a -d "$OT_ROOT/mergehex" ]; then
            echo "<SCRIPT_LOG> Extract skipped as directory already exists..."
        else
            echo "<SCRIPT_LOG> Extracting $CLI_TOOL_FILE ..."
            tar -xvf $OT_ROOT/../tools/$CLI_TOOL_FILE -C $OT_ROOT
        fi
    else
        echo "<SCRIPT_LOG> Extracting $CLI_TOOL_FILE ..."
        tar -xvf $OT_ROOT/../tools/$CLI_TOOL_FILE -C $OT_ROOT
    fi

    #Create symlinks
    echo "<SCRIPT_LOG> Creating symbolic links..."
    sudo ln -s $DW_OT_ROOT/examples/platforms/dw1000 $OT_ROOT/examples/platforms/dw1000
    sudo ln -s $DW_OT_ROOT/examples/Makefile-nrf52840-dw1000 $OT_ROOT/examples/Makefile-nrf52840-dw1000
    sudo ln -s $DW_OT_ROOT/scripts $OT_ROOT/scripts
    sudo ln -s $DW_OT_ROOT/third_party/decawave $OT_ROOT/third_party/decawave
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_delay.h $OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_delay.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_drv_spi.h $OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_drv_spi.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_error.h $OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_error.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/drivers/sdk_common.h $OT_ROOT/third_party/NordicSemiconductor/drivers/sdk_common.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/drivers/sdk_config.h $OT_ROOT/third_party/NordicSemiconductor/drivers/sdk_config.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/drivers/sdk_errors.h $OT_ROOT/third_party/NordicSemiconductor/drivers/sdk_errors.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_drv_spi.c $OT_ROOT/third_party/NordicSemiconductor/drivers/nrf_drv_spi.c
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/hal/nordic_common.h $OT_ROOT/third_party/NordicSemiconductor/hal/nordic_common.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/hal/nrf_gpiote.h $OT_ROOT/third_party/NordicSemiconductor/hal/nrf_gpiote.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/hal/nrf_spi.h $OT_ROOT/third_party/NordicSemiconductor/hal/nrf_spi.h
    sudo ln -s $DW_OT_ROOT/third_party/NordicSemiconductor/hal/nrf_spim.h $OT_ROOT/third_party/NordicSemiconductor/hal/nrf_spim.h
    sudo ln -s $DW_OT_ROOT/tools/pyterm $OT_ROOT/tools/pyterm
    sudo ln -s $DW_OT_ROOT/doc/doxygen $OT_ROOT/doc/doxygen
    sync

    #Apply a patch for the makefile changes.
    echo "<SCRIPT_LOG> Applying patch $PATCH_FILE for the common files..."
    if [ -f $DIR/$PATCH_FILE ]; then
        echo "<SCRIPT_LOG> $DIR/$PATCH_FILE file exists"
        patch -p0 -b < $PATCH_FILE
    else
        echo "<SCRIPT_LOG> $DIR/$PATCH_FILE not found"
    fi
fi
fi

