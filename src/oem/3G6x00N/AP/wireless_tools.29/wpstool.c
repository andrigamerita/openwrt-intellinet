#include "iwlib.h"		/* Header */

// Wsc status code
#define	STATUS_WSC_NOTUSED						0
#define	STATUS_WSC_IDLE							1
#define STATUS_WSC_FAIL        			        2		// WSC Process Fail
#define	STATUS_WSC_LINK_UP						3		// Start WSC Process
#define	STATUS_WSC_EAPOL_START_RECEIVED			4		// Received EAPOL-Start
#define	STATUS_WSC_EAP_REQ_ID_SENT				5		// Sending EAP-Req(ID)
#define	STATUS_WSC_EAP_RSP_ID_RECEIVED			6		// Receive EAP-Rsp(ID)
#define	STATUS_WSC_EAP_RSP_WRONG_SMI			7		// Receive EAP-Req with wrong WSC SMI Vendor Id
#define	STATUS_WSC_EAP_RSP_WRONG_VENDOR_TYPE	8		// Receive EAPReq with wrong WSC Vendor Type
#define	STATUS_WSC_EAP_REQ_WSC_START			9		// Sending EAP-Req(WSC_START)
#define	STATUS_WSC_EAP_M1_SENT					10		// Send M1
#define	STATUS_WSC_EAP_M1_RECEIVED				11		// Received M1
#define	STATUS_WSC_EAP_M2_SENT					12		// Send M2
#define	STATUS_WSC_EAP_M2_RECEIVED				13		// Received M2
#define	STATUS_WSC_EAP_M2D_RECEIVED				14		// Received M2D
#define	STATUS_WSC_EAP_M3_SENT					15		// Send M3
#define	STATUS_WSC_EAP_M3_RECEIVED				16		// Received M3
#define	STATUS_WSC_EAP_M4_SENT					17		// Send M4
#define	STATUS_WSC_EAP_M4_RECEIVED				18		// Received M4
#define	STATUS_WSC_EAP_M5_SENT					19		// Send M5
#define	STATUS_WSC_EAP_M5_RECEIVED				20		// Received M5
#define	STATUS_WSC_EAP_M6_SENT					21		// Send M6
#define	STATUS_WSC_EAP_M6_RECEIVED				22		// Received M6
#define	STATUS_WSC_EAP_M7_SENT					23		// Send M7
#define	STATUS_WSC_EAP_M7_RECEIVED				24		// Received M7
#define	STATUS_WSC_EAP_M8_SENT					25		// Send M8
#define	STATUS_WSC_EAP_M8_RECEIVED				26		// Received M8
#define	STATUS_WSC_EAP_RAP_RSP_ACK				27		// Processing EAP Response (ACK)
#define	STATUS_WSC_EAP_RAP_REQ_DONE_SENT		28		// Processing EAP Request (Done)
#define	STATUS_WSC_EAP_RAP_RSP_DONE_SENT		29		// Processing EAP Response (Done)
#define STATUS_WSC_EAP_FAIL_SENT                30      // Sending EAP-Fail
#define STATUS_WSC_ERROR_HASH_FAIL              31      // WSC_ERROR_HASH_FAIL
#define STATUS_WSC_ERROR_HMAC_FAIL              32      // WSC_ERROR_HMAC_FAIL
#define STATUS_WSC_ERROR_DEV_PWD_AUTH_FAIL      33      // WSC_ERROR_DEV_PWD_AUTH_FAIL
#define STATUS_WSC_CONFIGURED					34
#define	STATUS_WSC_SCAN_AP						35		// Scanning AP
#define	STATUS_WSC_EAPOL_START_SENT				36
#define STATUS_WSC_EAP_RSP_DONE_SENT			37
#define STATUS_WSC_WAIT_PIN_CODE                38
#define STATUS_WSC_START_ASSOC					39
#define STATUS_WSC_DO_MULTI_PBC_DETECTION		40


