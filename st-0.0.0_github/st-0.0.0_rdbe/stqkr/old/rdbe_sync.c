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

void rdbe_sync(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
// try up to 3 times to sync DBE
{
    char data[180];  
    char *ptok;
    char output[200];
    char dbe_name[10],dbe_command[200],dbe_reply[200];
    char *pttosync;
    char sync_state[20];
    int i,ierr,ntry,synced;
    strcpy(dbe_command,"dbe_dot_set=");
    strcpy(dbe_name,"dbe0");
    if(command->argv[0] != NULL)strcpy(dbe_name,command->argv[0]);
//printf("DB %s %s \n",dbe_name,dbe_command);
    ntry=1;           //try to sync up to 3 times
    synced=0;
    while(ntry <3){
      ierr=0;
      rdbe_talk(dbe_name,dbe_command,dbe_reply,3,ierr); 
      if(ierr <0)goto error;
      sleep(1);
      rdbe_talk(dbe_name,"dbe_dot?",dbe_reply,3,ierr); 
      if(ierr <0)goto error;
      pttosync=strstr(dbe_reply,"syncerr_eq_0");
      if (pttosync != NULL){
        synced=1; 
        strncpy(sync_state,pttosync,12);
        sync_state[12]='\0';
        break;
      }
      ntry++;
    }
    strcpy(output,"rdbe_sync");
    strcat(output,"/");
    strncat(output,dbe_name,4);
    sprintf(data,",sync_state=%s,tries=%d",sync_state,ntry);
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
