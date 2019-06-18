/* setfmtime.c - set formatter time */

#include <stdio.h>
#include <sys/types.h>   /* data type definition header file */
#include <stdlib.h>
#include "/usr2/fs/include/params.h"

#include "fmset.h"
#define RDBE 0x111111    //XXXX
void setvtime();
void set4time();

extern int rack;
extern int source;
extern int s2type;
extern char s2dev[2][3];

void setfmtime(formtime,delta)
time_t formtime;
int delta;
{

if (nsem_test(NSEM_NAME) != 1) {
  endwin();
  fprintf(stderr,"Field System not running - fmset aborting\n");
  rte_sleep(SLEEP_TIME);
  exit(0);
}
  if(source == RDBE)  //XXX
//    setrdbetime(formtime,delta); //XXX
    set5btime(formtime,delta); //take ou
  else if (source == MK5 ) //XXX
    set5btime(formtime,delta);
  else if (source == S2)
    sets2time(s2dev[s2type],formtime+delta);
  else if (rack & VLBA)
    setvtime((time_t) (formtime + delta));
  else
    set4time(formtime,delta);

}
