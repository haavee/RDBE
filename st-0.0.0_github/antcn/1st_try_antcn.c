/* antcn.c
 *
 * This is the APEX version of antcn (ANTenna CoNtrol program).
 * This version sends a log message whenever it is called.
 * Modified jun04 for patch
 *
 */

/* Input */
/* IP(1) = mode
       0 = initialize LU
       1 = pointing (from SOURCE command)
       2 = offset (from RADECOFF, AZELOFF, or XYOFF commands)
       3 = on/off source status (from ONSOURCE command)
       4 = direct communications (from ANTENNA command)
       5 = on/off source status for pointing programs
       6 = reserved for future focus control
       7 = log tracking data (from TRACK command)
  8 - 99 = reserved for future use
100 - 32767 = for site specific use

   IP(2) = class number (mode 4 only)
   IP(3) = number of records in class (mode 4 only)
   IP(4) - not used
   IP(5) - not used
*/

/* Output */
/*  IP(1) = class with returned message
      (2) = number of records in class
      (3) = error number
            0 - ok
           -1 - illegal mode
           -2 - timeout
           -3 - wrong number of characters in response
           -4 - interface not set to remote
           -5 - error return from antenna
           -6 - error in pointing model initialization
            others as defined locally
      (4) = 2HAN for above errors, found in FSERR.CTL
          = 2HST for site defined errors, found in STERR.CTL
      (5) = not used
*/
/* Eff stuff */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/termios.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <signal.h>
#define APEXTRANSPORT 22122 
/* This host (self) for normal operation*/
/* One of following hosts for testing, don't control antenna*/
//#define APEXTRANSHOST "display2.mpifr-bonn.mpg.de" 
#define APEXTRANSHOST "127.0.0.1"
/* Defined variables */
#define MINMODE 0  /* min,max modes for our operation */
#define MAXMODE 110

/* Include files */

#include <stdio.h>

#include "/usr2/fs/include/params.h" /* FS parameters            */
#include "/usr2/fs/include/fs_types.h" /* FS header files        */
#include "/usr2/fs/include/fscom.h"  /* FS shared mem. structure */
#include "/usr2/fs/include/shm_addr.h" /* FS shared mem. pointer */

#include "/usr2/st/include/stparams.h"
#include "/usr2/st/include/stcom.h"
#include "/usr2/st/include/stm_addr.h"   /* Station shared mem. pointer*/

struct fscom *fs;
//struct stcom *st;

/* Subroutines called */
void setup_ids();
void setup_st();
void putpname();
void skd_run(), cls_clr();
int nsem_test();
void logit();
int rte_prior();

