#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"
//#include "rdbe_addr.h"

extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200
#define DATA_SIZE 512
 
void p_rdbe_alc(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
    FILE *pf;
    char rdbe_command[COMMAND_LEN];
    char data[DATA_SIZE];
    char  timout [10] , dbe_ip [30];
    char output[200];
    int i,ierr,idummy;

//    if(strncmp(command->argv [0],"0",1) == 0)strcpy(dbe_ip,RDBE0);  //dbe 0
//    if(strncmp(command->argv [0],"1",1) == 0)strcpy(dbe_ip,RDBE1);  //dbe 1
//    if(strncmp(command->argv [0],"dbe0",4) == 0)strcpy(dbe_ip,RDBE0);  //dbe 0
//    if(strncmp(command->argv [0],"dbe1",4) == 0)strcpy(dbe_ip,RDBE1);  //dbe 0
//===================================
    strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0 default
    if((strncmp(command->argv [0],"dbe0",4)==0)||(strncmp(command->argv [0],"0",1)==0))strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0
    if((strncmp(command->argv [0],"dbe1",4)==0)||(strncmp(command->argv [0],"1",1)==0))strcpy(dbe_ip,stm_addr->dbe1_add);  //dbe 1
//===================================

    strcpy(timout,command->argv [1]);
//    sprintf(rdbe_command, "/usr2/st/arecibo/rdbe_alc_adj.py -i %s -t %s %s",dbe_ip,timout,command->argv [2]); //This is python version
    sprintf(rdbe_command, "/usr2/st/bin/prog_rdbe_alc_adj  %s  %s %s",dbe_ip,timout,command->argv [2]); //Try local C version
//printf("DBG_ALC_COMMAND %s\n",rdbe_command);
    pf = popen(rdbe_command,"r"); 
 
    if(!pf){
      printf( "Could not open pipe for output.\n");
      return;
    }
 
    fgets(data, DATA_SIZE , pf);
 
//    printf("DBDATA=%s",data); 
    if (pclose(pf) != 0)
        printf(" Error: Failed to close command stream \n");
   strcpy(output,"rdbe_alc");
   strcat(output,"/");
   strcat (output,data);
//   for(i=0;i<strlen(output);i++){if(output[i]=='\n' || output[i]=='\r')output[i]='\0';}//strip any nl
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
