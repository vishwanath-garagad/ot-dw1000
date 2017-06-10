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
#              3) Updating the path variables to the bashrc file
#
#******************************************************************#

#Get the real path of the script
BASEDIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

#Export the Environment variables.
OT_ROOT=$BASEDIR/openthread-master
DW_OT_ROOT=$BASEDIR/DW1000

#export OT_ROOT
#export DW_OT_ROOT

BASH_FILE=~/.bashrc

# Function to add the path variables to the bashrc file
add_env_path()
{
    path_exported=0
    ret_val=0
    eval bash_search_str="$1"
    eval bashrc_append="$2"

    #check if path is exported in bash
    cat $BASH_FILE | \
    { while read line; do
        if [[ $line == *$bash_search_str* ]]; then
            #check if the path exported is same as current path
            if [ "$line" ==  "$bashrc_append" ]; then
	            path_exported=1
            else
	            path_exported=1
                echo "$line was set before."
                #Update the new path
                echo "Updating $bashrc_append to the $BASH_FILE"
                sed -i "s%$line%$bashrc_append%g" $BASH_FILE
                ret_val=1
            fi
            break
        fi
    done

    #Append the path if not present in the file
    if [ $path_exported -eq 0 ]; then
        echo "Appending $bashrc_append to the $BASH_FILE"
        echo $bashrc_append >> $BASH_FILE
        ret_val=1
    fi
    return "$ret_val"
    }
}

#check if OT_ROOT is exported in bashrc
bashrc_txt="export OT_ROOT=$OT_ROOT"
search_txt="export OT_ROOT"
add_env_path "\${search_txt}" "\${bashrc_txt}"
retval1=$?
if [ $retval1 == 1 ]; then
    export OT_ROOT
fi

#check if DW_OT_ROOT is exported in bashrc
bashrc_txt="export DW_OT_ROOT=$DW_OT_ROOT"
search_txt="export DW_OT_ROOT"
add_env_path "\${search_txt}" "\${bashrc_txt}"
retval2=$?
if [ $retval2 == 1 ]; then
    export DW_OT_ROOT
fi

