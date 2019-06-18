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

void rdbe_fchk(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
// in DDC mode, read cfg and xbar settings, 
//compare with values from setup, if NOK set flag to call setup again
// input is  dbe0/1 and number of channels to compare, e.g. 2 for first 2 chans 
//   3rd parameter is setup command (e.g. setup01) to call if check fails
// string from dbe enquiry:
//0:1:64.00:1:2014014135938:1:64.00:1:2014014135938:1:64.00:0:2014014135938:1:64.00:0:2014014135938:.. (8 entries for each dbe)
// [                      ] [                     ] [                     ] [                     ] [      ]
{
    char data[180];  
    char *pin;
    char *ptok,*ptr;
    char output[300];
    char dbe_name[30],dbe_command[200],cfg_reply[400],xbar_reply[200],buf1[300];
    char junk[100],part_0[100],part_1[100], part_2[100],part_3[100],part_4[100];
    char instring[100],setupcmd[100];
    char freq_state[30];
    int i,ierr,differ,nbbc,noffset,kk;
    float ff,ffwant;
    strcpy(dbe_name,"dbe0"); nbbc=4; noffset=0; strcpy(setupcmd," "); //default
    if(command->argv[0] != NULL)strcpy(dbe_name,command->argv[0]);
    if(command->argv[1] != NULL)strcpy(instring,command->argv[1]);
    if(command->argv[2] != NULL)strcpy(setupcmd,command->argv[2]);
    if((strncmp(dbe_name,"0",1)==0) || (strncmp(dbe_name,"dbe0",4)==0))noffset=0;  //start of fq data in common
    if((strncmp(dbe_name,"1",1)==0) || (strncmp(dbe_name,"dbe1",4)==0))noffset=4; 
    sscanf(instring,"%d",&nbbc);
printf("DBFCHK %s nbbc=%d noff=%d %s\n",dbe_name,nbbc,noffset ,setupcmd);
    ierr=0;
//================================temporary check section==================================
    for (i=0;i<nbbc;i++){
      kk=i+noffset;
      printf("CHKTMP %d %d %d %d %f %d\n",i,noffset,kk,stm_addr->dcfgdeci[kk],stm_addr->dcfgfq[kk],stm_addr->dcfgmode[kk]);
    }
//=========================================================================================
    rdbe_talk(dbe_name,"dbe_dc_cfg?",cfg_reply,3,ierr); 
    printf("DBCFGREPLY %s >%s< %d\n",dbe_name,cfg_reply,ierr);
    if(ierr <0)goto error;
    ptr=index(cfg_reply,'=');
    if(ptr!=NULL) {
      strcpy(buf1,ptr+1);
      printf("%s\n",buf1);
      ptok=strtok(buf1,":"); strcpy(junk,ptok); //first zero
      differ=0;
      for (i=0;i<nbbc;i++){
//  following numbers something like 1 64.0 1 2014343000000   Now compare with wanted values
        ptok=strtok(NULL,":");strcpy(part_0,ptok); sscanf(part_0,"%d",&kk); if(kk != stm_addr->dcfgdeci[i+noffset])differ=1;
        ptok=strtok(NULL,":");strcpy(part_1,ptok); sscanf(part_1,"%f",&ff); ffwant= stm_addr->dcfgfq[i+noffset];
        if((ff > (ffwant+0.1)) || (ff < (ffwant -0.1)))differ=1;
        ptok=strtok(NULL,":");strcpy(part_3,ptok); //time stamp ignore
        printf("CHKRESULT %s %s %s : %d %f :%d\n",part_0,part_1, part_2,stm_addr->dcfgdeci[i+noffset],stm_addr->dcfgfq[i+noffset],differ);
      }
    }
    rdbe_talk(dbe_name,"dbe_xbar?",xbar_reply,3,ierr); //read out present xbar setting 
//    printf(">%s<\n",xbar_reply);
    ptr=index(xbar_reply,'?');
    if(ptr!=NULL) {
       strcpy(buf1,ptr+1);
//       printf("%s\n",buf1);
       ptok=strtok(buf1,":");
       strcpy(junk,ptok); //first zero
       for (i=0;i<4;i++){
         ptok=strtok(NULL,":"); strcpy(part_0,ptok); //0:1:1:1:1:1:1:1:1
         sscanf(part_0,"%d",&kk); if(kk != stm_addr->dxbar[i+noffset])differ=1;
//         printf("%s %d %d\n",part_0,stm_addr->dxbar[i+noffset],differ);
       }
    }
    if(differ == 0){
      strcpy(freq_state,"check OK");//
    } else {
//      sprintf(freq_state,"check fail, call setup");//
      sprintf(freq_state,"check fail, call >%s<",setupcmd);//
      scmds(setupcmd);  //call setup command
    }
    strcpy(output,"rdbe_fchk");
    strcat(output,"/");
    strncat(output,dbe_name,4);
    sprintf(data,",rdbe_fchk=%s",freq_state);
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
