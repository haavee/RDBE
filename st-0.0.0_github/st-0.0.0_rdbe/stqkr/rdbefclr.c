#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <sys/types.h>    // Needed for system defined identifiers.
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"

extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200

// clear list of rdbe channels

void rdbefclr(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
//input is zB    rdbefclr=dbe0
{
    char data[180];  
    char output[200];
    int i,ierr;
    int ib,ibm1; //number 1-16 of our channel, also 0-15 same
    int dbenum; //0 or 1, at moment only use 0

//at moment assume dbe0, don't look at dbe number
    if(strncmp(command->argv[0],"dbe0",4) == 0){ 
      dbenum=0;
//goto getmeout;
      for(ibm1=0;ibm1<16;ibm1++){
        stm_addr->rdbe0_tk[ibm1]=-2;
        stm_addr->rdbe0_if[ibm1]=-2;
        stm_addr->rdbe0_chanfreq[ibm1]=-2; //clear for ddc
      }
      strcpy(stm_addr->rdbe0_if0_iflet,"x"); //mark dbe0 ifo,if1 as unconnected 
      strcpy(stm_addr->rdbe0_if1_iflet,"x"); 
    }
    else { 
      dbenum=1; 
      for(ibm1=0;ibm1<16;ibm1++){
        stm_addr->rdbe1_tk[ibm1]=-2;
        stm_addr->rdbe1_if[ibm1]=-2;
        stm_addr->rdbe1_chanfreq[ibm1]=-2; //clear for ddc
      }
      strcpy(stm_addr->rdbe1_if0_iflet,"x"); //mark dbe1 ifo,if1 as unconnected 
      strcpy(stm_addr->rdbe1_if1_iflet,"x"); 
    }
//getmeout:
//    printf("RDBF_CLR %s\n",command->argv[0]);
    sprintf(data, "DBFQ:chan_cleared");
    strcpy(output,"rdbefclr");
    strcat(output,"/");
    strcat (output,data);
    for (i=0;i<5;i++)ip[i]=0;
    cls_snd(&ip[0],output,strlen(output),0,0);
    ip[1]=1;
    return;
error:
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"rd",2);
    return;
}
//----------------------------------
//  int  rdbe0_if[32]; //only need 16 really...
//  int  rdbe0_tk[32];
//  float rdbe0_chanfreq [32]; //read in if, tkno, chan freq, poln 
//  char rdbe0_pol[32];
//  float rdbe0_tcal[32];
//----------------
//  double rdbe0_if0_caloff[32]; //Tsys readout from RDBE
//  double rdbe0_if0_calon[32];
//  double rdbe0_if1_caloff[32];
//  double rdbe0_if1_calon[32];