// Authentication types
#define WSC_AUTHTYPE_OPEN        0x0001
#define WSC_AUTHTYPE_WPAPSK      0x0002
#define WSC_AUTHTYPE_SHARED      0x0004
#define WSC_AUTHTYPE_WPA         0x0008
#define WSC_AUTHTYPE_WPA2        0x0010
#define WSC_AUTHTYPE_WPA2PSK     0x0020
int WSC_AUTHTYPE_WPA1PSKWPA2PSK=WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK;
// Encryption type
#define WSC_ENCRTYPE_NONE    0x0001
#define WSC_ENCRTYPE_WEP     0x0002
#define WSC_ENCRTYPE_TKIP    0x0004
#define WSC_ENCRTYPE_AES     0x0008
int WSC_ENCRTYPE_TKIPAES=WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES;

///////////////////////////////////////////
#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE                              0x8BE0
#endif
#define SIOCIWFIRSTPRIV								SIOCDEVPRIVATE
#endif

#define RT_PRIV_IOCTL								(SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET							(SIOCIWFIRSTPRIV + 0x02)
				

#ifdef DBG
#define RTPRIV_IOCTL_BBP                            (SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC                            (SIOCIWFIRSTPRIV + 0x05)
#define RTPRIV_IOCTL_E2P                            (SIOCIWFIRSTPRIV + 0x07)
#endif

#define RTPRIV_IOCTL_STATISTICS                     (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE                (SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA                    (SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY					(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY                    (SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE					(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_STATIC_WEP_COPY                (SIOCIWFIRSTPRIV + 0x10)

#define RTPRIV_IOCTL_SHOW							(SIOCIWFIRSTPRIV + 0x11)
#define RTPRIV_IOCTL_WSC_PROFILE                    (SIOCIWFIRSTPRIV + 0x12)
#define RTPRIV_IOCTL_QUERY_BATABLE                  (SIOCIWFIRSTPRIV + 0x16)

#define OID_GET_SET_TOGGLE							0x8000

#define RT_QUERY_ATE_TXDONE_COUNT					0x0401
#define RT_QUERY_SIGNAL_CONTEXT						0x0402
#define RT_SET_IAPP_PID                 			0x0404
#define RT_SET_APD_PID								0x0405
#define RT_SET_DEL_MAC_ENTRY						0x0406


// Ioctl value for WPS UPnP daemon and LLTD daemon must be identical in RT61 and RT2860
#define RT_OID_SYNC_RT61                            0x0D010750


// for consistency with RT61
#define RT_OID_WSC_QUERY_STATUS						((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_WSC_PIN_CODE							((RT_OID_SYNC_RT61 + 0x02) & 0xffff)
#define RT_OID_WSC_UUID								((RT_OID_SYNC_RT61 + 0x03) & 0xffff)
#define RT_OID_WSC_SET_SELECTED_REGISTRAR			((RT_OID_SYNC_RT61 + 0x04) & 0xffff)
#define RT_OID_WSC_EAPMSG							((RT_OID_SYNC_RT61 + 0x05) & 0xffff)
#define RT_OID_WSC_MANUFACTURER						((RT_OID_SYNC_RT61 + 0x06) & 0xffff)
#define RT_OID_WSC_MODEL_NAME						((RT_OID_SYNC_RT61 + 0x07) & 0xffff)
#define RT_OID_WSC_MODEL_NO							((RT_OID_SYNC_RT61 + 0x08) & 0xffff)
#define RT_OID_WSC_SERIAL_NO						((RT_OID_SYNC_RT61 + 0x09) & 0xffff)
#define RT_OID_WSC_MAC_ADDRESS						((RT_OID_SYNC_RT61 + 0x10) & 0xffff)

/////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((packed)) _WSC_CONFIGURED_VALUE {
	unsigned short WscConfigured; // 1 un-configured; 2 configured
	unsigned char	WscSsid[32 + 1];
	unsigned short WscAuthMode;	// mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x20: wpa2-psk
	unsigned short	WscEncrypType;	// 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
	unsigned char	DefaultKeyIdx;
	unsigned char	WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;


/*------------------------------------------------------------------*/
/*
 * Print usage string
 */
