#!/bin/sh
#############################################################################
#					Include File                                                                            
	#!/bin/sh
	. ../../define/PATH
	. ../../define/FUNCTION_SCRIPT
	. ../../target.def
	if [ "${ROMFSDIR}" = "" ]; then
		echo "NO Defined PATH!"
		exit 0
	fi
#                                                                                                                                                                               
#############################################################################


#############################################################################
#					Create System Directory           
	                                                                   
	rm -rf ${ROMFSDIR}
	mkdir ${ROMFSDIR}
	mkdir ${ROMFSDIR}/dev
	mkdir ${ROMFSDIR}/bin
	mkdir ${ROMFSDIR}/sbin
	mkdir ${ROMFSDIR}/usr
	mkdir ${ROMFSDIR}/etc
	mkdir ${ROMFSDIR}/var
	mkdir ${ROMFSDIR}/proc
	mkdir ${ROMFSDIR}/tmp

	if [ "$_WAN3G_" = "y" ]; then
		mkdir ${ROMFSDIR}/sys
	fi
#                                                                                                                                                                               
#############################################################################

#############################################################################
#					Create Device Node                                                                              

	mknod -m666 ${ROMFSDIR}/dev/mtdblock0 b 31 0
	mknod -m666 ${ROMFSDIR}/dev/mtdblock1 b 31 1
	mknod -m666 ${ROMFSDIR}/dev/mtdblock2 b 31 2
	mknod -m666 ${ROMFSDIR}/dev/mtdblock3 b 31 3
	mknod -m666 ${ROMFSDIR}/dev/mtdblock4 b 31 4
	mknod -m666 ${ROMFSDIR}/dev/mtdblock5 b 31 5
	mknod -m666 ${ROMFSDIR}/dev/mtdblock6 b 31 6
	mknod -m666 ${ROMFSDIR}/dev/mtd0   c 90 0
	mknod -m666 ${ROMFSDIR}/dev/mtd0ro c 90 1
	mknod -m666 ${ROMFSDIR}/dev/mtd1   c 90 2
	mknod -m666 ${ROMFSDIR}/dev/mtd1ro c 90 3
	mknod -m666 ${ROMFSDIR}/dev/mtd2   c 90 4
	mknod -m666 ${ROMFSDIR}/dev/mtd2ro c 90 5
	mknod -m666 ${ROMFSDIR}/dev/mtd3   c 90 6
	mknod -m666 ${ROMFSDIR}/dev/mtd3ro c 90 7
	mknod -m666 ${ROMFSDIR}/dev/mtd4   c 90 8
	mknod -m666 ${ROMFSDIR}/dev/mtd4ro c 90 9
	mknod -m666 ${ROMFSDIR}/dev/mtd5   c 90 10
	mknod -m666 ${ROMFSDIR}/dev/mtd5ro c 90 11
	mknod -m666 ${ROMFSDIR}/dev/mtd6   c 90 12
	mknod -m666 ${ROMFSDIR}/dev/mtd6ro c 90 13
	#------------------System Node-----------------------#
	mknod -m666 ${ROMFSDIR}/dev/kmem 	c 	1 	2 	
	mknod -m666 ${ROMFSDIR}/dev/null 	c 	1 	3 	
	mknod -m666 ${ROMFSDIR}/dev/random c 1 8
	mknod -m666 ${ROMFSDIR}/dev/urandom 	c 1 9
	mknod -m600 ${ROMFSDIR}/dev/ptmx c 5 2
	mknod -m666 ${ROMFSDIR}/dev/ttyS0 c 4 64
	mknod -m666 ${ROMFSDIR}/dev/console c 5 1
	mknod -m666 ${ROMFSDIR}/dev/flash0	c 	200 	0
	mknod -m666 ${ROMFSDIR}/dev/rdm0 c   254   0  #for reg tool
	#------------------PPPoE Node-----------------------#
	mknod  -m666 ${ROMFSDIR}/dev/ptyp0 c 2 0
	mknod  -m666 ${ROMFSDIR}/dev/ttyp0 c 3 0
	mknod  -m666 ${ROMFSDIR}/dev/ttyS1 c 4 65
	mknod  -m666 ${ROMFSDIR}/dev/ppp c 108 0
	#------------------PPTP Node------------------------#
	mknod  -m666 ${ROMFSDIR}/dev/ptyp1 c 2 1
	mknod  -m666 ${ROMFSDIR}/dev/ptyp2 c 2 2
	mknod  -m666 ${ROMFSDIR}/dev/ptyp3 c 2 3
	mknod  -m666 ${ROMFSDIR}/dev/ttyp1 c 3 1
	mknod  -m666 ${ROMFSDIR}/dev/ttyp2 c 3 2
	mknod  -m666 ${ROMFSDIR}/dev/ttyp3 c 3 3
	#------------------GPIO Node------------------------#

	mknod -m666 ${ROMFSDIR}/dev/WLAN_LED0 c 240 "$_HW_LED_WIRELESS_"
	mknod -m666 ${ROMFSDIR}/dev/PowerLED  c 240 "$_HW_LED_POWER_"
	mknod -m666 ${ROMFSDIR}/dev/USB_LED0 c 240 "$_HW_LED_USB_"
	mknod -m666 ${ROMFSDIR}/dev/reset_but c 241 "$_HW_BUTTON_RESET_"
	mknod -m666 ${ROMFSDIR}/dev/wps_but c 241 "$_HW_BUTTON_WPS_"

