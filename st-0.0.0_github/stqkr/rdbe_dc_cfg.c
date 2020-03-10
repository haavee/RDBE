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
//#define PFB "pfbg_1_4_2.bin"
//#define DDC "ddc_1501383.bin:loaded"
extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200

void rdbe_dc_cfg(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
// to implement rdbe_dc_cfg with time stamp 5 seconds after current fs time
// if this is wrong the bastard will never change freq
// typical input command  dbe0,0:4:111.75:1
//                             0 1 2      3
//  with parameters:
//  0   down converter index "<DC>"
//  1   down converter decimation factor "<rate>"
//  2   down converter frequency "<LO freq>"
//  3   "dcfgmode" value - only used in stcom struct as far as I've been
//      able to find
//
// however, RDBE command is:
//         dbe_dc_cfg=<DC>:<rate>:<LO freq>:<time>
//
//  Until now (Mar 2020) the code in this function would blindly
//  append the FS time to whatever was passed in, i.e. making the command
//  being sent to the RDBE look like this:
//          dbe_dc_cfg=<DC>:<rate>:<LO freq>:[01]:<time>
//
//  Update Mar 2020:
//      fixed code to accept "rdbe_dc_cfg=dbeX,..." with "..." having three
//      or four parameters. Fourth parameter will be processed first and
//      then removed before sending command to RDBE
{
    
    int ita[6]; /*for rte_time */
    char timestamp[16];
    char data[180];  
    char *ptok;
    char output[200];
    char dbe_name[10],dbe_command[200],dbe_reply[200];
    char input_parm[200];
    char *ptt;
    int i,j,k,ierr,nbbc;
    int npos,noff;
    char tt, term_char;
    char part_0[100];
    strcpy(dbe_command,"dbe_dc_cfg=");
    strcpy(dbe_name,"dbe0");
    rte_time(ita,ita+5);     /*FS time*/
//add 5 sec to FS time
    ita[1]=ita[1]+5; if(ita[1] >=60){ita[1]=ita[1]-60; ita[2]=ita[2]+1;}
    if(ita[2] >=60){ita[2]=ita[2]-60; ita[3]=ita[3]+1;}
    if(ita[3] >=24){ita[3]=ita[3]-24; ita[4]=ita[4]+1;} 
//overflow of 1 year is unlikely....
    sprintf(timestamp,":%d%03d%02d%02d%02d",ita[5],ita[4],ita[3],ita[2],ita[1]);
    if(command->argv[0] != NULL)strcpy(dbe_name,command->argv[0]);
    if(command->argv[1] != NULL){
       strcpy(input_parm,command->argv[1]);
//-----------save input parameters in common------------
// HV Mar 2020: do this *before* appending the time stamp
//              to the command so we can support with/without
//              dcfgmode parameter
       noff=0; //default
       if(strncmp(dbe_name,"0",1) == 0){noff=0;}
       if(strncmp(dbe_name,"1",1) == 0){noff=4;}
       if(strncmp(dbe_name,"dbe0",4) == 0){noff=0;}
       if(strncmp(dbe_name,"dbe1",4) == 0){noff=4;}
       ptok=strtok(input_parm,":"); strcpy(part_0,ptok); sscanf(part_0,"%d",&nbbc);//nbbc is 0-3 for chans 1-4
       npos=nbbc+noff;  //so we get a number 0-7 to number ddc channels 1-8
       ptok=strtok(NULL,":"); strcpy(part_0,ptok); sscanf(part_0,"%d",&stm_addr->dcfgdeci[npos]);
       ptok=strtok(NULL,":"); strcpy(part_0,ptok); sscanf(part_0,"%f",&stm_addr->dcfgfq[npos]);
       // HV Mar 2020: only process last parameter if one seems to be
       //              present
       // HV Mar 2020: also need to decide how many characters from
       //              command->argv[1] to send to the RDBE.
       //              Reuse the "pch" value from this section:
       //                 if NULL this means no fourth parameter was present
       //                 so we can copy the whole input string
       //                 if not NULL we must find the last ":" in 
       //                 command->argv[1] and only copy characters until
       //                 there
       ptok=strtok(NULL,":");
       // Start off by accepting the whole string - i.e. only strip the
       // terminating ';'
       term_char = ';';
       if( ptok!=NULL ) {
           // Decode the value
           strcpy(part_0,ptok); sscanf(part_0,"%d",&stm_addr->dcfgmode[npos]);
           // Now we need to strip the last ':' and everything following it
           term_char = ':';
       } else {
           // No fourth parameter in CFG (syntax is "rdbe_dc_cfg=<DBE>,<CFG>")
           stm_addr->dcfgmode[npos] = -1;
       }
       // Now append the correct chunk of CFG to the actual dbe_command that
       // we're building. Take care of potentially not finding the
       // terminating character in which case the whole string will be
       // copied
       ptt = strrchr(command->argv[1], term_char);
       j   = (ptt == NULL ? strlen(command->argv[1]) : (int)(ptt - command->argv[1]));
       strncat(dbe_command, command->argv[1], j);
       // Now append the timestamp
       strcat(dbe_command, timestamp);
//       printf("DBGCFG %d %d %d : %d %f %d\n",nbbc,noff,npos, stm_addr->dcfgdeci[npos],stm_addr->dcfgfq[npos],stm_addr->dcfgmode[npos]);
//------------------------------------------------------
    }
    ierr=0;
    rdbe_talk(dbe_name,dbe_command,dbe_reply,3,ierr); 
    if(ierr <0)goto error;
    strcpy(output,"rdbe_dc_cfg");
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