static void
wps_usage(void)
{
  fprintf(stderr, "Usage: \nwpstool Pincode\nwpstool Status\nwpstool Profile\nwpstool Profile Configured\nwpstool Profile Ssid\nwpstool Profile AuthMode\nwpstool Profile EncrypType\nwpstool Profile WpaKey\nwpstool CheckStatus confiuredType[0/1]\n");
}
static void
profile_usage(void)
{
  fprintf(stderr, "Profile Command Usage: Configured \n Ssid \n AuthMode \n EncrypType \n WpaKey\n CheckStatus\n");
}
static char *StrReplace(char *Str, char *OldStr, char *NewStr)
{
      int OldLen, NewLen;
      char *p, *q;
		p=Str;
		q=Str;

	  while(1){
      if(NULL == (p = strstr(p, OldStr))){
      	break;
      }
      OldLen = strlen(OldStr);
      NewLen = strlen(NewStr);
      memmove(q = p+NewLen, p+OldLen, strlen(p+OldLen)+1);
      memcpy(p, NewStr, NewLen);
      p+=NewLen;
	  }
      return q;
}
char *CharFilter(char *Str){
		char tmpBuf2[1024]={0};
		strncpy(tmpBuf2,Str+1,strlen(Str)-2);
		StrReplace(tmpBuf2,"\\","\\\\");
		StrReplace(tmpBuf2,"\"","\\\"");
		StrReplace(tmpBuf2,"$","\\$");
		StrReplace(tmpBuf2,"`","\\`");
		sprintf(Str,"\"%s\"",tmpBuf2);
		return Str;
}