/* antcn main program starts here */
main()
{
  int ierr, nrec, nrecr;
  int dum = 0;
  int r1, r2;
  unsigned int echo_len;
  int imode,i,nchar,n1,n2,nepoch;
  long ip[5], class, clasr;
  char buf[300], buf2[300],buf3[300];
/*  stuff for UDP out stuff */
struct servent *sp;
static int socket_fd;          /* socket file descriptor */
static int echo_fd;          /* socket file descriptor */
struct sockaddr_in sin;   /* our socket address */
struct sockaddr_in echo;   /* back from APEX server */
struct hostent *hp;       /* host address */
   char ceff;
   char suffix[2];
  char *host = APEXTRANSHOST;
  int port = APEXTRANSPORT;
   int nw,nr,rc;
   int flags=0;
//   long lomhz[4]; /*get LO MHz for four LOs*/
   static char effra[12],effdec[11];
   static char effprg[8];              /* project name */
   static char effname[12];           /* source name */
   static char effxtra[11];
   static char effband[9];            /* WHICH RF BAND IN USE */
   static int effpcal,efftcal;   /* pcal 0 off 1 on swcal 0 off 1 on */
   static float fflo;             /* gets lo frequenz*/
   static long ifflo,iffcent; /*same , and rf_centre for comparison with what is read back from system*/
   char off1[30],off2[30];      /*for case 2 offsets: in arcsec*/
   
/* end of eff stuff */

/* Set up IDs for shared memory, then assign the pointer to
   "fs", for readability.
*/
  setup_ids();
  fs = shm_addr;
//  st=stm_addr;
  setup_st();
//  ---------------------------------
      stm_addr->antenna_sentoff=0; /* clear new-source indicator-*/
//  ---------------------------------
/* eff setup socket stuff */
   /* convert host name to hostent structure */
    if (!(hp = gethostbyname(host ? host : APEXTRANSHOST))) {
       printf ("unknown host: \n");
   }
   if ((socket_fd = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror ("socket() failed");
   }
    memset(&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    /* providing host server identity */
    bzero ((char *)&sin, sizeof (sin));
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
    /* port number of the server to the sin structure */
    sin.sin_port = htons(port ? port : APEXTRANSPORT);
    /* Make all sockets blocking */
//    fcntl(socket_fd, F_GETFL, &flags); flags &= ~O_NDELAY; fcntl(socket_fd, F_SETFL, flags);
    fcntl(socket_fd, F_GETFL, &flags); flags &= O_NONBLOCK; fcntl(socket_fd, F_SETFL, flags);
    usleep(100000);  /* a 100millisec sleep to let settle */
    printf("debug:setup EFFCON socket\n");
/* eff end */
/*set up for echo back from APEX*/
     memset(&echo, 0, sizeof(echo));       /* Clear struct */
     echo.sin_family = AF_INET;                  /* Internet/IP */
     echo.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any IP address */
     echo.sin_port = htons(port ? port : APEXTRANSPORT); /*echo port*/
     echo_len=sizeof(echo);
/* set real-time priority */

  rte_prior(FS_PRIOR);

/* Put our program name where logit can find it. */

  putpname("antcn");

/* Return to this point to wait until we are called again */

Continue:
  skd_wait("antcn",ip,(unsigned)0);

  imode = ip[0];
  class = ip[1];
  nrec = ip[2];

      nrecr = 0;  /* these two line inserted March 96 */
      clasr = 0; 

  if (imode < MINMODE || imode > MAXMODE) {
    ierr = -1;
    goto End;
  }

/* Handle each mode in a separate section */
  switch (imode) {

    case 0:             /* initialize */
      ierr = 0;
      strcpy(buf,"Initializing antenna interface");
      logit(buf,0,NULL);
      fs->ionsor = 0;
      strcpy(effprg,"NONE");  /* no program name */
      strcpy(effband,"NONE");  /* no band (receiver) name */
      strcpy(effra,"00h00m00.0s");
      strcpy(effdec,"00d00'00s\"");
      strcpy(effname,"NONE");
      strcpy(effxtra,"1950FSX");
      break;

    case 1:             /* source= command */
      ierr = 0;
//      stm_addr->antenna_sentoff=1; /*2005:signal antenna has been sent off-source*/
  /*seems a bit silly translating back to a string after sorce just
    took the trouble to convert to a real, but here goes. Use a routine
    stolen from VLBA system */
      rad2str(fs->ra50,"h.1",effra);
      rad2str(fs->dec50,"d.0",effdec);
      n1=0; n2=strcut(fs->lsorna,effname,' ',n1); /* look for space at end of source, max 10 char*/
      if(n2 > 9)n2=9; effname[10]='\0';
      nepoch=fs->ep1950;
      if (nepoch < 1900)nepoch=1950;
      fflo=fs->lo.lo[0];
      if(fs->lo.lo[1] > fflo)fflo=fs->lo.lo[1];
      if(fs->lo.lo[2] > fflo)fflo=fs->lo.lo[2]; /*last 2 for VLBA*/
      if(fs->lo.lo[3] > fflo)fflo=fs->lo.lo[3];
      if (fflo !=0.0 )sprintf(effband,"%5.2f",fflo);
          else strcpy(effband,"0");  
      sprintf(effxtra,"%4dFSX",nepoch);
/* rather clumsy section follows to implement geodetic neutral,cw,ccw */
      if (strncmp(fs->cwrap,"ne",2) == 0){sprintf(effxtra,"%4dFSX",nepoch);}
      if (strncmp(fs->cwrap,"cw",2) == 0){sprintf(effxtra,"%4dFSN",nepoch);}
      if (strncmp(fs->cwrap,"cc",2) == 0){sprintf(effxtra,"%4dFSS",nepoch);}
      sprintf(buf2,">>SM %s %s %s %s %s %1d %1d %s",
	effprg,effname,effra,effdec,effband,efftcal,effpcal,effxtra); 
/*eff out to transponder*/
        nw=sendto(socket_fd,buf2,strlen(buf2),flags,                            
                 (struct sockaddr *) &sin, sizeof(sin)); /* Oct01 */  
/*-------------------------rpc client send too ----------------------------*/
//        messageprog_1(APEXTRANSHOST,buf2);
/*-------------------------------------------------------------------------*/
/*      strcpy(buf2,"Sent SOURCE  to EFF200");*/
      logit(buf2,0,NULL);
//      fs->ionsor = 0;
//      set_patch_panel(fs->ifp2vc,fs->lo.pol,fs->lo.lo,ip);
//      set_patch_panel(fs->ifp2vc,ip); /*oops*/
      break;

    case 3:        /* onsource command with error message */
      ierr = 0;
/* The on/off source information for Eff is set by background program antrcv,
   which simply sets ionsor to 0 for slew and 1 for track, this program just
   interprets the value found */
    /*strcpy(buf,"Checking onsource status, extended error logging");*/
      if(fs->ionsor==0)
            strcpy(buf,"Antenna SLEWING");
      else  strcpy(buf,"Antenna TRACKING");
      logit(buf,0,NULL);
      /* June 2007: operator warning message if RF frequency on operator's screen does 
         not match (fictional) LO+150 which we are trying to send out*/
      ifflo=fflo; /*this is nominal LO from schedule to compare with what VAX has set*/
      iffcent=ifflo+150; /*at most frequencies we assume a centre of LO + 150.. */
      if(ifflo == 4640)iffcent=ifflo+350; /*C-band high*/
      if(ifflo >80000)iffcent=ifflo+350;
      sprintf(buf2," RFcentre wanted:%6d Antenna:%6d ",iffcent,stm_addr->ant_rfcentre);
      logit(buf2,0,NULL);
      if(iffcent != stm_addr->ant_rfcentre){
         strcpy(buf2,"WARNING: OPERATOR PLEASE CHECK ULO FREQUENCIES");
         logit(buf2,0,NULL);
      }
      break;

    case 4:            /* direct antenna= command */
      if (class == 0)
        goto End;
      for (i=0; i<nrec; i++) {
        strcpy(buf2,"Received message for antenna: ");
        nchar = cls_rcv(class,buf,sizeof(buf),&r1,&r2,dum,dum);
        buf[nchar] = ' ';  
        buf[nchar+1] = 0;  /* make into a string */
/* Oct 01
        nw=write(socket_fd, buf, nchar);  
*/
/*eff out to transponder, now using sendto*/
        nw=sendto(socket_fd,buf,nchar,flags,(struct sockaddr *) &sin, sizeof(sin));
/*-------------------------anything back? ----------------------------*/
 //     if ((nr = recvfrom(echo_fd, buf3, 300, 0,
 //                                      (struct sockaddr *) &echo_fd,
 //                                      &echo_len)) < 0) {
              usleep(10000);
              if(nr=recvfrom(socket_fd,buf3,300,0,NULL,NULL) <0){
                printf("# of char returned=%d\n",nr);
              } else {
                      buf3[nr]='\0';
                      printf("MSG rcvd: %s %d\n",buf3,nr);
               }

/*-------------------------------------------------------------------------*/
        strcat(buf2,buf);
        logit(buf2,0,NULL);
        strcpy(buf,"ACK");
        cls_snd(&clasr,buf,3,dum,dum);
        nrecr += 1;
      }
 /* or:   cls_clr(class); */
      break;

    case 5:    /* onsource command with no error logging used by onoff and fivpt*/
      /* make sure that if antenna just sent off but not started moving yet,
       * dont falsely report onsrc 2005 October*/
      ierr = 0;
      usleep(10000); /*10 millisec wait, otherwise too much time used in loops*/
      break;

    case 6:            /* reserved */
      ierr = -1;
      goto End;
      break;

    case 7:    /* onsource command with additional info  */
      ierr = 0;
    /*strcpy(buf,"Checking onsource status, log tracking data");*/
      if(fs->ionsor==0)
            strcpy(buf,"Antenna SLEWING");
      else  strcpy(buf,"Antenna TRACKING");
      logit(buf,0,NULL);
      break;

    case 8:    /* call from onoff or fivpt asking for external detector: here dummy  */
      ierr = 0;
      strcpy(buf," ext det not implemented ");
      logit(buf,0,NULL);
      break;


  }  /* end of switch */

End:
  ip[0] = clasr;
  ip[1] = nrecr;
  ip[2] = ierr;
  memcpy(ip+3,"AN",2);
  ip[4] = 0;
  goto Continue;

}

/* - - - - - - - - - - - - - - - - - - - -*/
int strcut(sin,sout,t,n)
char sin[],sout[],t; int n;
/* input is string sin, look for a string in this starting with
   (not ' '),terminated by t, copy to sout, null terminated and
    not including t . Start looking in sin at position n ,
    return finishing position*/
{
  int n1,n2,nn,i;
  n1=locaten(' ',sin,n); n2=locate(t,sin,n1);
  for (i=n1,nn=0; i<n2 ; i++,nn++) sout[nn]=sin[i]; sout[n2-n1]='\0';
  return (n2);
}
/* - - - - - - - - - - - - - - - - - - - -*/
int locate(t,s,n) /*find posn of char t in s starting at posn n*/
char s[],t; int n;
{
  int i,j,k;
  for (i=n;s[i] != '\0';i++) {
      if(s[i]==t)
         return(i);
      }
  return(-1);
}
/* - - - - - - - - - - - - - - - - - - - -*/
int locaten(t,s,n) /*find posn of not char t in s starting at posn n*/
char s[],t; int n;
{
  int i,j,k;
  for (i=n;s[i] != '\0';i++) {
      if(s[i]!=t)
         return(i);
      }
  return(-1);
}
/* - - - - - - - - - - - - - - - - - - - -*/
