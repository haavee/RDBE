/* antcn.c
 *
 * This is the stub version of antcn (ANTenna CoNtrol program).
 * This version sends a log message whenever it is called.
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

/* Defined variables */
#define MINMODE 0  /* min,max modes for our operation */
#define MAXMODE 7

/* Include files */

#include <stdio.h>
#include <string.h>

#include "../include/params.h" /* FS parameters            */
#include "../include/fs_types.h" /* FS header files        */
#include "../include/fscom.h"  /* FS shared mem. structure */
#include "../include/shm_addr.h" /* FS shared mem. pointer */
#include "antcn_ar.h"

struct fscom *fs;

/* Subroutines called */
void setup_ids();
void putpname();
void skd_run(), cls_clr();
int nsem_test();
void logit();
int rte_prior();

/* antcn main program starts here */
main()
{
  int stat;
  int ierr, nrec, nrecr,len;
  int dum = 0;
  int r1, r2;
  int imode,i,nchar;
  long ip[5], class, clasr;
  char buf[256], buf2[100], source[100];

/* Set up IDs for shared memory, then assign the pointer to
   "fs", for readability.
*/
  setup_ids();
  fs = shm_addr;

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

  nrecr = 0;
  clasr = 0;

  if (imode < MINMODE || imode > MAXMODE) {
    ierr = -1;
    goto End;
  }

/* Handle each mode in a separate section */

  ar_timeout_enable();
  memcpy(source, fs->lsorna, sizeof(fs->lsorna));
  source[sizeof(fs->lsorna)] = 0;
  fix_source(source);

  switch (imode) {

    case 0:             /* initialize */
      ierr = 0;
      strcpy(buf,"Initializing antenna interface");
      logit(buf,0,NULL);
      ar_antcn_init();
      fs->ionsor = 0;
      break;

    case 1:             /* source= command */
      ierr = 0;
      strcpy(buf,"Commanding to a new source");
      logit(buf,0,NULL);
      if(fs->ep1950 > 1949.9 && fs->ep1950 < 1950.1)
        ar_antcn_radec_B1950(source, fs->ra50,fs->dec50);
      else if(fs->ep1950 > 1999.9 && fs->ep1950 < 2000.1)
        ar_antcn_radec_J2000(source,fs->ra50,fs->dec50);
      else
        ar_antcn_radec_now(source, fs->radat,fs->decdat);
      fs->ionsor = 0;
      break;

    case 2:             /* offsets         */
      ierr = 0;
      strcpy(buf,"Commanding new offsets");
      logit(buf,0,NULL);
      if(fs->ep1950 > 1949.9 && fs->ep1950 < 1950.1)
        ar_antcn_radec_offset(source, "B", fs->ra50, fs->dec50,
             fs->RAOFF,fs->DECOFF);
      else if(fs->ep1950 > 1999.9 && fs->ep1950 < 2000.1)
        ar_antcn_radec_offset(source, "J", fs->ra50, fs->dec50,
             fs->RAOFF,fs->DECOFF);
      else
        ar_antcn_radec_offset(source, "H", fs->radat, fs->decdat,
             fs->RAOFF,fs->DECOFF);
      fs->ionsor = 0;
      break;

    case 3:        /* onsource command with error message */
      ierr = 0;
      strcpy(buf,"Checking onsource status");
      logit(buf,0,NULL);
      stat = ar_antcn_onsource_elog();
      if(stat ==1) {
        strcpy(buf,"on source");
        fs->ionsor = 1;
      } else {
        strcpy(buf,"off source");
        fs->ionsor = 0;
      }
      logit(buf,0,NULL);
      break;

    case 4:            /* direct antenna= command */
      if (class == 0)
        goto End;
      for (i=0; i<nrec; i++) {
        strcpy(buf2,"Received message for antenna: ");
        nchar = cls_rcv(class,buf,sizeof(buf),&r1,&r2,dum,dum);
        buf[nchar] = 0;  /* make into a string */
        strcat(buf2,buf);
        logit(buf2,0,NULL);
        stat = ar_antcn_direct(buf2,buf,sizeof(buf));
        if(stat < 0)
          strcpy(buf,"ERR");
        len = strlen(buf);
        cls_snd(&clasr,buf,len,dum,dum);
        nrecr += 1;
      }
      break;

    case 5:    /* onsource command with no error logging */
      ierr = 0;
/*
      strcpy(buf,"Checking onsource status");
      logit(buf,0,NULL);
*/
      stat = ar_antcn_onsource_noelog();
      if(stat ==1)
        fs->ionsor = 1;
      else
        fs->ionsor = 0;
      break;

    case 6:            /* reserved */
      ierr = -1;
      goto End;
      break;

    case 7:    /* onsource command with additional info  */
      ierr = 0;
      strcpy(buf,"Checking onsource status, log tracking data");
      logit(buf,0,NULL);
      stat = ar_antcn_onsource_extra();
      if(stat ==1)
        fs->ionsor = 1;
      else
        fs->ionsor = 0;
      break;

  }  /* end of switch */
  ar_timeout_disable();
End:
  ip[0] = clasr;
  ip[1] = nrecr;
  ip[2] = ierr;
  memcpy(ip+3,"AN",2);
  ip[4] = 0;
  goto Continue;
}

fix_source(s)
char *s;
{
  while(*s) {
    if( *s == ' ' ) {
      *s = 0;
      return;
    }
    s++;
  }
}