static int
set_private_cmd(int		skfd,		/* Socket */
		char *		args[],		/* Command line args */
		int		count,		/* Args count */
		char *		ifname,		/* Dev name */
		char *		cmdname,	/* Command name */
		iwprivargs *	priv,		/* Private ioctl description */
		int		priv_num)	/* Number of descriptions */
{
	int ret;
  struct iwreq	wrq;
  u_char	buffer[4096];	/* Only that big in v25 and later */
  unsigned int  unIntData=0;
  int checkTime=125;
  signed int beforeStatus=-1; 
  signed int    intData=0;
 	int encryptType=0,authMode=0;
  char flashCommand[200];
  char pinCode[10];
  char tmpBuf[200];
  WSC_CONFIGURED_VALUE Profile;
	strcpy(wrq.ifr_name,ifname);
	if(!strcmp(cmdname, "Pincode")){
 		sprintf(pinCode,"");
		int mod=100000000;
		int i=1;
		
		wrq.u.data.length=sizeof(unsigned int);
		wrq.u.data.pointer=&unIntData;
		wrq.u.data.flags=RT_OID_WSC_PIN_CODE;
		
		// Do ioctl
		ret = ioctl(skfd,RT_PRIV_IOCTL,&wrq);
		
		if(ret != 0) {
			printf("Get pincode error!\n");
			return -1;
		}

		printf("%08u\n",unIntData);
	}
	else if(!strcmp(cmdname, "Status")){
		
		wrq.u.data.length=sizeof(signed int);
		wrq.u.data.pointer=&intData;
		wrq.u.data.flags=RT_OID_WSC_QUERY_STATUS;
	// Do ioctl
		ret = ioctl(skfd,RT_PRIV_IOCTL,&wrq);
		
		if(ret != 0) {
			printf("Get wps status error!\n");
			return -1;
		}
		printf("Status=%u\n",intData);
	}
	else if(!strcmp(cmdname, "Profile")){
		memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
		wrq.u.data.length = sizeof(Profile);
		wrq.u.data.pointer=&Profile;
		wrq.u.data.flags=0;
	// Do ioctl
		ret = ioctl(skfd,RTPRIV_IOCTL_WSC_PROFILE,&wrq);
		if(ret != 0) {
			printf("Get profile error!\n");
			return -1;
		}		
		if(count<=0)
			printf("Configured=%d:Ssid=%s:AuthMode=%d:EncrypType=%d:WpaKey=%s\n",Profile.WscConfigured,Profile.WscSsid,Profile.WscAuthMode,Profile.WscEncrypType,Profile.WscWPAKey);
		else{
			if(!strcmp(args[0], "Configured")){
					printf("%d\n",Profile.WscConfigured);
			}else if(!strcmp(args[0], "Ssid")){
					printf("%s\n",Profile.WscSsid);
			}else if(!strcmp(args[0], "AuthMode")){
					printf("%d\n",Profile.WscAuthMode);
			}else if(!strcmp(args[0], "EncrypType")){
					printf("%d\n",Profile.WscEncrypType);
			}else if(!strcmp(args[0], "WpaKey")){
					printf("%s\n",Profile.WscWPAKey);
			}else{
				printf("Invalid Command:: %s\n",args[0]);
				profile_usage();
			}	
		}
	} 
	else if(!strcmp(cmdname,"CheckStatus")){
		system("flash set WPS_CONFIG_STATUS 0");
		while(checkTime>0){
			intData=0;
			wrq.u.data.length=sizeof(signed int);
			wrq.u.data.pointer=&intData;
			wrq.u.data.flags=RT_OID_WSC_QUERY_STATUS;
			// Do ioctl
			ret = ioctl(skfd,RT_PRIV_IOCTL,&wrq);
			if(ret != 0) {
				printf("Get wps status error!\n");
				return -1;
			}
			if(intData!=beforeStatus){
				printf("Wps status change::%d->%d\n",beforeStatus,intData);
				sprintf(flashCommand,"flash set WPS_CONFIG_STATUS %d",intData);
				//printf("debug: command=%s\n",flashCommand);
				system(flashCommand);
				
				//system("flash get WPS_CONFIG_STATUS | cut -f 2 -d = >/var/wpsStatus.var");

			}
			beforeStatus=intData;
			if(intData==STATUS_WSC_NOTUSED || intData==STATUS_WSC_FAIL || intData==STATUS_WSC_EAP_FAIL_SENT || intData==STATUS_WSC_ERROR_HASH_FAIL ||
			intData==STATUS_WSC_ERROR_HMAC_FAIL || intData==STATUS_WSC_ERROR_DEV_PWD_AUTH_FAIL || intData==STATUS_WSC_CONFIGURED ){
				if(intData == STATUS_WSC_CONFIGURED)
					printf("WPS Finish!\n",intData);
				else
					printf("WPS Config Status Error : %d !\n",intData);
				checkTime=0;
				break;
			}
			//sprintf(flashCommand,"echo %d >/var/wpsTime.var ",checkTime);
			//system(flashCommand);
			checkTime--;	
			
			sleep(1);
		}
		if(intData==34){
			printf("Device Configured!!\n");
			
			if(count>0 &&  !strcmp(args[0], "0")){//unConfigured
				//unsigned short	WscEncrypType;	// 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
				
						//USHORT WscAuthMode;	// mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x20: wpa2-psk
						//USHORT	WscEncrypType;	// 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
				
				memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
				wrq.u.data.length = sizeof(Profile);
				wrq.u.data.pointer=&Profile;
				wrq.u.data.flags=0;
				// Do ioctl
				ret = ioctl(skfd,RTPRIV_IOCTL_WSC_PROFILE,&wrq);
				if(ret != 0) {
					printf("Get profile error!\n");
					return -1;
				}		
				
				encryptType=(Profile.WscEncrypType);
				authMode=Profile.WscAuthMode;
				printf(" encryptType=%d authMode=%d\n\n",encryptType,authMode);
				//printf("\n***********debug2****\n wlanEncrypt=%d\n",encryptType);
				//printf("debug command=");

				sprintf(flashCommand,"flash set WLAN_ENCRYPT %d",encryptType/2);

				//printf("debug command=%s\n",flashCommand);
				system(flashCommand);
				
				
				
				sprintf(tmpBuf," %s ", Profile.WscSsid);
				CharFilter(tmpBuf);
				sprintf(flashCommand,"flash set SSID %s",tmpBuf);
				system(flashCommand);
		
				if(encryptType == WSC_ENCRTYPE_NONE || encryptType == WSC_ENCRTYPE_WEP || encryptType == WSC_ENCRTYPE_TKIP){//0:open 1:wep 2:wpa
					sprintf(flashCommand,"flash set SECURITY_MODE %d",encryptType/2);
					//printf("debug command=%s\n",flashCommand);
					system(flashCommand);
				}else if(encryptType == WSC_ENCRTYPE_AES || encryptType== WSC_ENCRTYPE_TKIPAES){
					sprintf(flashCommand,"flash set SECURITY_MODE %d",2);
					//printf("debug command=%s\n",flashCommand);
					system(flashCommand);
				}
				
				if(encryptType==WSC_ENCRTYPE_WEP){//wep mode
					system("flash set WEP 1");//select key 1
					sprintf(flashCommand,"flash set WEP64_KEY1 %s",Profile.WscWPAKey);
					//printf("debug command=%s\n",flashCommand);
					system(flashCommand);
					
				}
				else if(encryptType == WSC_ENCRTYPE_TKIP || encryptType == WSC_ENCRTYPE_AES || encryptType== WSC_ENCRTYPE_TKIPAES){
					
					if(encryptType==WSC_ENCRTYPE_TKIP && authMode==WSC_AUTHTYPE_WPAPSK){//wpa-psk tkip
						printf("WPA-PSK TKIP Mode!\n");
						system("flash set WLAN_WPA_CIPHER_SUITE 1");
						system("flash set WPA2_CIPHER_SUITE 0");
					}else if(encryptType==WSC_ENCRTYPE_AES && authMode ==WSC_AUTHTYPE_WPA2PSK){//Wpa2-psk AES
						printf("WPA2-PSK AES Mode!\n");
						system("flash set WLAN_WPA_CIPHER_SUITE 0");
						system("flash set WPA2_CIPHER_SUITE 2");
					}else if(encryptType==WSC_ENCRTYPE_TKIPAES && authMode == WSC_AUTHTYPE_WPA1PSKWPA2PSK){//Wpa2-psk AES)
							// mixMode
						printf("MIX Mode!\n");
						printf("Mix Mode encryptType=%d authMode=%d\n",encryptType,authMode);
						system("flash set WLAN_WPA_CIPHER_SUITE 1");
						system("flash set WPA2_CIPHER_SUITE 2");
					}
					else{
							// mixMode
						printf("MIX Mode!\n");
						printf("Mix Mode encryptType=%d authMode=%d\n",encryptType,authMode);
						system("flash set WLAN_WPA_CIPHER_SUITE 1");
						system("flash set WPA2_CIPHER_SUITE 2");
					}
		#if defined(_WPS_SHORT_KEY_) || defined(_WEP_MAC_) || defined(_AUTOWPA_) || defined(_WPA_KEY_BY_MAC_)
						system("flash set WLAN_PSK_FORMAT 0");//passphrase
		#else
						system("flash set WLAN_PSK_FORMAT 1");//hex format
		#endif
						

						sprintf(flashCommand,"flash set WLAN_WPA_PSK %s",Profile.WscWPAKey);//set key
								//printf("debug command=%s\n",flashCommand);
						system(flashCommand);
				}
					
				//printf("\n***********end debug*****\n");
				system("flash set WPS_DISPLAY_KEY 1");
			}
				system("flash set WPS_CONFIG_TYPE 1");

			system("/bin/scriptlib_util.sh reloadFlash");
		}else if(intData==1){
			printf("Time Out!!\n");	
		} 
		else{
			printf("Device Configure Fail :: %d!!\n",intData);
		}
	}
	else
		wps_usage();
}

