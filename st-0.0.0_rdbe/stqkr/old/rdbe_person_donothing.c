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
#define PFB "pfbg_1_4_2.bin"
#define DDC "ddc_1501383.bin:loaded"
extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200

//2013.004.15:07:07.83/rdbe_cmd/!dbe_personality?0:PFBG:ddc_1501383.bin:loaded
//2013.004.15:07:15.55/rdbe_cmd/!dbe_personality?0:PFBG:pfbg_1_4_2.bin:loaded
//2013.008.13:52:39.78/rdbe_cmd/!dbe_personality?0:DDC:ddc_1501383.bin:loaded
//rdbe_cmd=0,30,dbe_personality=pfbg:roachvlbi2010_oct13.bin
//rdbe_cmd=0,30,dbe_personality=:pfbg_1_4_2.bin
//rdbe_cmd=0,10,dbe_execute=init;

void rdbe_person(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
// accepts command for example dbe0,ddc or dbe0,pfb
//checks which personality is loaded and changes
// if necessary
{
    char data[180];  
    char *ptok;
    char output[200],reply[200];
    char pers_wanted[20],pers_loaded[20],fpga_file[100];
    char dbe_name[10],dbe_command[200],dbe_reply[200];
    char *ptt,*tmpch;
    int i,j,k,ierr;
    char tt;
    strcpy(pers_loaded," ");
    strcpy(fpga_file," ");
    strcpy(dbe_command,"dbe_personality?");
    strcpy(dbe_name,"dbe0");
    if(command->argv[0] != NULL)strcpy(dbe_name,command->argv[0]);
    if(command->argv[1] != NULL)strcpy(pers_wanted,command->argv[1]);
//printf("DB %s %s \n",dbe_name,dbe_command);
    ierr=0;
    rdbe_talk(dbe_name,dbe_command,dbe_reply,3,ierr); 
    if(ierr <0)goto error;
    strcpy(reply,dbe_reply);
//-----0:DDC:ddc_1501383.bin:loaded -----------------------------------
    ptt=(char *)strtok(dbe_reply,":");
    if (ptt != NULL){
       tmpch=(char *)strtok(0,":");
       if (tmpch != NULL){
          while (tmpch[i] != '\0')
          {
            pers_loaded[i]= tolower(tmpch[i]);
            i++;
          }
       }
       pers_loaded[3]='\0';
       if (tmpch != NULL){
         tmpch=(char *)strtok(0,":");
         strcpy(fpga_file,tmpch);
       }
    }
//printf ("DB>%s<>%s<>%s<\n",pers_wanted,pers_loaded,fpga_file);
    strcpy(output,"rdbe_personality");
    strcat(output,"/");
    strncat(output,dbe_name,4);
    sprintf(data,",type=%s,fpga=%s:",pers_loaded,fpga_file);
    strcat(data,dbe_reply);
    strcat (output,data);
    if(strncmp(pers_loaded,pers_wanted,3) !=0)strcat(output,":NOTE:WRONG PERSONALITY LOADED");
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
