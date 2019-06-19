/*----------------------------------------------------------*
 * Port Client for Antcn program
 *
 *  this program implements the software to communicate
 *  with the CIMA executive
 *  ar_antcn_init() - open socket
 *  ar_antcn_radec_now - Command RA and DEC at epoch NOW
 *  ar_antcn_radec_offset - Command RA and DEC offset 
 * 
 * NOTE - This program uses signal SIGALRM to timeout from
 *   IO operations after 4 seconds
 *---------------------------------------------------------*/
#include <stdio.h>
 
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "antcn_ar.h"

extern int errno;
/*-------------------------------------------------------*
 * These error mirror the error conditions listed in
 *  /usr2/control/sterr.ctl
 *------------------------------------------------------*/
#define ST_ERR_CONN_PORT  -100
#define ST_ERR_RECONNECT  -101   /*- Reconnect Failed -*/

#define AN_ERR_TRACKERR   -103   /*-- Tracking Errors TOO large --*/
#define AN_ERR_TIMEOUT      -2   /*-- Timeout waiting for response --*/
#define AN_ERR_RGARBLED     -3   /*- Antenna Response Garbled --*/
#define AN_ERR_ERRRET       -5   /*-- Error Return from Telescope --*/
#define AN_ERR_ANTRESTORED  -7  /*-- Antenna Communications Restored ! -*/

#define MAX_CMD  256
#define MAX_RESP 1000

#define ST_LOGIT_DEBUG   0  /* Use this for debug messages ONLY */
#define ST_LOGIT_CECHO   1  /* Command Echo */
#define ST_LOGIT_RECHO   2  /* Response Echo */
#define ST_LOGIT_AN      3  /* Field System Antenna (AN) message  */ 
#define ST_LOGIT_ST      4  /* Field System Station (ST) message  */ 

int mylogit(char *,int,int);

int readresp(int,char *,int);
/* #define NDEBUG */
/*-------------------------------------------------------------
 * MY Logit - Note
 *   This routine automatically appends "ST" or "AN"
 *   Also, it 
 *-------------------------------------------------------------*/
int mylogit(char *msg,int ierr,int severity)
{
#ifdef NDEBUG
  if(severity == ST_LOGIT_DEBUG)
    fprintf(stderr,"DEBUG %d: %s\n",ierr,msg);
  else if(severity == ST_LOGIT_CECHO)
    fprintf(stderr,"CMD %d: %s\n",ierr,msg); 
  else if(severity == ST_LOGIT_RECHO)
    fprintf(stderr,"RESP %d: %s\n",ierr,msg);
  else if(severity == ST_LOGIT_AN)
    fprintf(stderr,"AN ERROR %d: %s\n",ierr,msg);
  else if(severity == ST_LOGIT_ST)
    fprintf(stderr,"ST ERROR %d: %s\n",ierr,msg);
#else
  if(severity == ST_LOGIT_AN)
    logit(msg,ierr,"AN");
  else if(severity == ST_LOGIT_ST)
    logit(msg,ierr,"ST");
#endif
  return 0;
}
/*----------------------------------------------------------*
 * Sets an alarm to avoid locking up the system
 *----------------------------------------------------------*/
struct sigaction oact;
void alarm_stub(int);

int ar_timeout_enable()
{
  struct sigaction act,oact;
  act.sa_handler = alarm_stub;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if(sigaction(SIGALRM,&act,&oact) <0)
    return(-1);
  alarm(4);
  return 0;
}

int ar_timeout_disable()
{
    alarm(0);
    sigaction(SIGALRM,&oact,NULL);
}

void alarm_stub(int signo)
{
    return;
}
/*----------------------------------------------------------*
 *  Initialization of Phoenix Socket
 *    This routine tries to open the Socket interface to
 *  Phoenix as a client.  IF the socket is opened successfully,
 *  
 *
 *----------------------------------------------------------*/
#define IPC_SIZE 50
int connected = NO;  /*-- Are we connected? --*/

int ar_antcn_init()
{
  static int bound = 0;
  int exec;
   
  mylogit("Initializing with ar_antcn_init()",0,ST_LOGIT_DEBUG);

  if( !bound ) {
    setenv( "MAILBOXDEFS",  "/usr2/st/mailboxdefs", 1 );
    sock_bind("VLBIANTCN");
  }

  exec = sock_connect("EXECUTIVE");
  if( sock_send(exec, "display_log {WARN VLBI control initialized}")<0 )
    mylogit("Error initializing CIMA connection, running?",0,ST_LOGIT_DEBUG);

  connected = YES;  /*-- OK we are ready to go --*/

  return(0);
}
/*-------------------------------------------------------------*
 * Antenna Source Command Received
 *   Input is RA and DEC J2000 - AV001218
 *------------------------------------------------------------*/
int ar_antcn_radec_J2000(char *name, double raj2000,double decj2000)
{
  char cmd_buf[100];
  char ack_buf[100];

  if(connected == NO)
     ar_antcn_init(); 

  snprintf(cmd_buf,100, "vlbi_trackj %s %f %f",name, raj2000,decj2000);
 
  if( sock_send( sock_connect("EXECUTIVE"), cmd_buf ) < 0 ) {
    mylogit("Failed J2000 Command",AN_ERR_RGARBLED,ST_LOGIT_AN);
    return(-1);
  } 

  if( sock_wait(NULL, 5) <0 )
    return(-1);
  else
    return(0);
}