static inline int
set_private(int		skfd,		/* Socket */
	    char *	args[],		/* Command line args */
	    int		count,		/* Args count */
	    char *	ifname)		/* Dev name */
{
  iwprivargs *	priv;
  int		number;		/* Max of private ioctl */
  int		ret;

  /* Read the private ioctls */
  number = iw_get_priv_info(skfd, ifname, &priv);

  /* Is there any ? */
  if(number <= 0)
    {
      /* Should I skip this message ? */
      fprintf(stderr, "%-8.16s  no private ioctls.\n\n",
	      ifname);
      if(priv)
	free(priv);
      return(-1);
    }

  /* Do it */
  ret = set_private_cmd(skfd, args+1, count-1 , ifname, args[0],
			priv, number);

  free(priv);
  return(ret);
}
/******************************* MAIN ********************************/

/*------------------------------------------------------------------*/
/*
 * The main !
 */
int
main(int	argc,
     char **	argv)
{
  int skfd;		/* generic raw socket desc.	*/
  int goterr = 0;

  /* Create a channel to the NET kernel. */
  if((skfd = iw_sockets_open()) < 0)
    {
      perror("socket");
      return(-1);
    }	
  if(argc <= 1){
  	wps_usage();
   return 0;
  }
    			
	if((!strncmp(argv[1], "-h", 2)) || (!strcmp(argv[1], "--help")))
		wps_usage();
	else
		goterr = set_private(skfd, argv+1 , argc-1, "ra0");
	iw_sockets_close(skfd);
	return(goterr);

}
