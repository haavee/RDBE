#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fscom.h"
#include "rdbe_addr.h"

extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200
#define DATA_SIZE 512
 
void p_rdbe_adc(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
    FILE *pf;
    char rdbe_command[COMMAND_LEN];
    char data[DATA_SIZE];
    char deb_num [10], timout [10] , dbe_ip [30];
    char output[200];
    int i,ierr,idummy;
    if(strncmp(command->argv [0],"1",1) == '0'){
        strcpy(dbe_ip,RDBE1);
    } else {
        strcpy(dbe_ip,RDBE0);
    }
    strcpy(timout,command->argv [1]);
    sprintf(rdbe_command, "/usr2/st/arecibo/rdbe_read_adc.py -i %s -t %s %s",dbe_ip,timout,command->argv [2]); 
    printf("DEB_command_sent=%s\n",rdbe_command);
    pf = popen(rdbe_command,"r"); 
 
    if(!pf){
      printf( "Could not open pipe for output.\n");
      return;
    }
    fgets(data, DATA_SIZE , pf);
 
    printf("%s",data); 
    if (pclose(pf) != 0)
        printf(" Error: Failed to close command stream \n");
   strcpy(output,"rdbe_adc");
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
    memcpy(ip+3,"st",2);
    return;
}