/*-------------------------------------------------------------*
 * Antenna Source Command Received
 *   Input is RA and DEC B1950 - AV001218
 *------------------------------------------------------------*/
int ar_antcn_radec_B1950(char *name, double rab1950,double decb1950)
{
  char cmd_buf[100];

  if(connected == NO)
     ar_antcn_init(); 

  snprintf(cmd_buf,100, "vlbi_trackb %s %f %f",name, rab1950,decb1950);
 
  if( sock_send( sock_connect("EXECUTIVE"), cmd_buf ) < 0 ) {
    mylogit("Failed B1950 Command",AN_ERR_RGARBLED,ST_LOGIT_AN);
    return(-1);
  } 

  if( sock_wait(NULL, 5) <0 )
    return(-1);
  else
    return(0);
}

/*-------------------------------------------------------------*
 * Antenna Source Command Received
 *   Input is RA and DEC now!
 *------------------------------------------------------------*/

int ar_antcn_radec_now(char *name, double radat,double decdat)
{
  char cmd_buf[100];

  if(connected == NO)
     ar_antcn_init(); 

  snprintf( cmd_buf, 100, "vlbi_track_now %s %f %f", name, radat, decdat );
 
  if( sock_send( sock_connect("EXECUTIVE"), cmd_buf ) < 0 ) {
    mylogit("Failed track now Command",AN_ERR_RGARBLED,ST_LOGIT_AN);
    return(-1);
  } 

  if( sock_wait(NULL, 5) <0 )
    return(-1);
  else
    return(0);
  return(0);
}

/*-------------------------------------------------------------*
 * Antenna Offset Command Received
 *   Input is RA and DEC offset!
 *------------------------------------------------------------*/

ar_antcn_radec_offset(name, epoch, ra, dec, raoff, decoff )
char *name;
char *epoch;
double ra;
double dec;
double raoff;
double decoff;
{
  char cmd_buf[100];

  if(connected == NO)
     ar_antcn_init(); 

  snprintf( cmd_buf, 100, "vlbi_track %s %s %f %f %f %f", 
    name, epoch, ra, dec, raoff, decoff );
 
  if( sock_send( sock_connect("EXECUTIVE"), cmd_buf ) < 0 ) {
    mylogit("Failed offset Command",AN_ERR_RGARBLED,ST_LOGIT_AN);
    return(-1);
  } 

  if( sock_wait(NULL, 5) <0 )
    return(-1);
  else
    return(0);
  return(0);
}

/*
 * Antenna On Source Checks
 *   - elog -> with error logging
 *   - noelog -> without error logging
 *   - extra -> extra error logging
 * Returns:
 *  <0 - Error
 *  0 = No Error, slewing
 *  1 = No Error, On Source
 */

ar_antcn_onsource_elog()
{
  char cmd_buf[100];
  char ant_resp[100];

  if(connected == NO)
     ar_antcn_init(); 

  strncpy( cmd_buf, "vlbi_onsource", 100 );
 
  if( sock_send( sock_connect("EXECUTIVE"), cmd_buf ) < 0 ) {
    mylogit("Failed onsource Command",AN_ERR_RGARBLED,ST_LOGIT_AN);
    return(-1);
  } 

  ant_resp[0] = 0;
  if( sock_wait(ant_resp, 5) <0 )
    return(-1);

  if( strncmp( ant_resp, "TRACKING", 8 ) == 0 )
    return(1);
  else
    return(0);
}


ar_antcn_onsource_noelog()
{
  return(ar_antcn_onsource_elog());
}
ar_antcn_onsource_extra()
{
  return(ar_antcn_onsource_elog());
}

/*
 * Antenna Direct Command
 */

int ar_antcn_direct(char *cmd,char *ack,int nack)
{
  int nlen;
  char *mbuf;
  
  nlen = strlen(cmd);
  if(nlen > 2000)
  {
    mylogit("Failed Antenna Direct Command, Excessive Buffer",0,ST_LOGIT_DEBUG);
    return(-1);
  }

  if(connected == NO)
     ar_antcn_init(); 

  if( sock_send( sock_connect("EXECUTIVE"), cmd ) < 0 ) {
    mylogit("Failed onsource Command",AN_ERR_RGARBLED,ST_LOGIT_AN);
    return(-1);
  } 

  ack[0] = 0;
  if( sock_wait(ack, 5) <0 )
    return(-1);

  return(0);
}


/* wait for response from socket library */

sock_wait( ack, tim )
char *ack;
int tim;
{
  char wt[8192];
  int len;

  len = 8192;
  if( sock_sel( wt, &len, NULL, 0, tim ) < 0 ) {
    mylogit("Timeout on ACK",AN_ERR_RGARBLED,ST_LOGIT_AN);
    return (-1);
  }

  if( ack )
    strncpy( ack, wt, 100 );

  return (0);
}

