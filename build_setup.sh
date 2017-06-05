#!/bin/bash
#******************************************************************#
# Author     : Nishal
# Date       : 5th June 2017
# File       : build_setup.sh
# Version    : 0.1 
# Description: This script does the following thing
#              1)setup for the system to work when called with INITIAL
#              2)git clone of opensource repo when called with UPDATE
#              3)builds an application 
#              
#******************************************************************#

#Export environment variable
source setenv.sh

if [ "$#" -ne  "0" -a "$1" == "INITIAL" ]; then
    cd $DW_OT_ROOT/..
    pwd
    ./setup.sh $1
    cd -
elif [ "$#" -ne  "0" -a "$1" == "UPDATE" ]; then
    cd $DW_OT_ROOT/..
    pwd
    ./setup.sh $1
    cd -
fi

#Build and flash command for RTLS Ranging applicatio
$OT_ROOT/scripts/build.sh

