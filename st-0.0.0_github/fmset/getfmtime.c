/* getfmtime.c - get formatter time */

#include <stdio.h>
#include <sys/types.h>   /* data type definition header file */
#include <stdlib.h>
#define RDBE 0x111111    //XXXX
#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"
#include "/usr2/fs/include/shm_addr.h"

#include "fmset.h"

void getvtime();
void get4time();
extern int rack;
extern int source;
extern int s2type;
extern char s2dev[2][3];

void getfmtime(unixtime,unixhs,fstime,fshs,formtime,formhs,m5sync,sz_m5sync,
	       m5pps,sz_m5pps,m5freq,sz_m5freq,m5clock,sz_m5clock)
time_t *unixtime; /* computer time */
int    *unixhs;
time_t *fstime; /* field system time */
int    *fshs;
time_t *formtime; /* formatter time */
int    *formhs;
char *m5sync;
int sz_m5sync;
char *m5pps;
int sz_m5pps;
char *m5freq;
int sz_m5freq;
char *m5clock;
int sz_m5clock;
{
  static long phase =-1;
  long raw, sleep, rawch;
  if (nsem_test(NSEM_NAME) != 1) {
    endwin();
    fprintf(stderr,"Field System not running - fmset aborting\n");
    rte_sleep(SLEEP_TIME);
    exit(0);
  }
  if(source == RDBE){  //XXX
    rte_sleep(10);  //XXX or maybe 10...
    rte_ticks(&raw);  //XXX
//    sleep=101-(raw%100+phase)%100;  //XXX
    sleep=90-(raw%100+phase)%100;  //XXX  cant get same sec..
    if(sleep >=0) {  //XXX
      rte_sleep(sleep);   //XXX
    getrdbetime(unixtime,unixhs,fstime,fshs,formtime,formhs,&rawch,m5sync,
	      sz_m5sync,m5pps,sz_m5pps,m5freq,sz_m5freq,m5clock,sz_m5clock);
    if(*formhs > -1 && *formhs < 100) { phase=(100+*formhs-rawch%100)%100; }
    return(0);
   }
  }  //XXX
  if (source == MK5) {
    rte_sleep(10);
    rte_ticks(&raw);
    sleep=102-(raw%100+phase)%100;
    if(sleep >=0) {
      rte_sleep(sleep); 
    }
    get5btime(unixtime,unixhs,fstime,fshs,formtime,formhs,&rawch,m5sync,
	      sz_m5sync,m5pps,sz_m5pps,m5freq,sz_m5freq,m5clock,sz_m5clock);
    if(*formhs > -1 && *formhs < 100) {
      phase=(100+*formhs-rawch%100)%100;
    }
  }  else if (source == S2) {
    gets2time(s2dev[s2type],unixtime,unixhs,fstime,fshs,formtime,formhs);
  } else if(rack&VLBA)
      getvtime(unixtime,unixhs,fstime,fshs,formtime,formhs);
  else {
    rte_sleep(10);
    rte_ticks(&raw);
    sleep=102-(raw%100+phase)%100;
    if(sleep >=0) {
      rte_sleep(sleep); 
    }
    get4time(unixtime,unixhs,fstime,fshs,formtime,formhs,&rawch);
    if(*formhs > -1 && *formhs < 100) {
      phase=(100+*formhs-rawch%100)%100;
    }
  }
}

