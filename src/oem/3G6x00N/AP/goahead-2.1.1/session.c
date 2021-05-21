/*************************************************************************
 *      session support for GoAhead Webserver
 *       
 *       Synopsis:
 *       The session id is stored in user password, which is automatically
 *       created by "websSessionHandler". Currently sessions never expire.
 *                  
 *       Usage:
 *       Two ASP functions are defined.
 *       1) SessionGetVar(varname, global)
 *       2) SessionSetVar(varname, global)
 *       where varname is the name of variable in string, global is an optional
 *       argument, when set to "1", sharing variable among sessions is possible.
 *                                            
 *       Example:
 *       // a auto varible x is created
 *       x = 1;
 *       // create a session varible with the value of 1
 *       SessionSetVar("x");
 *       x = 7;
 *       write(x);  // return 7
 *       SessionGetVar("x");
 *       write(x)   // return 1
 *                                                                                     
 *       Note that SessionGetVar("varname") will always create a ejVar named 
 *       varname even if the session varible couldn't be found.
 **************************************************************************/
 
 
#include <wsIntrn.h>
 
sym_fd_t sessions;
 
int sessionInit()
{
 sym_fd_t globalsession;
 value_t value;
 
 sessions = symOpen(64);
 
 /* Global Session variables */
 globalsession = symOpen(64);
 value = valueInteger(globalsession);
 
 /* create a default session to hold global variables */
 symEnter(sessions,"0",value,0);
 
 return (sessions?0:-1) ;
}
 
static void sessionVarsFree(sym_t *sessionVar)
{
  symClose( (sym_fd_t) sessionVar->content.value.integer, NULL);
}
 
void sessionClose()
{
  symClose(sessions,sessionVarsFree);
}
 
/*
 *  * add this line in  main.c
 *   *
 *  *  websUrlHandlerDefine(T(""), NULL, 0, websSessionHandler, WEBS_HANDLER_FIRST); 
 *   */
 
int websSessionHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
                       char_t *url, char_t *path, char_t *query)
{
 char newuser[16];
 sym_t *session;
 char *username=NULL,*password=NULL;
 sym_fd_t sessionVar=0;
 value_t value;
 unsigned long uniqueid=0;
 
 password = websGetRequestPassword(wp);
 
 session = symLookup(sessions,password);
 
 if ( !session )
 {
  if ( !password )
  {
   if (wp->query && strcmp("login",wp->query)==0)
   {
    /* ask browser to send a username and password by returning 401 */
 
    websWrite(wp, T("HTTP/1.1 401 Unauthorized\r\n"));
    websWrite(wp, T("Server: GoAhead-Webs\r\n"));
    websWrite(wp, T("Pragma: no-cache\r\n"));
    websWrite(wp, T("Cache-control: no-cache\r\n"));
    websWrite(wp, T("Content-Type: text/html\r\n"));
    websWrite(wp, T("WWW-authenticate: Basic realm=\"/\"\r\n"));
    websWrite(wp, T("\r\n"));
    websWrite(wp, T("<html>You shouldn't see this screen</html>\r\n"));
    websDone(wp,0);
 
   }
   else
   {
     /* 
    *       * Customize here to generate your own unique session id
    *             * for Windows 95/98/NT , i simply use gettickcount()    
      *                   */
 
       uniqueid = gettickcount();
       sprintf(newuser,"%.8x",uniqueid);
       password=newuser;
   }
  }
 
  if (password || password==newuser)
  {
   sessionVar = symOpen(64);
   value = valueInteger(sessionVar);
   symEnter(sessions,password,value,uniqueid);
 
   value = valueString(password,VALUE_ALLOCATE);
   symEnter(sessionVar, "session", value,0);
 
   websHeader(wp);
 
   /* customize here to fit your need */
   websWrite(wp,"<meta http-equiv=refresh content=0;url=http://g:%s@%s:%d/home.asp?login><body bgcolor=black>
                 <table border=0 width=100 height=100>
                 <tr><td width=100%%><p align=center><font color=#00ff00>Creating Session....</font></td></tr>
                 </table></HTML>",
                 password, wp->host, wp->port);
   websDone(wp, 200);
  }
 
 }
 else
 {
  return 0;
 }
 
 return 0;
}
 
static sym_t *getSession(int global, unsigned char *sid)
{
 if (global)
 {
  return symLookup(sessions,"0");
 }
 else
 {
  return symLookup(sessions,sid);
 }
}
 
/* create a javascript local var from the sessionvars */
int SessionGetVar(int eid, webs_t wp, int argc, char_t **argv)
{
 unsigned char *varname;
 unsigned long global=0;
 unsigned char numberbuffer[32];
 
 sym_t *session,*var;
 sym_fd_t sessionVar;
 
 if (ejArgs(argc, argv, T("%s %d"), &varname, &global) < 1) {
  websWrite(wp, T("Insufficient args\n"));
  return 0;
 }
 
 
 /* use password as sid */
 session = getSession(global, wp->password);
 
 if (!session)
 {
  websWrite(wp,T("No Session Info"));
  return 0;
 }
 
 sessionVar = (sym_fd_t) session->content.value.integer;
 
 /* lookup the var name */
 var = symLookup(sessionVar,varname);
 
 if (var)
 {
  if (var->content.type == string)
  {
   ejSetVar(eid, varname, var->content.value.string);
  }
  else if (var->content.type == integer)
  {
   stritoa(var->content.value.integer,numberbuffer,sizeof(numberbuffer));
   ejSetVar(eid, varname, numberbuffer);
  }
  else
  {
   ejSetVar(eid, varname , "");
  }
 }
 else
 {
  ejSetVar(eid, varname , "");
 }
 return 0;
}
 
/* put the content of javascript var to session var */
int SessionSetVar(int eid, webs_t wp, int argc, char_t **argv)
{
 unsigned char *varname=NULL, *varvalue=NULL;
 unsigned long global=0;
 value_t value;
 
 sym_t *session;
 sym_fd_t sessionVar;
 if (ejArgs(argc, argv, T("%s %d"), &varname, &global) < 1) {
  websWrite(wp, T("Insufficient args\n"));
  return 0;
 }
 
 ejGetVar(eid,varname, &varvalue);
 if (!varvalue)
 {
  websWrite(wp,T("ej var has no value"));
  return 0;
 }
 
 session = getSession(global,wp->password);
 
 if (!session)
 {
  websWrite(wp,T("No Session Info"));
  return 0;
 }
 
 sessionVar = (sym_fd_t) session->content.value.integer;
 
 value = valueString(varvalue,VALUE_ALLOCATE);
 symEnter(sessionVar , varname, value, 0);
 return 0;
}
 
/* call this routine in main.c */
int buildSessionInterface()
{
 websAspDefine(T("SessionGetVar"), SessionGetVar);
 websAspDefine(T("SessionSetVar"), SessionSetVar);
 return 0;
}
