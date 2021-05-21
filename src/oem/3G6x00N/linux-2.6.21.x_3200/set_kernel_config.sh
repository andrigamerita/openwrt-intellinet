#!/bin/sh

#Descript: Add General Kernel config file by function defined.
#Auth: Kyle
#Date: 2008/04/11



#**************************Configure Pattern END ********************



#Include Function Defined List.
. ../define/FUNCTION_SCRIPT


KERNEL_CONFIG=".config"
TEMP_CONFIG=".temp_config"
NEW_CONFIG=".new_config"
cp -f $KERNEL_CONFIG $NEW_CONFIG

#Author:    Kyle
#
#Date:      2008/09/24
#
#Describe:  字串取代
#
#Usage:     $1 old string  $2 new string 
#

function replaceConfigure
{
sed -e s/"$1"/"$2"/g $NEW_CONFIG > $TEMP_CONFIG
mv -f $TEMP_CONFIG $NEW_CONFIG
}



#********************************************************************************************************************
if [ "$_MEMSIZE_" = "32" ]; then
	replaceConfigure "# CONFIG_RT2880_DRAM_32M is not set" "CONFIG_RT2880_DRAM_32M=y"
	replaceConfigure "CONFIG_RT2880_DRAM_16M=y" "# CONFIG_RT2880_DRAM_16M is not set"
	replaceConfigure "CONFIG_RALINK_RAM_SIZE=16" "CONFIG_RALINK_RAM_SIZE=32"
else   
	replaceConfigure "# CONFIG_RT2880_DRAM_16M is not sett" "CONFIG_RT2880_DRAM_16M=y"
	replaceConfigure "CONFIG_RT2880_DRAM_32M=y" "# CONFIG_RT2880_DRAM_32M is not set"
	replaceConfigure "CONFIG_RALINK_RAM_SIZE=16" "CONFIG_RALINK_RAM_SIZE=16"
fi


if [ "$_RFTYPE_" = "2T2R" ]; then
	replaceConfigure "# CONFIG_RALINK_RT3052AP_2T2R is not set" "CONFIG_RALINK_RT3052AP_2T2R=y"
	replaceConfigure "CONFIG_RALINK_RT3050AP_1T1R=y" "# CONFIG_RALINK_RT3050AP_1T1R is not set"
elif [ "$_RFTYPE_" = "1T1R" ]; then
	replaceConfigure "# CONFIG_RALINK_RT3050AP_1T1R is not set" "CONFIG_RALINK_RT3050AP_1T1R=y"
	replaceConfigure "CONFIG_RALINK_RT3052AP_2T2R=y" "# CONFIG_RALINK_RT3052AP_2T2R is not set"
fi

if [ "$_CARRIER_" = "y" ]; then
	replaceConfigure "# CONFIG_RT2860V2_AP_CARRIER is not set" "CONFIG_RT2860V2_AP_CARRIER=y"
	replaceConfigure "# CONFIG_RALINK_TIMER_DFS is not set" "CONFIG_RALINK_TIMER_DFS=y"
else
	replaceConfigure "CONFIG_RT2860V2_AP_CARRIER=y" "# CONFIG_RT2860V2_AP_CARRIER is not set"
	replaceConfigure "CONFIG_RALINK_TIMER_DFS=y" "# CONFIG_RALINK_TIMER_DFS is not set"
fi

if [ "$_CONNECTION_CTRL_" = "y" ]; then
	replaceConfigure "# CONFIG_IP_NF_MATCH_CONNLIMIT is not set" "CONFIG_IP_NF_MATCH_CONNLIMIT=m"
else
	replaceConfigure "CONFIG_IP_NF_MATCH_CONNLIMIT=m" "# CONFIG_IP_NF_MATCH_CONNLIMIT is not set"
fi

if [ "$_WL_STA_DRIVER_" = "y" ] && [ "$_WIRELESS_DRIVER_VERSION_" = "19" ]; then
	replaceConfigure "# CONFIG_RT2860V2_STA is not set" "CONFIG_RT2860V2_STA=m"
	replaceConfigure "# CONFIG_RT2860V2_STA_RBUS is not set" "CONFIG_RT2860V2_STA_RBUS=y"
	echo "# CONFIG_RT2860V2_STA_2850 is not set" >> $NEW_CONFIG
	echo "# CONFIG_RT2860V2_STA_LED is not set" >> $NEW_CONFIG
	echo "# CONFIG_RT2860V2_STA_WPA_SUPPLICANT is not set" >> $NEW_CONFIG
	echo "# CONFIG_RT2860V2_STA_WSC is not set" >> $NEW_CONFIG
	echo "# CONFIG_RT2860V2_STA_DPB is not set" >> $NEW_CONFIG
	echo "# CONFIG_CONFIG_RT2860V2_STA_CARRIER is not set" >> $NEW_CONFIG
	echo "# CONFIG_CONFIG_RT2860V2_STA_DLS is not set" >> $NEW_CONFIG
fi


#********************************************************************************************************************

CMP_CONFIG=`cmp $KERNEL_CONFIG $NEW_CONFIG`
if [ "$CMP_CONFIG" != "" ]; then
	cp -f $NEW_CONFIG $KERNEL_CONFIG
	rm -f include/asm/asm-mips
   	#make config < enter

fi 

echo "Set kernel config file successfully!"    
