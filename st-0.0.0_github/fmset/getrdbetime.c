/* getrdbetime.c - get rdbe formatter time */

#include <ncurses.h>      /* ETI curses standard I/O header file */
#include <memory.h>      /* for memcpy */
#include <sys/types.h>   /* data type definition header file */
#include <stdlib.h>

#include "/usr2/fs/include/params.h"

#include "fmset.h"

extern long ip[5];           /* parameters for fs communications */
extern int synch;
int rack, rack_type;

void rte2secs();

void getrdbetime(unixtime,unixhs,fstime,fshs,formtime,formhs,raw,
m5sync,sz_m5sync,m5pps,sz_m5pps,m5freq,sz_m5freq,m5clock,sz_m5clock)
time_t *unixtime; /* computer time */
int    *unixhs;
time_t *fstime; /* field system time */
int    *fshs;
time_t *formtime; /* formatter time received from rdbe */
int    *formhs;
long *raw;
char *m5sync;
int sz_m5sync;
char *m5pps;
int sz_m5pps;
char *m5freq;
int sz_m5freq;
char *m5clock;
int sz_m5clock;
{
	long centisec[6], centiavg, centidiff, hsdiff;
        int it[6];
	char *name;
	char *str;
	int out_recs;
	long out_class;
	if(synch) {
	  out_recs=0;
	  out_class=0;

//	  str="tvr = 0;\n";
//	  cls_snd(&out_class, str, strlen(str) , 0, 0);
//	  out_recs++;
//
//	  if((rack == VLBA4 && rack_type == VLBA45) ||
//	     (rack == MK4   && rack_type == MK45  )) {

//	    str="1pps_source = vsi;\n";
//	    cls_snd(&out_class, str, strlen(str) , 0, 0);
//	    out_recs++;
	    
//	    str="clock_set = 32 : ext;\n";
//	    cls_snd(&out_class, str, strlen(str) , 0, 0);
//	    out_recs++;
//	    logit("Mark 5B 1pps_source and clock_set commands sent.",0,NULL);
//	  }
//	  str="dot_set= : force;\n";
//	  cls_snd(&out_class, str, strlen(str) , 0, 0);
//	  out_recs++;
//	     
//	  logit("Mark 5B re-synch command sent.",0,NULL);
//	  ip[0]=1;
//	  ip[1]=out_class;
//	  ip[2]=out_recs;
//	  
//	  nsem_take("fsctl",0);
//	  name="mk5cn";
//	  while(skd_run_to(name,'w',ip,120)==1) {
//	    if (nsem_test("fs   ") != 1) {
//	      endwin();
//	      fprintf(stderr,"Field System not running - fmset aborting\n");
//	      exit(0);
//	    }
//	    name=NULL;
//	  }
//	  skd_par(ip);
//	  nsem_put("fsctl");
//	  if(ip[2] != 0)
//	    {
//	      endwin();
//	      fprintf(stderr,"Error %d from formatter\n",ip[2]);
//	      logita(NULL,ip[2],ip+3,ip+4);
//	      rte_sleep(SLEEP_TIME);
//	      exit(0);
//	    }
//	  cls_clr(ip[0]);
	  synch=0;
	}


        nsem_take("fsctl",0);
        if(get_rdbetime(centisec,it,ip,1,m5sync,sz_m5sync,m5pps,sz_m5pps,
		      m5freq,sz_m5freq,m5clock,sz_m5clock)!=0) {
//          if(get_rdbe_issync(ip,m5sync,sz_m5sync)!=0) {
//	  endwin();
	  fprintf(stderr,"Field System not running - fmset aborting\n");
	  exit(0);
	}
        nsem_put("fsctl");
	if( ip[2] != 0 )
		{
		endwin();
		fprintf(stderr,"Error %d from formatter\n",ip[2]);
                logita(NULL,ip[2],ip+3,ip+4);
                rte_sleep(SLEEP_TIME);
		exit(0);
		}

	/* time before is more accurate */
	
//	centisec[1]=centisec[0];
//	centisec[3]=centisec[2];
//	centisec[5]=centisec[4];

//        centidiff =centisec[1]-centisec[0];
//        centiavg= centisec[0]+centidiff/2;
//	*raw=centiavg;

        rte_rawt(&centiavg);  /* retrieve the raw time XXX*/

        rte_fixt(fstime,&centiavg);
        *fshs=centiavg;

	hsdiff=(centisec[3]-centisec[2])*100+centisec[5]-centisec[4];
	*unixhs=centisec[4]+(hsdiff/2)/100;
	*unixtime=centisec[2];
	if(*unixhs>=100) {
	  *unixtime+=*unixhs/100;
	  *unixhs=*unixhs%100;
	}

        rte2secs(it,formtime);
        *formhs=it[0];
}
