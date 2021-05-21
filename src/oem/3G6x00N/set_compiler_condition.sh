#!/bin/sh
. ./define/PATH
. ./define/FUNCTION_SCRIPT

echo "********************************************************************************"
echo "*           START CONFIGURE COMPILER CONDITIONS                                *"
echo "********************************************************************************"

##########Add compiler condition by function.  Kyle 2008.08.01##########

#************************System Variable**********************************
INDEX=""
COMPILER_DEF_FILE="${ROOTDIR}/define/COMPILER_CFG.dat"
space=1
column_num=3
column_width=30
temp=0
#COUNT=0
#Clean DEF_LIST
	 > "$COMPILER_DEF_FILE"
#************************System Variable**********************************


#*************************Utility Call***********************************
function display_space
{
    i=0
    while [ $i -lt $1 ]; do
        echo -n " "
        i=`expr $i + 1`
    done
}

#*************************Function Call***********************************
#Author:    Kyle
#
#Date:      2008/08/01
#
#Describe:  Add compiler confition.
#
#Usage:     $1 Application catolog name $2 "y"=compiler other string=no comipler

#Example: 
#Input:  add_condition "ezview_upnp" "n" 
#Result = no compiler ezview_upnp tool.      
#
function add_condition
{
	if [ "$1" != "" ]; then
		if [ "$2" = "" ]; then
				INDEX="$INDEX $1;n"
				echo -n "["
                display_space $space
                
                if [ `expr $temp % $column_num` -eq `expr $column_num - 1` ]; then
                    echo "]$1"
                else
                    echo -n "]$1"
                    display_space `expr $column_width - $space - length $1 - 2`
                fi
                
		else
				INDEX="$INDEX $1;$2"
                echo -n "["
                display_space `expr $space - length $2`

                if [ "$2" = "y" ]; then
				    echo -n "*"
                elif [ "$2" = "n" ]; then
                    echo -n " "
                else
                    echo -n "$2"
                fi

                if [ `expr $temp % $column_num` -eq `expr $column_num - 1` ]; then
                    echo "]$1"
                else
                    echo -n "]$1"
                   display_space `expr $column_width - $space - length $1 - 2`
                fi
		fi
		temp=`expr $temp + 1`
	fi	
}

echo "INDEX=\"$INDEX\"" >> $COMPILER_DEF_FILE

echo ""