#
#############################################################################



#############################################################################
#					Copy Share Library       
                                                                         
	mkdir ${ROMFSDIR}/lib
	cp /opt/buildroot-gcc342/lib/libuClibc-0.9.28.so ${ROMFSDIR}/lib/libc.so.0
	cp /opt/buildroot-gcc342/lib/ld-uClibc-0.9.28.so ${ROMFSDIR}/lib/ld-uClibc.so.0
	cp /opt/buildroot-gcc342/lib/libpthread-0.9.28.so ${ROMFSDIR}/lib/libpthread.so.0
	cp /opt/buildroot-gcc342/lib/libcrypt-0.9.28.so ${ROMFSDIR}/lib/libcrypt.so.0
	cp /opt/buildroot-gcc342/lib/libdl-0.9.28.so ${ROMFSDIR}/lib/libdl.so.0
	cp /opt/buildroot-gcc342/lib/libutil-0.9.28.so ${ROMFSDIR}/lib/libutil.so.0
	cp /opt/buildroot-gcc342/lib/libresolv-0.9.28.so ${ROMFSDIR}/lib/libresolv.so.0 #for "rdisc
	cp /opt/buildroot-gcc342/lib/libnsl-0.9.28.so ${ROMFSDIR}/lib/libnsl.so.0 #for appletalk
	cp /opt/buildroot-gcc342/lib/libm-0.9.28.so ${ROMFSDIR}/lib/libm.so.0
	cp ${APPDIR}/wireless_tools.29/libiw.so.29 ${ROMFSDIR}/lib
	
#                                                                                                                                                                               
#############################################################################

