#!/bin/bash
#******************************************************************#
# Author     : Nishal
# Date       : 5th June 2017
# File       : build.sh
# Version    : 0.1 
# Description: This script initializes the path as enviroment 
#              variables for the following
#              1) OT_ROOT(open source)
#              2) DW_OT_ROOT(DW specific)
#              
#******************************************************************#

#Get the real path of the script
BASEDIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

#Export the Environment variables.
OT_ROOT=$BASEDIR/openthread-master
DW_OT_ROOT=$BASEDIR/DW1000

export OT_ROOT
export DW_OT_ROOT

