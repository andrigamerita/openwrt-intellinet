#!/bin/sh

if [ $# -lt 1 ]; then echo "Usage: $0 root_dir ";  exit 1 ; fi

. ./define/PATH

##########Add dfined for special function.  Kyle 2008.01.15###########
#**************************Function Defined***************************
#================Templet: 0====================
#Author: Kyle
#Date: 	xxxx/xx/xx
#Describe: 
#	    Support XXXXX Function.
TEST="TEST"
#Flash Reserved: xx byte
F_TEST="6"

#================Function: 1====================
#Author:    Wise
#Date:      2008/01/25
#Describe: 
#           Auto Firmware Upgrade Notice Function ,implement by XML..
#
XMLUPG="XMLUPG"

#================Function: 2====================
#Author:    Wise
#Date:      2008/01/25
#Describe: 
#           Support MSSID function.
#
MSSID="MSSID"
F_MSSID="1302"
#================Function: 3====================
#Author:    Wise
#Date:      2008/01/25
#Describe: 
#           MSSID Max Number.
#
MSSIDNUM="MSSIDNUM"

#================Function: 4====================
#Author:    Kyle
#Date:      2007/12/14
#Describe: 
#	     Support NetBios Name replaced IP address.
#          支援NetBios Scan功能,可以掃到同一個子網段中連接線上的電腦List.
#          可收集到電腦名稱,MAC位址,IP位址 等三項資訊.
#          目前應用於一些需要設定IP或MAC的UI上,列如Port Forwarding與DMZ..等等
#          可以改用選電腦名稱的方式進行設定,送出Cgi Form時,會自動帶入該名稱的IP或MAC.
#
NETBIOSNAME="NETBIOSNAME"

#================Function: 5====================
#Author: Kyle
#Date: 	2008/01/29
#Describe: 
#	     Support Dynamic setting ALG FTP Port.
#       FTP ALG原先只能對應Port 21. 開此Defined後,需同時修改alg.asp頁面
#          於FTP欄位旁會多出一個可以更換Port位址的欄位,可由使用者指定目前FTP Server
#         開的Port號碼.
#
DALGFTP="DALGFTP"
F_DALGFTP="2"

#================Function: 6====================
#Author: Kyle
#Date: 	2008/02/12
#Describe: 
#	     Memorry SIZE
#	     Memorry BUS   
#         決定硬體的記憶體大小與滙流排定址線長度.
#
MEMSIZE="MEMSIZE"
MEMBUS="MEMBUS"
#================Function: 7====================
#Author: Kyle
#Date: 	2008/02/12
#Describe: 
#	     指定RF是幾根傳送與幾根接收.
#Value=  1T1R or 2T2R or 2T3R
#
RFTYPE="RFTYPE"

#================Function: 8====================
#Author: Kyle
#Date: 	2008/02/15
#Describe: 
#	     Support VLAN ID Function(Base on MSSID)
#
ENVLAN="ENVLAN"
F_ENVLAN="14"
#================Function: 9====================
#Author: Wise
#Date: 	2008/02/19
#Describe: 
#	     For Edimax EZ View (per 52)
#
EZVIEW="EZVIEW"
#F_EZVIEW="9403"
F_EZVIEW="7802"
#================Function: 10====================
#Author: Kyle
#Date: 	2008/02/29
#Describe: 
#	    Reboot system via kernel for web firmware upgrade function.
#
KREBOOT="KREBOOT"
NO_KREBOOT="NO_KREBOOT"
#================Function: 11(wireless driver)====================
#Author: Wise
#Date: 	2008/03/07
#Describe: 
#		Wireless client login/logout Log.
#
WLCIOLOG="WLCIOLOG"

#================Function: 12====================
#Author: Wise
#Date: 	2008/03/10
#Describe: 
#		Don't display type ( Infrastructure / Adhoc ) when Site Survey
#
SS_NO_TYPE="SS_NO_TYPE"

#================Function: 13====================
#Author: Kyle
#Date: 	2008/03/11
#Describe: 
#	    Support Dual Band.

DBAND="DBAND"
#Flash Reserved: 6 byte
F_DBAND="6"

#================Function: 14====================
#Author: Kyle
#Date: 	2008/03/13
#Describe: 
#	    Support Carrier Detect.
#
CARRIER="CARRIER"

#================Function: 15(wireless driver)====================
#Author: Kyle
#Date: 	2008/03/13
#Describe: 
#	    Support DFS(Dynamic Frequency Selection) .
#        此功能目前只有開在日本A Band的產品.主要用來避免搶到軍用雷達波
#        開此Defined後,會自動偵測是否環境中有軍用雷達波,若有,則自動跳
#      channel 避掉該雷達波,並於30分鐘內不得在跳回原先跳離的Channel. 
#   
DFS="DFS"

#================Function: 16====================
#Author: Kyle
#Date: 	2008/03/17
#Describe: 
#	    Auto generate WPA Key from mac address for first use WPS function.
#        此項Defined目前只有Jensen在使用.為Jensen的特殊功能,主要是在首次使用WPS時會自動產測一組
#        使用MAC位置算出長度為8碼的Key.並將該組WPA2AES Key做為Profile傳送給目前正在進行WPS的Client.
AUTOWPA="AUTOWPA"

#================Function: 17====================
#Author: Kyle
#Date: 	2008/03/17
#Describe: 
#	    Support Hardway AP/Router Switch
#
ARSWITCH="ARSWITCH"
#Author: Kyle
#Date: 	2008/10/27
#Describe: 
#	    AP/Router Switch開關動作反相
#
ARSWITCH_REVERSE="ARSWITCH_REVERSE"
#================Function: 18====================
#Author: Kyle
#Date: 	2008/03/17
#Describe: 
#	    Support SNMP
#
ENSNMP="ENSNMP"
F_ENSNMP="32"
#================Function: 19====================
#Author: Kyle
#Date: 	2008/03/17
#Describe: 
#	    Support RIP
#
ENRIP="ENRIP"
F_ENRIP="1"
#================Function: 20====================
#Author: Kyle
#Date: 	2008/03/25
#Describe: 
#	    Support External Radius Server
#
EXTRADIUS="EXTRADIUS"

#================Function: 21====================
#Author: Kyle
#Date: 	2008/03/26
#Describe: 
#	    Support Multi PPPOE.
MPPPOE="MPPPOE"
#Flash Reserved: 0 byte
F_MPPPOE="218"

#================Function: 22====================
#Author: Wise
#Date: 	2008/03/26
#Describe: 
#	    Support Clear-Net DDNS
#
CLEARNET="CLEARNET"

#================Function: 23====================
#Author: Kyle
#Date: 	2008/03/26
#Describe: 
#	    OK Message support count down function.
OKMSG_COUNTDOWN="OKMSG_COUNTDOWN"

#================Function: 24====================
#Author: Wise
#Date: 	2008/03/27
#Describe: 
#	    Support IPv6 Bridge function.  Check WAN interface name in linux-2.4.x/net/bridge/br_input.c before use this
IPV6_BRIDGE="IPV6_BRIDGE"
F_IPV6_BRIDGE="1"
#================Function: 25====================
#Author: Kyle
#Date: 	2008/03/26
#Describe: 
#	    Support LAN<->WAN access control function.
#	    This function base on Multiple SSID function.
#      305x修改後,LAN WAN ACCESS從SSID0開始算起
LAN_WAN_ACCESS="LAN_WAN_ACCESS"
F_LAN_WAN_ACCESS="4"


#================Function: 27====================
#Author: Wise
#Date: 	2008/04/01
#Describe: 
#	    PPPoE Passthrough
PPPOE_PASSTHROUGH="PPPOE_PASSTHROUGH"
F_PPPOE_PASSTHROUGH="1"

#================Function: 28====================
#Author: Wise
#Date: 	2008/04/08
#Describe: 
#	    PPTP FQDN
PPTP_FQDN="PPTP_FQDN"
F_PPTP_FQDN="27"

#================Function: 29====================
#Author: Bryan
#Date: 	2008/04/08
#Describe: 
#	     Support ALG RTSP.
#
ALGRTSP="ALGRTSP"

#================Function: 30====================
#Author: Bryan
#Date: 	2008/04/08
#Describe: 
#	     Support TTL Setting
#
WANTTL="WANTTL"
F_WANTTL="3"

#================Function: 31====================
#Author: Kyle
#Date: 	2008/04/09
#Describe: 
#	     Support Null user name and password to login web.
#
NULL_LOGIN="NULL_LOGIN"

#================Function: 32====================
#Author: Kyle
#Date: 	2008/04/10
#Describe: 
#	     Support EZ View Upnp XML Tag.
#
EZ_XML_TAG="EZ_XML_TAG"


#================Function: 33====================
#Author: Kyle
#Date: 	2008/04/10
#Describe: 
#	     Definde Model Name.
#
MODEL_NAME="MODEL_NAME"

#================Function: 34====================
#Author: Kyle
#Date: 	2008/04/15
#Describe: 
#	     Independ wps button
#
WPS_INDEPEND="WPS_INDEPEND"

#================Function: 35====================
#Author: Kyle
#Date: 	2008/04/15
#Describe: 
#	     Reverse wps and reset h/w button loaction.
#
REVERSE_BUTTON="REVERSE_BUTTON"


#================Function: 36====================
#Author: Kyle
#Date: 	2008/04/21
#Describe: 
#	     Set TX CLK and RX ClK = no delay.
#
WIRELESS_TRUBO="WIRELESS_TRUBO"

#================Function: 37====================
#Author: Kyle
#Date: 	2008/04/22
#Describe: 
#	     SSID_MAC.
#
SSID_MAC="SSID_MAC"

#================Function: 38====================
#Author: Kyle
#Date: 	2008/04/23
#Describe: 
#	     for Logitec pppoe connection issue.
#
PATD="PATD"

#================Function: 39====================
#Author: Wise
#Date: 	2008/04/22
#Describe: 
#	     Enable WPS LED
#
WPS_LED="WPS_LED"


#================Function: 40====================
#Author: Kyle
#Date: 	2008/04/23
#Describe: 
#	     Support Router Discover.
#
RDISC="RDISC"

#================Function: 41(wireless driver)====================
#Author: Wise
#Date: 	2008/04/23
#Describe: 
#	     Generate a (13 ASCII) WEP key from MAC 
#
WEP_MAC="WEP_MAC"
#================Function: 42====================
#Author: Kyle
#Date: 	2008/04/29
#Describe: 
#	     NoForwarding between bssid.
#
NOFORWARDBTSSID="NOFORWARDBTSSID"

#================Function: 43====================
#Author: Wise
#Date: 	2008/04/30
#Describe: 
#	     VLAN for AP, change vlan packets forwarding path in kernel, code by Rex
#
VLAN_AP="VLAN_AP"


#================Function: 44====================
#Author: Kyle
#Date: 	2008/05/01
#Describe: 
#	        DHCPC Debug ON.
#
DEBUG_DHCPC="DEBUG_DHCPC"


#================Function: 44====================
#Author: Wise
#Date: 	2008/05/01
#Describe: 
#	     Define Modual Name
#
MODUAL_NAME="MODUAL_NAME"

#================Function: 45====================
#Author: Wise
#Date: 	2008/05/02
#Describe: 
#	     Defined if it is a gateway 
#
IS_GATEWAY="IS_GATEWAY"


#================Function: 46====================
#Author: Kyle
#Date: 	2008/05/06
#Describe: 
#	     Support WiFi Test 
#
WIFI_SUPPORT="WIFI_SUPPORT"


#================Function: 47====================
#Author: Wise
#Date: 	2008/05/12
#Describe: 
#	     WLAN_LED0 = LED ON          when wireless signal >=50
#     WLAN_LED0 = LED BLINK 65535  when wireless signal <=49
DISPLAY_SIGNAL_STRENGTH="DISPLAY_SIGNAL_STRENGTH"

#================Function: 48====================
#Author: Kyle
#Date: 	2008/05/20
#Describe: 
#	     Support Wireless H/W switch.
#
WIRELESS_SWITCH="WIRELESS_SWITCH"
#================Function: 48.5====================
#Author: Wise
#Date: 	2008/10/14
#Describe: 
#	     Support Wireless H/W switch. ON/OFF反向
#
WIRELESS_SWITCH_REVERSE="WIRELESS_SWITCH_REVERSE"
#================Function: 49====================
#Author: Kyle
#Date: 	2008/05/21
#Describe: 
#	     WPS_NO_BROADCAST.
#
WPS_NO_BROADCAST="WPS_NO_BROADCAST"
#================Function: 50====================
#Author: Wise
#Date: 	2008/05/20
#Describe: 
#	     when APMODE=1 or 2  , DON'T set encryption flash to 0, only set from script
RESERVE_ENCRYPTION_SETTING="RESERVE_ENCRYPTION_SETTING"

#================Function: 51====================
#Author: Kyle
#Date: 	2008/05/27
#Describe: 
#	     Support IGMP Function.
#Effect:
#      Script(linux config, wireless config file).
WIRELESS_IGMPSNOOP="WIRELESS_IGMPSNOOP"



#================Function: 52(wireless driver)====================
#Author: Wise
#Date: 	2008/05/21
#Describe: 
#         Repeater status in /proc/repeater_stat   0=disconnected 1=connected
#	     
REPEATER_STAT="REPEATER_STAT"


#================Function: 53====================
#Author: Wise
#Date:  2008/06/10
#Describe: 
#         MTUs of <Dynamic IP> and <PPPOE> use different flash  
#        
INDEPEND_DHCP_MTU="INDEPEND_DHCP_MTU"
F_INDEPEND_DHCP_MTU="2"


#================Function: 54====================
#Author: Wise
#Date:  2008/06/16
#Describe: 
#         Tag is embedded in each page. First used in BR6524N Hawking RN1A and EW7416APN Hawking REN1
#         This will define 2 variables, <main_menu_number> and <sub_menu_number>, so that CGI pages can have menu headers in it. 
TAG_IN_PAGE="TAG_IN_PAGE"

#================Function: 55====================
#Author: Wise
#Date:  2008/06/16
#Describe: 
#        OK_MSG charset will be iso-8859-1 if this is defined.
#        If not defined, charset will be utf-8
OK_MSG_CHARSET_ISO_8859_1="OK_MSG_CHARSET_ISO_8859_1"


#================Function: 56====================
#Author: Wise
#Date:  2008/06/20
#Describe: 
#        Add Timezone function in AP
#        time maybe used in log or something else.
AP_WITH_TIMEZONE="AP_WITH_TIMEZONE"
F_AP_WITH_TIMEZONE="15"


#================Function: 57====================
#Author: Wise
#Date:  2008/06/23
#Describe: 
#        DHCP table  ->  IP list table (static and dhcp)
IP_LIST_TABLE="IP_LIST_TABLE"


#================Function: 58====================
#Author: Wise
#Date:  2008/06/27
#Describe: 
#        Watch Dog
WATCH_DOG="WATCH_DOG"
F_WATCH_DOG="7"


#================Function: 59====================
#Author: Wise
#Date:  2008/06/27
#Describe: 
#        AP also has WLAN No Forwafing function
AP_WLAN_NOFORWARD="AP_WLAN_NOFORWARD"


#================Function: 60====================
#Author: Kyle
#Date:  2008/06/23
#Describe: 
#        Blank power led when software rebot router .
SREBOOT_BLINK_POWER="SREBOOT_BLINK_POWER"
F_SREBOOT_BLINK_POWER="1"

#================Function: 61====================
#Author: Kyle
#Date:  2008/07/02
#Describe:
#        Support PPTP Passthrough .
PPTP_PASSSTHROUGH="PPTP_PASSSTHROUGH"

#================Function: 62====================
#Author: Kyle
#Date:  2008/07/02
#Describe:
#        Change WPS H/W button trigger condition(for Corega). 
#				   Push wps button and hold within 2~5 second then release button.
#						
COREGA_WPS_TRIGGER_CONDITION="COREGA_WPS_TRIGGER_CONDITION"

#================Function: 63====================
#Author: Kyle
#Date:  2008/07/06
#Describe:
#        Support IGMP Proxy Function.
#						
IGMP_PROXY="IGMP_PROXY"

#================Function: 64====================
#Author: Kyle
#Date:  2008/07/18
#Describe:
#        修正flash.inc中存在特殊字元,導至include 至shell script時造成無法順利開完機的問題.
#						
SPECIAL_CHAR_FILTER_IN_SCRIPT="SPECIAL_CHAR_FILTER_IN_SCRIPT"

#================Function: 65====================
#Author: Kyle
#Date: 	2008/07/23
#Describe: 
#	     Support IGMP Function.
IGMPSNOOP="IGMPSNOOP"

#================Function: 66====================
#Author: Kyle
#Date:  2008/07/24
#Describe:
#        Defined H/W GPIO Pin.
#          對應硬體的GPIO Pin腳位置,可依不同的客戶或硬體,於Build時填入正確的GPIO腳位.
#
if [ "${1}" = "3G6230N" ]; then
	HW_LED_POWER="HW_LED_POWER"
	HW_LED_WIRELESS="HW_LED_WIRELESS"
	HW_LED_USB="HW_LED_USB"
	HW_BUTTON_RESET="HW_BUTTON_RESET"
	HW_BUTTON_WPS="HW_BUTTON_WPS"
	HW_ROUTER_SW="HW_ROUTER_SW"
	HW_AP_SW="HW_AP_SW"
	HW_CLIENT_SW="HW_CLIENT_SW"
	HW_USB_PWR="HW_USB_PWR"
else
	HW_LED_POWER="HW_LED_POWER"
	HW_LED_WIRELESS="HW_LED_WIRELESS"
	HW_LED_USB="HW_LED_USB"
	HW_LED_WPS="HW_LED_WPS"
	HW_BUTTON_RESET="HW_BUTTON_RESET"
	HW_BUTTON_WPS="HW_BUTTON_WPS"
	HW_BUTTON_SWITCH="HW_BUTTON_SWITCH"
fi

#================Function: 67====================
#Author: Kyle
#Date: 	2008/08/14
#Describe: 
#	     Swap Tx , Rx switch
SWAP_TR_SWITCH="SWAP_TR_SWITCH"


#================Function: 68====================
#Author: Kyle
#Date: 	2008/08/13
#Describe: 
#	     WPS default key改為長度8 ascii字元.
WPS_SHORT_KEY="WPS_SHORT_KEY"

#================Function: 69====================
#Author: Wise
#Date:  2008/08/13
#Describe:
#       WEB Upgrade進度：在formUpload做完大部分firmware的判斷，並且準備把goahead存在記憶體的firmware寫到flash之前，
#                        先fork出來丟個倒數的頁面，倒數完剛好開完機。 
#                       
UPGRADE_PROCESS="UPGRADE_PROCESS"
#================Function: 70===================
#Author: Kyle
#Date:  2008/08/27
#Describe:
#       Upgrade檔案Header所使用的tag。將會帶入cvimg.c中，防止不同Model之間Firmware互相Upgrade.
#       Tag 長度限制為4個字元                     
WEB_HEADER="WEB_HEADER"

#================Function: 71====================
#Author: Kyle
#Date:  2008/08/27
#Code:
#Describe:
#        Platform 相關的一些資訊,此Defined內容會在Build時自動產測填入,
#						
MODEL="MODEL"
MODE="MODE"
VERSION="VERSION"
DATE="DATE"
PLATFORM="PLATFORM"
PRODUCT_NAME="PRODUCT_NAME"
#================Function: 72===================
#Author: Wise
#Date:  2008/09/02
#Describe:
#        隱藏的CGI產測頁面 http://xxx.xxx.xxx.xxx/goform/mp
#						
HIDING_CGI_MP_PAGE="HIDING_CGI_MP_PAGE"
#================Function:73====================
#Author: Wise
#Date: 	2008/10/20
#Describe: 
#	     PCI 用 UPnP Configure
#         回應的內容  ssdp_device.c   CreateServicePacket 內。   (ssdp_server.c) 從6524 port過來
PCI_UPNP_CONFIGURE="PCI_UPNP_CONFIGURE"


#================Function:74====================
#Author: Kyle
#Date: 	2008/10/20
#Code: 
#				Modify:			goahead/LINUX/apform.h fmget.c fmwlan.c main.c
#				Add:					AP/script/wanprob.sh
#Describe: 
#	     WAN 端連線模式自動偵測精靈，從BR6524N port過來.
AUTO_WAN_PROB="AUTO_WAN_PROB"


#================Function: 75====================
#Author: vance
#Date:  2008/10/20
#Code:  /linux-2.6.21.x/drivers/net/wireless/rt2860v2/os/linux/ap_ioctl.c
#	    /linux-2.6.21.x/drivers/net/wireless/rt2860v2/include/rtmp.h  /linux-2.6.21.x/drivers/net/wireless/rt2860v2/include/oid.h
#	    /AP/script/interrupt.sh	/AP/script/wlan.sh	/AP/script/wps.sh	 /AP/script/scriptlib.sh
#Describe:	
#       按下wps按鈕3秒執行wps功能,超過10秒後執行自訂功能
#       自訂功能為關閉ra1的mac filter及顯示ssid，等待使用者連進來
#		使用者一旦連入，將mac加入acl list中，並開啟mac filter及隱藏ssid
#		若沒有使用者連入，90秒後自動結束功能				
HW_MACFILTER_IPHONE="HW_MACFILTER_IPHONE"

#================Function: 76====================
#Author: Kyle
#Date:  2008/10/22
#Code:    /AP/goahead/LINUX/webs.c
#Describe:
#			WEB登入時的提示字串,提示user Name與Password
#			若未defined則預設字串為:Default: admin/1234
#						
WEB_LOGIN_ALERT_STRING="WEB_LOGIN_ALERT_STRING"

#================Function: 77====================
#Author: Kyle
#Date:  2008/10/22
#Code:    /AP/goahead/LINUX/*.c  web/wlcontrol.asp
#Describe:
#			Wireless access control支援Multiple ssid function
#			最多可以支援四組SSID每組SSID可以有各自的Access Control LIST.
#			
#						
MULTIPLE_WLAN_ACCESS_CONTROL="MULTIPLE_WLAN_ACCESS_CONTROL"
F_MULTIPLE_WLAN_ACCESS_CONTROL="2586"
#================Function: 78====================
#Author: Bryan
#Date:  2008/07/18
#Code:
#Describe:
#       PC Database 
#                                               
PC_DATABASE="PC_DATABASE"
F_PC_DATABASE="628"
#================Function: 79====================
#Author: Vance
#Date:  2008/11/10
#Code:   
#Describe:
#			轉移公版RT288x_SDK的linux-igd和wsc_upnp
#			使用libupnp-1.3.1						
UPNP_LIB_VERSION2="UPNP_LIB_VERSION2"
#================Function: 80====================
#Author: Wise
#Date:  2008/11/11
#Code:
#Describe:
#       Wireless Driver Version 
#                                               
WIRELESS_DRIVER_VERSION="WIRELESS_DRIVER_VERSION"
#================Function: 80====================
#Author: Vance
#Date:  2008/11/14
#Describe:
#       可設定時間排程將wireless功能打開或關閉
#                                               
WIRELESS_SCHEDULE="WIRELESS_SCHEDULE"
F_WIRELESS_SCHEDULE="922"
#Author: virance
#Date: 	2009/01/08
#Code:   scriptlib_util.sh  scriptlib.sh
#Describe: 
#	     增加schedule_PortForwarding及schedule_UrlBlocking兩個script function
#
Schedule_PortForwarding="Schedule_PortForwarding"
Schedule_UrlBlocking="Schedule_UrlBlocking"

#================Function: 80====================
#Author: Vance
#Date:  2008/11/18
#Code:
#Describe:
#      test iptables-1.4.2
#                                               
IPTABLES_V142="IPTABLES_V142"
#================Function: 81====================
#Author: Kyle
#Date:  2008/11/14
#Code:
#Describe:
#       Ralnk SDK Version,Kernel目錄會隨著不同的SDK版本進行切換
#Input $1:  SDK版號
#               3100
#               3200
#Example:
#       addDefined_to_script "$RALINK_SDK_VERSION" "3200"
#                                               
RALINK_SDK_VERSION="RALINK_SDK_VERSION"
#================Function: 82====================
#Author: Bryan
#Date: 	2008/12/01
#Code:
#Describe: 
#	     Support 3G WAN
#
WAN3G="WAN3G"
F_WAN3G="257"
#================Function: 82====================
#Author: Bryan
#Date: 	2009/03/06
#Code:
#Describe: 
#	     Support Fail Over Mail Alert
#
FOMAIL="FOMAIL"
F_FOMAIL="97"
#================Function: 83====================
#Author: Vance
#Date: 	2008/12/09
#Describe: 
#	     web-based login web 
#
LOGIN_WEB="LOGIN_WEB"
#================Function: 84====================
#Author: Kyle
#Date: 	2008/12/08
#Code:   kernel/driver/net/pppoe.c
#Describe: 
#	     解決PPPOE不當斷線時，server上連線斷不乾淨問題，當Server發出Echo封包時
#        判斷session號碼，若不存在則自動發送PADT封包斷掉前一次的連線
#
PPPOE_AUTO_PADT="PPPOE_AUTO_PADT"
#================Function: 85====================
#Author: Wise
#Date: 	2008/12/16
#Code: 
#Describe:
#	無線 station driver
WL_STA_DRIVER="WL_STA_DRIVER"
F_WL_STA_DRIVER="217"
#================Function: 86====================
#Author: Vance
#Date: 	2008/12/11
#Code:   goahead/security.c 
#			cgic205/index.cgi
#			script/init.sh
#			STAR_app_collecting_script 
#Describe: 
#	     web-based login web in SSID1 for star 
#		  輸入正確的帳號密碼才能連上internet
STAR_LOGIN_WEB="STAR_LOGIN_WEB"
#================Function: 87====================
#Author: Kyle
#Date: 	2008/12/08
#Code:   /script/init.sh
#Describe: 
#	     AP Router  切換軟體控制版
#
SOFT_ARSWITCH="SOFT_ARSWITCH"
#================Function: 88====================
#Author: virance
#Date: 	2008/12/29
#Code:    apform.h   main.c  fmget.c  fmmgmt.c  tlping.asp  tlpingresult.asp  title_left.asp  title_middle.asp  title_right.asp  tlmenu.asp  index.asp  allasp-n.var  mutilanguage.var  scriptlib.sh  scriptlib_util.sh
#Describe: 
#	在web UI中新增ping tool功能
#	apform.h： extern int pingstatus為全域變數, formSystempingtool 及 formSystempingEnd兩個function
#	main.c： 定義formSystempingtool及formSystempingEnd function為form表單
#	fmget.c： 1. 設置getIndex("pingstatus")裡，當pingstatus = 0時表示還沒開始執行ping command並且不執行每秒更新tlpingresult.asp
#				    當pingstatus = 1時為開始執行ping command並且開始每秒refresh tlpingresult.asp
#				 2. 設置getInfo("pingresult")讀取/tmp/pingresultdata.txt的結果
#	fmmgmt.c： 1. formSystemtool function 設置pingstatus的值為1，
#					   呼叫scriptlib_util.sh在背景執行ping command將結果導入/tmp/pingresultdata.txt，
#					   並且在ping結束後再結尾echo "---   END   ---"   
#					2. formSystemEnd function 設置pingstatus的值為0, 下達sed指令取代END字串為Finish
#	tlping.asp： 設置Systempingtool表單裡加入hidden欄位傳送pingstatus狀態，其值為1
#				   若getIndex("pingstatus")的值為0，Start button為Enable，若值為1則Start button 為disable
#	tlpingresult.asp：當pingstatus為1時，每秒更新讀取/tmp/pingresultdata.txt，如果取得---   END   ---字串則停止每秒refresh，傳送pingstatus的值為0，並且reload tlping.asp頁面
#	scriptlib.sh：新增pingtool() function，執行ping command
#	scriptlib_util.sh：呼叫scriptlib.sh執行pingtoo()，帶入三個參數分別為IP, 次數，檔案名稱
#
Systempingtool="Systempingtool"
#================Function: 89====================
#Author: Kyle
#Date: 	2008/12/24
#Code:   /script/cleanlog.sh
#Describe: 
#	     自動偵測WAN Port link down時將WAN ip  release.並在link up時重新發出DHCP Request 重要IP.
#
WAN_DHCP_AUTO_RELEASED="WAN_DHCP_AUTO_RELEASED"
#================Function: 90====================
#Author: Vance
#Date: 	2008/12/30
#Describe: 
#	     3G for STAR 
#
STAR_CF_3G="STAR_CF_3G"
#================Function: 91====================
#Author: Vance
#Date: 	2009/1/05
#Describe: 
#	     WAN interface 
#
WAN_IF="WAN_IF"
#================Function: 92====================
#Author: Vance
#Date: 	2009/1/05
#Describe: 
#	     LAN interface 
#
LAN_IF="LAN_IF"
#================Function: 93====================
#Author: Vance
#Date: 	2009/1/05
#Describe: 
#	     default value of WAN interface 
#
DEFAULT_WAN_IF="DEFAULT_WAN_IF"
#================Function: 94====================
#Author: Vance
#Date: 	2009/1/05
#Describe: 
#	     default value of LAN interface 
#
DEFAULT_LAN_IF="DEFAULT_LAN_IF"
#================Function: 95====================
#Author: Wise
#Date:  2008/12/26
#Code:    udhcpd/udhcpc.c script/dhcpc.sh.ap
#Describe:
#			取得dhcp ip跟lease time但是不馬上設定
#						
GET_A_DHCP_IP="GET_A_DHCP_IP"
#================Function: 96====================
#Author: virance
#Date:  2009/01/18
#Code:
#Describe: 
#           在status 增加connectionifo功能 
#	    在nat    增加connectionCtrl及timeout
CONNECTION_INFO="CONNECTION_INFO"
#================Function: 97====================
#Author: virance
#Date:  2009/01/18
#Code: fmget.c：設置getIndex('connectionCtrlEnabled') 開關control功能，1為ON, 0為OFF
#					 設置getInfo('connection_ctrlSet') 將所有的設定值列出，再用javascript令值
#		   fmtcpip.c：新增formConnectionCtrlSet function 存取所有欄值的值，"MaxConnectionCount" 的最大值不得超過3076
#		   mibtbl.c ： 新增加MIB_CONNECTIONCTRL_ENABLED , MIB_CONNECCOUNT_MAX,MIB_UDPCOUNT_MAX,MIB_ICMPCOUNT_MAX,
#                     	     MIB_TIMEOUT_ESTABLISHED,MIB_TIMEOUT_SYNSENT,MIB_TIMEOUT_SYNRECV,MIB_TIMEOUT_FINWAIT,MIB_TIMEOUT_CLOSEWAIT,
#					     MIB_TIMEOUT_LASTACK,MIB_TIMEOUT_GENERIC
#		   apmib.h：  (conneccountmax udpcountmax icmpcountmax timeoutestablished timeoutsynsent timeoutsynrecv timeoutfinwait timeoutclosewait timeoutlastcck timeoutgeneric )宣告為short，存取欄位值
#						connectionctrlenabled 宣告為char，存取connection control的開關控制的值
#		   apform.h：extern formConnectionCtrlSet function 
#   	   main.c： 定義formConnectionCtrlSet function
#		   genmenu.asp：當_CONNECTION_CTRL_被定義時，natconnectionctrl.asp的display才能被block 
#		   natconnectionctrl.asp 設定connection control 及connection timeout的web UI介面
#Describe: 
CONNECTION_CTRL="CONNECTION_CTRL"
F_CONNECTION_CTRL="23"
#================Function: 98====================
#Author: Morris,Rex,Virance
#Date:  2009/01/19
#Code:  
# linux-2.6.21.x_3200/net/ipv4/netfilter/ipt_time.c
# linux-2.6.21.x_3200/net/ipv4/netfilter/Kconfig
# linux-2.6.21.x_3200/net/ipv4/sysctl_net_ipv4.c
# linux-2.6.21.x_3200/net/ipv4/arp.c
# linux-2.6.21.x_3200/net/ipv4/ip_input.c
# linux-2.6.21.x_3200/include/linux/sysctl.h
# busybox-1.11.1/networking/udhcp/serverpacket.c
#
#Describe: 
#           在firewall 增加super DMZ功能 
#
SDMZ="SDMZ"
F_SDMZ="9"
#================Function: 99====================
#Author: Wise
#Date:  2009/01/21
#Code: wireless driver -> ap_ioctl.c & oid.h
#
#Describe: 
#           顯示wds apcli client三種wireless連線
#
WDS_UR_INFO="WDS_UR_INFO"
#================Function: 100====================
#Author: Vance
#Date: 	2009/01/08
#Code: init.sh
#		 STAR_app_collecting_script.sh,init.sh
#											
#Describe: 
#	    change 3G connetion script for JP 
#
STAR_3G_JP="STAR_3G_JP"

#================Function: 101====================
#Author: Wise
#Date:  2009/02/02
#Code: flash.c alg.sh
#
#Describe: 
#           ALG增加sip，只define在compiler，alg.sh寫死
#			另外還要改WEB的natalg.asp以及增加字串
ALGSIP="ALGSIP"

#================Function: 102====================
#Author: ThomasJi
#Date:  2009/08/03
#Describe:
#        Support L2TP Passthrough .
L2TP_PASSSTHROUGH="L2TP_PASSSTHROUGH"

#************************End Function Defined*****************************


#************************System Variable**********************************

C_DEF_FILE="${ROOTDIR}/mode.def"

FUNCTION_COMPILER_FILE="${ROOTDIR}/define/FUNCTION_COMPILER"
FUNCTION_SCRIPT_FILE="${ROOTDIR}/define/FUNCTION_SCRIPT"

DEFINED_LIST=""

C_DEFINED=""

SCRIPT_DEFINED=""

FLASH_RESERVE_SIZE=0

FUNCTION_FLASH_SIZE=""

#rm -f $SCRIPT_DEF_FILE > /dev/null

#*************************Function Call***********************************
usage()
{
	echo "Usage:"
	echo "   "
	exit 1
}

#
#Add defined to compiler and script.
#
function addDefined_to_All
{
	addDefined_to_script $1 $2
	addDefined_to_compiler $1 $2
	
}

#################################################################################################################
#Author:    Kyle
#
#Date:      2008/01/25
#
#Describe:  Add defined to script(javaScript and shellScript) only.
#
#Usage:     $1 FunctionName $2 Variable
#if Variable is Null, The variable default = y
#JavaScript: Use "<% getInfo("getDefined");%>" function to get script defined in html file.
#ShellScript:Include ". /web/FUNCTION_SCRIPT" in shell script head.
#
#Example: 
#Input:  addDefined_to_script "MSSIDNUM" "3"       
#OutPut: _MSSIDNUM_=3
#
function addDefined_to_script
{
    if [ "$1" != "" ]; then
        if [ "$2" != "" ]; then
        	SCRIPT_DEFINED="${SCRIPT_DEFINED} _${1}_=\"${2}\""
        else
       	 SCRIPT_DEFINED="${SCRIPT_DEFINED} _${1}_=\"y\""
        fi
    fi  

}

#################################################################################################################
#Author:    Kyle
#
#Date:      2008/01/25
#
#Describe:  Add defined to compiler only
#
#Usage:     $1 FunctionName 
#
#Example: 
#Input:   addDefined_to_compiler MSSID  
#OutPut:  FUNCTION=-D_MSSID_
function addDefined_to_compiler
{
	if [ "$1" != "" ]; then
		if [ "$2" != "" ]; then
            C_DEFINE="$C_DEFINE -D_$1_=$2"
		else
			C_DEFINE="$C_DEFINE -D_$1_"
		
		fi
	fi	
    eval "FUNCTION_FLASH_SIZE="\$F_$1""

    if [ "$FUNCTION_FLASH_SIZE" != "" ]; then
        echo	 "ADD FLASH RESERVED SIZE: F_$1 = $FUNCTION_FLASH_SIZE"
        FLASH_RESERVE_SIZE=`expr $FLASH_RESERVE_SIZE + $FUNCTION_FLASH_SIZE`
    fi
}


#################################################################################################################
#Author:    Kyle
#
#Date:      2008/08/27
#
#Describe:  套用字串型態的Defined
#
#Usage:     $1 String內容 
#
function addDefined_String_to_compiler
{
	
	addDefined_to_compiler $1 "\"((char *)\\\"${2}\\\")\""

}

#################################################################################################################
#Author:    Kyle
#
#Date:      2008/08/27
#
#Describe: Add reserved flash size.
function add_Flash_Reserved
{

    if [ "$1" != "" ]; then
        echo	 "ADD FLASH RESERVED SIZE: $1"
        FLASH_RESERVE_SIZE=`expr $FLASH_RESERVE_SIZE + $1`
    fi

}

#################################################################################################################
#Author:    Kyle
#
#Date:      2008/01/25
#
#Describe: Start to handle defined data.
function setDefined
{
#Clean DEF_LIST
	b=""
	DEFINED_LIST=""
	for ARG in ${C_DEFINE} ; do
	DEFINED_LIST=${DEFINED_LIST}${b}${ARG}
	if [ "$b" = "" ]; then
		b=" "
	fi
	done	
	
	> ${FUNCTION_COMPILER_FILE}
	echo -n       "FUNCTION=" >> ${FUNCTION_COMPILER_FILE}
	echo -n "-D_FLASH_RESERVED_=${FLASH_RESERVE_SIZE} " >> ${FUNCTION_COMPILER_FILE}
	echo    "${DEFINED_LIST}" >> ${FUNCTION_COMPILER_FILE}

  > ${FUNCTION_SCRIPT_FILE}
		echo       "_${DATE}_=\"`date`\"" >> ${FUNCTION_SCRIPT_FILE}
	for ARG in $SCRIPT_DEFINED ; do
		echo               "$ARG" >> ${FUNCTION_SCRIPT_FILE}
	done
}

#*********************End Function Call************************************#


#*********************Defined List*****************************************#
echo "**********************************************************************"
echo "*                       START FUNCTION DEFINE                        *"
echo "**********************************************************************"


###################################################
#           Defined Platform Information          #
###################################################
addDefined_to_compiler "${1}"
addDefined_to_compiler "${2}"
addDefined_to_compiler "${4}"
addDefined_String_to_compiler "$MODEL"   "${1}"
addDefined_String_to_compiler "$MODE"    "${2}"
addDefined_String_to_compiler "$PLATFORM"    "${4}"
addDefined_to_All    "$VERSION" "${3}"
addDefined_to_script "$MODEL"   "${1}"
addDefined_to_script "$MODE"    "${2}"
addDefined_to_script "$PLATFORM"    "${4}"
addDefined_to_script "$RALINK_SDK_VERSION" "3200"

###################################################
#           Defined GPIO                          #
###################################################
addDefined_to_All "$HW_LED_WPS" "11"
addDefined_to_All "$HW_LED_POWER" "9"
addDefined_to_All "$HW_LED_WIRELESS" "14"
addDefined_to_All "$HW_LED_USB" "7"
addDefined_to_All "$HW_BUTTON_RESET" "12"
addDefined_to_All "$HW_BUTTON_WPS" "0"
addDefined_to_All "$HW_BUTTON_SWITCH" "13"

###################################################
#          All Platform Function.                 #
###################################################
addDefined_to_script "$WIRELESS_IGMPSNOOP"
addDefined_to_All "$SPECIAL_CHAR_FILTER_IN_SCRIPT"
addDefined_to_compiler "$UPGRADE_PROCESS"
addDefined_to_compiler "$KREBOOT"
addDefined_to_script "$RDISC"
addDefined_to_compiler "$PATD"
addDefined_to_compiler "$NETBIOSNAME"
addDefined_to_compiler "$HIDING_CGI_MP_PAGE"
addDefined_to_All "$WPS_NO_BROADCAST"
addDefined_to_All "$UPNP_LIB_VERSION2"
addDefined_to_compiler "$PPPOE_AUTO_PADT"
addDefined_to_All "$WDS_UR_INFO"

addDefined_to_script "$RFTYPE" "2T2R" # RT3050-> 1T1R, RT3052-> 2T2R
addDefined_String_to_compiler "$RFTYPE" "2T2R" # RT3050-> 1T1R, RT3052-> 2T2R
addDefined_to_All "$IS_GATEWAY" 
addDefined_String_to_compiler "$WEB_HEADER" "3G64"
addDefined_to_All "$MEMBUS"  "32" # # RT3050-> 16bit, RT3052-> 32bit
addDefined_to_All "$MEMSIZE" "16" # memory size
addDefined_to_All "$WAN3G"
addDefined_to_All "$FOMAIL"
addDefined_to_All "$WIRELESS_SWITCH"
addDefined_to_script "$WIRELESS_SWITCH_REVERSE"
addDefined_String_to_compiler "$PRODUCT_NAME" "Wireless 3G Router"
addDefined_to_All "$WIRELESS_DRIVER_VERSION" "19"
addDefined_String_to_compiler "$WEB_LOGIN_ALERT_STRING" "Default: admin/admin"
addDefined_to_All "$PPTP_PASSSTHROUGH"
addDefined_to_All "$L2TP_PASSSTHROUGH"

setDefined ${1} ${2} ${3}

###################################################
#    Print Debug Message.                   #
###################################################
echo ""
echo "MODEL   =${1}"
echo "MODE    =${2}"
echo "DATE    =`date`"
echo "VERSION =${3}"
echo ""
echo "FUNCTION=$DEFINED_LIST"
echo ""
echo "SCRIPT  =$SCRIPT_DEFINED"
echo ""
echo "TOTAL FLASH RESERVED SIZE=$FLASH_RESERVE_SIZE"
echo ""
${ROOTDIR}/set_compiler_condition.sh