#############################################################################
#					Copy Application  to File System                                                                                  




		###########################################################
		#					Busy Box   , Bridge utlity
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:                                             
			cp -R -p ${APPDIR}/busybox-1.11.1/_install/* ${ROMFSDIR}
			cp ${APPDIR}/bridge-utils/brctl/brctl ${ROMFSDIR}/bin

		#                                                                                                                                      
		###########################################################
		
			
		###########################################################
		#					Script Files      
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:                                             

			#------------------Etc Script---------------------------#
			cp -R -p ${APPDIR}/etc.adm/* ${ROMFSDIR}/etc
			
			if [ "$_IS_GATEWAY_" = "y" ]; then		
				rm ${ROMFSDIR}/etc/profile.ap -f
			else
				mv -f ${ROMFSDIR}/etc/profile.ap ${ROMFSDIR}/etc/profile
			fi			
			rm ${ROMFSDIR}/etc/init.d/rcS.ath -f
			#cp ${APPDIR}/script/*.sh ${ROMFSDIR}/bin

		#                                                                                                                                      
		###########################################################

		###########################################################
		#					Wireless Files           
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:    
		
			mkdir -p ${ROMFSDIR}/etc/Wireless/RT2860
			#------------------Wireless Driver--------------------#
			if [ "$_RFTYPE_" = "1T1R" ]; then
				cp -f ${APPDIR}/wireless_driver/module/rt2860v2_ap_1T1R.ko ${ROMFSDIR}/bin/rt2860v2_ap.ko
			else
				cp -f ${APPDIR}/wireless_driver/module/rt2860v2_ap_2T2R.ko ${ROMFSDIR}/bin/rt2860v2_ap.ko
			fi

			#------------------Wireless Configure---------------#
			cp -f ${APPDIR}/wireless_driver/configure/RT2860AP.dat ${ROMFSDIR}/etc/Wireless/RT2860/
			#------------------Wireless EEPROM---------------#
			if [ "$_RFTYPE_" = "" ]; then 
				cp -f ${APPDIR}/wireless_driver/configure/SOC_AP_2T2R_V1_0.bin ${ROMFSDIR}/etc/Wireless/RT2860/EEPROM_V1_1.bin
			else
				cp -f ${APPDIR}/wireless_driver/configure/SOC_AP_${_RFTYPE_}_V1_0.bin ${ROMFSDIR}/etc/Wireless/RT2860/EEPROM_V1_1.bin
			fi		
			#------------------802.1x Demand--------------------#
			cp -f ${APPDIR}/wireless_driver/802.1x/rt2860apd  ${ROMFSDIR}/bin
			#------------------ATE Demand-----------------------#
			cp -f ${APPDIR}/wireless_driver/ated/ated  ${ROMFSDIR}/bin
			#------------------WPS Upnp--------------------------#
			mkdir -p ${ROMFSDIR}/etc/xml
			cp -f ${APPDIR}//wireless_driver/wsc_upnp2/xml/* ${ROMFSDIR}/etc/xml
			cp -f ${APPDIR}//wireless_driver/wsc_upnp2/wscd ${ROMFSDIR}/bin
			#------------------Wireless Configure Tool-------#
			cp -f ${APPDIR}/wireless_tools.29/iwpriv ${ROMFSDIR}/bin
			#------------------WPS Tool---------------------------#
			cp -f ${APPDIR}/wireless_tools.29/wpstool ${ROMFSDIR}/bin
			#------------------Ralink Timer---------------------------#
			if [ "$_DFS_" = "y" ] || [ "$_CARRIER_" = "y" ] ; then
				cp -f ${APPDIR}/wireless_driver/module/rt_timer.ko ${ROMFSDIR}/bin/
			fi

		#                                                                                                       
		###########################################################
		###########################################################
		#					LAN Related Files            
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:    
			#------------------Switch IC Configure Tool------#
			cp ${APPDIR}/switch/switch ${ROMFSDIR}/bin
		#                                                                                                                                      
		###########################################################


		###########################################################
		#					Sntp clock      
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:                                             
			cp ${APPDIR}/clockspeed-0.62/sntpclock ${ROMFSDIR}/bin		
		#                                                                                                                                      
		###########################################################


		###########################################################
		#					DDNS        
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:                                            
			cp ${APPDIR}/ez-ipupdate-3.0.10/ez-ipupdate ${ROMFSDIR}/bin
		#                                                                                                                                      
		###########################################################

		###########################################################
		#					Default setting        
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:                            
			cp ${IMAGEDIR}/config.bin ${ROMFSDIR}/etc/config.bin
		#                                                                                                                                      
		###########################################################

		###########################################################
		#					System Files         
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:                                            
			echo ${_VERSION_} > ${ROMFSDIR}/etc/version
			echo ${_DATE_} >  ${ROMFSDIR}/etc/compiler_date
			cp -R -p ${APPDIR}/var/* ${ROMFSDIR}/var
		#                                                                                                                                      
		###########################################################

		###########################################################
		#					System Tools         
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:                            
			#------------------MII Tool-----------------------#
			cp -f ${APPDIR}/wireless_driver/mii_mgr_3200/mii_mgr  ${ROMFSDIR}/bin
			#------------------LLTD---------------------------#
			cp ${APPDIR}/lltd/lld2d ${ROMFSDIR}/bin
			#------------------GPIO Demand--------------#
			cp ${APPDIR}/gpio/gpio ${ROMFSDIR}/bin
			#------------------Flash Tool--------------------#	
			cp -f ${GOAHEADDIR}/LINUX/flash-gw ${ROMFSDIR}/bin/flash
		#                                                                                                                                      
		###########################################################





		###########################################################
		#					Web Files      
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:         
 
			mkdir ${ROMFSDIR}/web
			cp -Rf ${GOAHEADDIR}/web/* ${ROMFSDIR}/web
			cp -f ${GOAHEADDIR}/LINUX/webs-gw ${ROMFSDIR}/bin/webs

		#                                                                                                                                      
		###########################################################



		###########################################################
		#					NetBios Scan tool    
		#Author: Kyle
		#Date: 	xxxx/xx/xx
		#Describe:         
		
			cp -Rf ${APPDIR}/nbtscan-1.5.1a/nbtscan ${ROMFSDIR}/bin
			
		#                                                                                                                                      
		###########################################################



	if [ "$_IS_GATEWAY_" = "y" ]; then		
		###########################################################
		#					Router only                                                                          
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:    
		
				#######################################################
				#					Qos            
				#Author: xxxxx
				#Date: 	xxxx/xx/xx
				#Describe:    
					cp ${APPDIR}/iproute2-2.4.7/ip/ip ${ROMFSDIR}/bin
					cp ${APPDIR}/iproute2-2.4.7/tc/tc ${ROMFSDIR}/bin
				#                                                                                                       
				#######################################################
			
				#######################################################
				#					ALG
				#Author: xxxxx
				#Date: 	xxxx/xx/xx
				#Describe:                                             
					mkdir -p ${ROMFSDIR}/lib/modules/2.6.20
					cp -Rf ${APPDIR}/module/alg   ${ROMFSDIR}/lib/modules/2.6.20
		
				#                                                                                                                                      
				#######################################################
		
				#######################################################
				#					Kernel Module
				#Author:    Kyle
				#Date: 	2009/01/20
				#Describe: 收集Kernel相關的Module    
				#                                        
					if [ "$_CONNECTION_CTRL_" = "y" ]; then
						mkdir -p ${ROMFSDIR}/lib/modules/2.6.20/netfilter
						cp -f ${APPDIR}/module/netfilter/ipt_connlimit.ko   ${ROMFSDIR}/lib/modules/2.6.20/netfilter
					fi
		
				#                                                                                                                                      
				#######################################################
				
				#######################################################
				#					WAN Related Files            
				#Author: xxxxx
				#Date: 	xxxx/xx/xx
				#Describe:    
					#------------------DHCP Client-----------------------#
					mkdir ${ROMFSDIR}/usr/share
					mkdir ${ROMFSDIR}/etc/udhcpc
					cp ${APPDIR}/udhcpc-scripts/* ${ROMFSDIR}/etc/udhcpc
					#------------------DHCP Server----------------------#
					mkdir ${ROMFSDIR}/var/lib
					mkdir ${ROMFSDIR}/var/lib/misc
					#------------------DNS relay (dnrd)------------------#
					cp ${APPDIR}/dnrd-2.10/src/dnrd ${ROMFSDIR}/bin
					mkdir ${ROMFSDIR}/etc/dnrd
					#------------------PPPoE--------------------------------#
					mkdir -p ${ROMFSDIR}/usr/sbin
					ln -s /bin/pppd ${ROMFSDIR}/usr/sbin/pppd
					cp ${APPDIR}/ppp-2.4.4/chat/chat ${ROMFSDIR}/sbin
					cp ${APPDIR}/ppp-2.4.4/pppd/pppd ${ROMFSDIR}/bin
					#------------------PPPoE Passthrough-------------#
					if [ "$_PPPOE_PASSTHROUGH_" = "y" ]; then
						cp -Rf ${APPDIR}/rp-pppoe-3.5/src/pppoe-relay ${ROMFSDIR}/bin
					fi		
					#------------------ L2TP----------------------------------#
					mkdir ${ROMFSDIR}/etc/l2tp
					cp ${APPDIR}/l2tpd/l2tpd ${ROMFSDIR}/bin
					#------------------Telstra Big Pond-------------------#
					cp ${APPDIR}/bpalogin-2.0.2/bpalogin ${ROMFSDIR}/bin
					#------------------PPPD----------------------------------#
					touch ${ROMFSDIR}/etc/ppp/pap-secrets
					chmod 600 ${ROMFSDIR}/etc/ppp/pap-secrets
					touch ${ROMFSDIR}/etc/ppp/chap-secrets
					chmod 600 ${ROMFSDIR}/etc/ppp/chap-secrets
					#------------------PPTP----------------------------------#
					cp ${APPDIR}/pptp-1.31/pptp ${ROMFSDIR}/bin
					mkdir ${ROMFSDIR}/var/lock
					
				#                                                                                                       
				######################################################

				######################################################
				#					IGMP PROXY     
				#Author: Kyle
				#Date: 	xxxx/xx/xx
				#Describe:         
				                                 
					if [ "$_IGMP_PROXY_" = "y" ]; then
						# add igmp
						cp -Rf ${APPDIR}/igmpproxy/igmpproxy ${ROMFSDIR}/bin
					fi
					
				#                                                                                                                                      
				######################################################


				#######################################################
				#					Iptables
				#Author: xxxxx
				#Date: 	xxxx/xx/xx
				#Describe:                 
					cp ${APPDIR}/iptables-1.4.0rc1/iptables ${ROMFSDIR}/bin
				#                                                                                                                                      
				#######################################################
		
				#######################################################
				#					Upnp IGDl    
				#Author: Kyle
				#Date: 	xxxx/xx/xx
				#Describe:         
					mkdir ${ROMFSDIR}/usr/lib
					#mkdir ${ROMFSDIR}/etc/linuxigd
					#cp -Rf ${APPDIR}/upnp_adm/IGD2/etc/* ${ROMFSDIR}/etc/linuxigd
				#                                                                                                                                      
				#######################################################

		#
		###########################################################
		
	fi

		###########################################################
		#					Reg_Tool                                                                
		#Author: xxxxx
		#Date: 	xxxx/xx/xx
		#Describe:    
			cp ${APPDIR}/reg/reg ${ROMFSDIR}/bin
		#                                                                                                       
		###########################################################


		###########################################################
		#					Router Discover                                                       
		#Author: xxxx
		#Date: 	xxxx/xx/xx
		#Describe:    

			cp ${APPDIR}/iputils/rdisc ${ROMFSDIR}/bin
		#                                                                                                      
		###########################################################


		###########################################################
		#					3G Tools
		#Author: Bryan
		#Date: 	2008/09/23
		#Describe:                            
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB0 c 188 0
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB1 c 188 1
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB2 c 188 2
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB3 c 188 3
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB4 c 188 4
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB5 c 188 5
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB6 c 188 6
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB7 c 188 7
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB8 c 188 8
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB9 c 188 9
			mknod -m666 ${ROMFSDIR}/dev/ttyUSB10 c 188 10

			mknod -m666 ${ROMFSDIR}/dev/ttyACM0 c 166 0
			mknod -m666 ${ROMFSDIR}/dev/ttyACM1 c 166 1
			mknod -m666 ${ROMFSDIR}/dev/ttyACM2 c 166 2
			mknod -m666 ${ROMFSDIR}/dev/ttyACM3 c 166 3

			mknod -m666 ${ROMFSDIR}/dev/ttyHS0 c 245 0
			mknod -m666 ${ROMFSDIR}/dev/ttyHS1 c 245 1
			mknod -m666 ${ROMFSDIR}/dev/ttyHS2 c 245 2
			mknod -m666 ${ROMFSDIR}/dev/ttyHS3 c 245 3
			mknod -m666 ${ROMFSDIR}/dev/ttyHS4 c 245 4

			mknod -m666 ${ROMFSDIR}/dev/sr0 b 11 0
			mknod -m666 ${ROMFSDIR}/dev/sr1 b 11 1

			cp -f ${LINUXDIR}/drivers/usb/serial/usbserial.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/usb/class/cdc-acm.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/usb/net/usbnet.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/usb/net/asix.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/usb/net/mcs7830.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/usb/net/sierra_net.ko ${ROMFSDIR}/bin/

			cp -f ${LINUXDIR}/drivers/usb/serial/hso.ko ${ROMFSDIR}/bin/ # tommy
			cp -f ${LINUXDIR}/drivers/usb/net/rndis_host.ko ${ROMFSDIR}/bin/ # tommy
			cp -f ${LINUXDIR}/drivers/usb/net/cdc_ether.ko ${ROMFSDIR}/bin/ # tommy
			cp -f ${LINUXDIR}/drivers/usb/serial/option.ko ${ROMFSDIR}/bin/ # tommy
			cp -f ${LINUXDIR}/drivers/usb/serial/sierra.ko ${ROMFSDIR}/bin/ # tommy
			cp -f ${LINUXDIR}/drivers/usb/serial/pl2303.ko ${ROMFSDIR}/bin/ # tommy

			cp -f ${LINUXDIR}/drivers/usb/storage/usb-storage.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/scsi/scsi_mod.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/scsi/sr_mod.ko ${ROMFSDIR}/bin/
			cp -f ${LINUXDIR}/drivers/cdrom/cdrom.ko ${ROMFSDIR}/bin/
	
			cp -a ${APPDIR}/3G/usb_modeswitch-0.9.7/usb_modeswitch ${ROMFSDIR}/bin/

			cp -a ${APPDIR}/3G/tools/atc ${ROMFSDIR}/bin/
			cp -a ${APPDIR}/3G/tools/probeport ${ROMFSDIR}/bin/
			cp -a ${APPDIR}/3G/ttcp/ttcp ${ROMFSDIR}/bin/

			cp -a ${APPDIR}/3G/comgt.0.32/comgt ${ROMFSDIR}/bin/comgt
			mkdir ${ROMFSDIR}/usr/lib/gcom
			cp -a ${APPDIR}/3G/gcom/* ${ROMFSDIR}/usr/lib/gcom
			mkdir ${ROMFSDIR}/usr/lib/chat
			cp -a ${APPDIR}/3G/chat/* ${ROMFSDIR}/usr/lib/chat
			cp -a ${APPDIR}/3G/tools/modemctl ${ROMFSDIR}/bin/modemctl
			cp -a ${APPDIR}/3G/option_icon/ozerocdoff ${ROMFSDIR}/bin/ozerocdoff
			cp -a ${APPDIR}/3G/fping-2.4b2/fping ${ROMFSDIR}/bin
			echo "none	/proc/bus/usb	usbfs	defaults	0	0" >> ${ROMFSDIR}/etc/fstab

			cp -a ${APPDIR}/3G/tools/sendmail ${ROMFSDIR}/bin/

		#                                                                                                       
		###########################################################

		###########################################################
		#					Copy Function defined       
		#Author: Kyle
		#Date: 	xxxx/xx/xx
		#Describe:                            
			cp ${ROOTDIR}/define/FUNCTION_SCRIPT ${ROMFSDIR}/web/
		#                                                                                                       
		###########################################################


#		      
#					END Copy  Application 
#############################################################################




#############################################################################
#					Clean&Copy Temp Files             
#Author: xxxx
#Date: 	xxxx/xx/xx
#Describe:                                      
                                   
	cp -Rf ${ROMFSDIR}/dev ${ROMFSDIR}/dev.tmp
	cp -Rf ${ROMFSDIR}/etc ${ROMFSDIR}/etc.tmp
	echo "clean SVN....."
	find ${ROMFSDIR} -name .svn | xargs -i rm -rf {}
	chmod 777 ${ROMFSDIR}/bin/*.*
#                                                                                                                                                                               
#############################################################################



#############################################################################
#					Stript Files          
#Author: xxxx
#Date: 	xxxx/xx/xx
#Describe:     
                                                                    
	find ${ROMFSDIR}/* | xargs -i file {}  | grep "strip" | cut -f1 -d":" | xargs -r ${CROSS_COMPILE}strip -R .comment -R .note -g --strip-unneeded;
	
#                                                                                                                                                                               
#############################################################################
