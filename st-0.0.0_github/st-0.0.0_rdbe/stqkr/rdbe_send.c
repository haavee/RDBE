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

void rdbe_send(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
// if command is "off", send dbe_data_send=off immediately, will stop on next second.
// if command is "on", seems to need a time stamp mark start as 5 seconds in future.
// Since dc_cfg command was 1 second ago, and marked as 5sec later, we should
// have 1 sec spare to carry out this command.
{
    
    int ita[6]; /*for rte_time */
    char timestamp[16],timestop[16];
    char data[180];  
    char *ptok;
    char output[200];
    char dbe_name[10],dbe_command[200],dbe_reply[200];
    char *ptt;
    int i,j,k,ierr;
    char tt;
    strcpy(dbe_command,"dbe_data_send=");
    strcpy(dbe_name,"dbe0");
    rte_time(ita,ita+5);     /*FS time*/
//add 5 sec to FS time
    ita[1]=ita[1]+5; if(ita[1] >=60){ita[1]=ita[1]-60; ita[2]=ita[2]+1;}
    if(ita[2] >=60){ita[2]=ita[2]-60; ita[3]=ita[3]+1;}
    if(ita[3] >=24){ita[3]=ita[3]-24; ita[4]=ita[4]+1;} 
//overflow of 1 year is unlikely....
    sprintf(timestamp,":%d%03d%02d%02d%02d",ita[5],ita[4],ita[3],ita[2],ita[1]);
    sprintf(timestop,":%d%03d235959",ita[5],ita[4]+1);//next data stop midnight
    if(command->argv[0] != NULL)strcpy(dbe_name,command->argv[0]);
    if(command->argv[1] != NULL){
       j=strlen(command->argv[1]);
       if(command->argv[1][j-1] == ';')j=j-1; //remove possible ';' at end
       strncat(dbe_command,command->argv[1],j);
       if(strncmp(command->argv[1],"on",2)== 0){
         strcat(dbe_command,timestamp);
         strcat(dbe_command,timestop);
       } else {
         strcat(dbe_command,"::");
       }
    }
    ierr=0;
    rdbe_talk(dbe_name,dbe_command,dbe_reply,3,ierr); 
    if(ierr <0)goto error;
    strcpy(output,"rdbe_send");
    strcat(output,"/");
    strncat(output,dbe_name,4);
//    sprintf(data,",time_stamp=%s:",timestamp);
    strcat (output,"("); strcat (output,dbe_command); strcat (output,")");
    strcpy(data,dbe_reply);
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
