/*
 * security.c -- Security handler
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 */

/******************************** Description *********************************/

/*
 *      This module provides a basic security policy.
 */

/********************************* Includes ***********************************/

#include        "wsIntrn.h"
#include        "um.h"
#ifdef DIGEST_ACCESS_SUPPORT
#include        "websda.h"
#endif
#ifdef _LOGIN_WEB_
	#include <time.h>
	#include "apform.h"
#endif
/********************************** Defines ***********************************/
/*
 *      The following #defines change the behaviour of security in the absence
 *      of User Management.
 *      Note that use of User management functions require prior calling of
 *      umInit() to behave correctly
 */

#ifndef USER_MANAGEMENT_SUPPORT
#define umGetAccessMethodForURL(url) AM_FULL
#define umUserExists(userid) 0
#define umUserCanAccessURL(userid, url) 1
#define umGetUserPassword(userid) websGetPassword()
#define umGetAccessLimitSecure(accessLimit) 0
#define umGetAccessLimit(url) NULL
#endif

#define LOGIN_TIMEOUT (60000*60) // tommy --> changed it to 60 min

static time_t last_access_time = {0};
int login=0;
extern int logout;
int safari_logout=0;

/******************************** Local Data **********************************/

static char_t   websPassword[WEBS_MAX_PASS];    /* Access password (decoded) */
// marked by david
#if 0
#ifdef _DEBUG
static int              debugSecurity = 1;
#else
static int              debugSecurity = 0;
#endif
#endif

/*********************************** Code *************************************/
/*
 *      Determine if this request should be honored
 */

#ifdef _NULL_LOGIN_
	#include "apmib.h"
#endif

int websSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
                                                char_t *url, char_t *path, char_t *query)
{
#ifdef _Phoebe_
        if ( !strcmp(url, "/set.css") || !strcmp(url, "/just_close.asp") || !strcmp(url, "/goform/formAlreadyLogout") )
                  return 0;
#endif

#ifdef _NULL_LOGIN_

	char tmpbuf[100];
	int ret;

	apmib_get(MIB_USER_PASSWORD, tmpbuf);
	if (tmpbuf[0] == '\0')
		return 0;

#endif

#ifdef _LOGIN_WEB_
	//struct sysinfo getmaintime;
	int maintime;
	int i=0;
	char currentKey[10];
	int isFind=0;

	if(!strstr(url,"login.asp") && !strstr(url,"/file/") && !strstr(url,"/goform/formLogin") && !strstr(url,"/goform/formApply") && !strstr(url,"/FUNCTION_SCRIPT")){
		for(i=0;i<_Login_IDNUM_;i++){
			sprintf(currentKey,"KeyID=%d",KeyID[i]);		
			if(strstr(url,currentKey)){
				maintime = time((time_t *) NULL);
				if ( (maintime - IDtime[i]) < _LOGIN_WEB_ ){
					isFind = 1;
					IDtime[i] = maintime;
				}
				else{
					IDtime[i] = -1;
					KeyID[i] = -1;
				}
				break;
			}else{
				isFind = 0;
			}
		}
		if(!isFind){
        //char_t *type, *userid, *password, *accessLimit, *userpass;
        //int flags, nRet;
			//websSetRequestFlags(wp, 64);
        		/*type = websGetRequestType(wp);
        		password = websGetRequestPassword(wp);
        		userid = websGetRequestUserName(wp);*/
			websRedirect(wp, T("login.asp"));
			return 1;
		}
	}
	return 0;
#endif	

#if defined(_STAR_LOGIN_WEB_)
                return 0;
#endif	
	
        char_t *type, *userid, *password, *accessLimit, *userpass;
        int flags, nRet;
        accessMeth_t    am;

        a_assert(websValid(wp));
        a_assert(url && *url);
        a_assert(path && *path);
/*
 *      Get the critical request details
 */
        type = websGetRequestType(wp);
        password = websGetRequestPassword(wp);
        userid = websGetRequestUserName(wp);
        flags = websGetRequestFlags(wp);

#if defined(_LAN_WAN_ACCESS_) && defined(__TARGET_BOARD__)
//----------------------------------
	FILE *fp;
	int i, interfaceNum;
	char *ptr;
	char buf[100]={0},macaddr[20]={0}, comment[50]={0}, comment2[100]={0};
	sprintf(comment,"cat /proc/net/arp | grep %s > /var/abc.txt",wp->ipaddr);
	system(comment);
	if ((fp = fopen("/var/abc.txt","r"))!=NULL)
	{
		while (fgets(buf,100, fp)>0){
			for (i=0;i<=1;i++) {
				if ((ptr = strchr(buf, ' ')) != NULL) 
					while (*ptr == ' ' ) ptr++;
				sprintf(buf,"%s",ptr);
			}
			fclose(fp);
			system("rm -f /var/abc.txt");
			snprintf(macaddr, 18, "%s", ptr);
		}
	}

	sprintf(comment2,"brctl showmacs br0 | grep -i \"%s\" | tr \\\\t \" \" | tr -s \" \" | cut -d \" \" -f2 > /var/aaa.txt", macaddr);
	//printf("Running -> %s\n", comment2);
	system(comment2);

	if ((fp = fopen("/var/aaa.txt","r"))!=NULL)
	{
		while(fgets(buf,100,fp)>0){
		
			interfaceNum=strtol( buf, (char **)NULL, 10);
			//printf("iterfaceNum=%d",interfaceNum);
			fclose(fp);
			system("rm -f /var/aaa.txt");
			if(interfaceNum>=3)
				return 1;
		}
	}
//-----------------------------------
#endif
/*
 *      Get the access limit for the URL.  Exit if none found.
 */
        accessLimit = umGetAccessLimit(path);
        if (accessLimit == NULL) {
                return 0;
        }

/*
 *      Check to see if URL must be encrypted
 */
#ifdef WEBS_SSL_SUPPORT
        nRet = umGetAccessLimitSecure(accessLimit);
        if (nRet && ((flags & WEBS_SECURE) == 0)) {
                websStats.access++;
                websError(wp, 405, T("Access Denied\nSecure access is required."));
                trace(3, T("SEC: Non-secure access attempted on <%s>\n"), path);
      /* bugfix 5/24/02 -- we were leaking the memory pointed to by
       * 'accessLimit'. Thanks to Simon Byholm.
       */
      bfree(B_L, accessLimit);
                return 1;
        }
#endif

/*
 *      Get the access limit for the URL
 */
        am = umGetAccessMethodForURL(accessLimit);

        nRet = 0;

// for debug, david /////////////////////////////////
#if 0
        if ((flags & WEBS_LOCAL_REQUEST) && (debugSecurity == 0)) {
/*
 *              Local access is always allowed (defeat when debugging)
 */
        } else if (am == AM_NONE) {
#endif
        if (am == AM_NONE) {

/////////////////////////////////////////////////////

/*
 *              URL is supposed to be hidden!  Make like it wasn't found.
 */
                websStats.access++;
                websError(wp, 404, T("Page Not Found"));
                nRet = 1;
        } else  if (userid && *userid) {
        
#ifdef _airlive_
        userpass = umGetUserPassword("airlive");
#elif defined(_cnet_)
					userpass = umGetUserPassword("root");
#elif defined(_corega_)
					userpass = umGetUserPassword("root");
#else
        userpass = umGetUserPassword("admin");
#endif


#ifndef _corega_
        if (!(userpass && *userpass)){
            goto passIsNull;//Password can set null Erwin 08.06
                }
#endif


                if (!umUserExists(userid)) {
                        websStats.access++;
// for debug
//                      websError(wp, 200, T("Access Denied\nUnknown User"));
websError(wp, 401, T("Access Denied\nUnknown User"));

                        trace(3, T("SEC: Unknown user <%s> attempted to access <%s>\n"),
                                userid, path);
                        nRet = 1;
                } else if (!umUserCanAccessURL(userid, accessLimit)) {
                        websStats.access++;
                        websError(wp, 403, T("Access Denied\nProhibited User"));
                        nRet = 1;
#ifdef _corega_
                }else if (1) {
                	
#else
                }else if (password && * password) {
#endif
//                      char_t * userpass = umGetUserPassword(userid);
                        if (userpass) {
                                if (gstrcmp(password, userpass) != 0) {
                                        websStats.access++;
// for debug
//                                     websError(wp, 200, T("Access Denied\nWrong Password"));
websError(wp, 401, T("Access Denied\nWrong Password"));
                                        trace(3, T("SEC: Password fail for user <%s>")
                                                                T("attempt to access <%s>\n"), userid, path);
                                        nRet = 1;
                                }
                                else if((login &&
                                        ((time(0) - last_access_time)*1000 > LOGIN_TIMEOUT))
                                        || logout){
                                        websError(wp, 401, T("Access Timeout\nPlease login again"));
                                        if( strstr(wp->userAgent, "Safari") != NULL )
                                        {
                                                if( safari_logout < 1 )
                                                {
                                                        safari_logout += 1;
                                                }
                                                else
                                                {
                                                        safari_logout = 0;
                                                        login=0;
                                                        logout=0;
                                                }                                                
                                        }
                                        else
                                        {
                                                login=0;
                                                logout=0;
                                        }
                                        nRet = 1;

                                } 
				 else {
/*
 *                                      User and password check out.
 */
                       															         }

passIsNull://Erwin 08.06
                                bfree (B_L, userpass);
                        }
#ifdef DIGEST_ACCESS_SUPPORT
                } else if (flags & WEBS_AUTH_DIGEST) {

                        char_t *digestCalc;

/*
 *                      Check digest for equivalence
 */
                        wp->password = umGetUserPassword(userid);

                        a_assert(wp->digest);
                        a_assert(wp->nonce);
                        a_assert(wp->password);

                        digestCalc = websCalcDigest(wp);
                        a_assert(digestCalc);

                        if (gstrcmp(wp->digest, digestCalc) != 0) {
                                websStats.access++;
            /* 16 Jun 03 -- error code changed from 405 to 401 -- thanks to
             * Jay Chalfant.
             */
                                websError(wp, 401, T("Access Denied\nWrong Password"));
                                nRet = 1;
                        }

                        bfree (B_L, digestCalc);
#endif
                } else {
/*
 *                      No password has been specified
 */
#ifdef DIGEST_ACCESS_SUPPORT
                        if (am == AM_DIGEST) {
                                wp->flags |= WEBS_AUTH_DIGEST;
                        }
#endif
                        websStats.errors++;
                        websError(wp, 401,
                                T("Access to this document requires a password"));
                        nRet = 1;
                }
        } else if (am != AM_FULL) {
/*
 *              This will cause the browser to display a password / username
 *              dialog
 */
#ifdef DIGEST_ACCESS_SUPPORT
                if (am == AM_DIGEST) {
                        wp->flags |= WEBS_AUTH_DIGEST;
                }
#endif
                websStats.errors++;
			#ifdef _pci_
                websError(wp, 401, T("�ݒ�y�[�W�ɐڑ�����??��[�U�[ID�ƃp�X���[�h���K�v�ł�."));
			#else
                websError(wp, 401, T("Access to this document requires a User ID"));
			#endif
                nRet = 1;
        }

        bfree(B_L, accessLimit);

        return nRet;
}

/******************************************************************************/
/*
 *      Delete the default security handler
 */

void websSecurityDelete()
{
        websUrlHandlerDelete(websSecurityHandler);
}

/******************************************************************************/
/*
 *      Store the new password, expect a decoded password. Store in websPassword in
 *      the decoded form.
 */

void websSetPassword(char_t *password)
{
        a_assert(password);

        gstrncpy(websPassword, password, TSZ(websPassword));
}

/******************************************************************************/
/*
 *      Get password, return the decoded form
 */

char_t *websGetPassword()
{
        return bstrdup(B_L, websPassword);
}

/******************************************************************************/


