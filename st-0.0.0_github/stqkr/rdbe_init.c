#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <sys/types.h>    // Needed for system defined identifiers.
#include <netinet/in.h>   // Needed for internet address structure.
#include <sys/socket.h>   // Needed for socket(), bind(), etc...
#include <arpa/inet.h>    // Needed for inet_ntoa()
#include <fcntl.h>        // Needed for sockets stuff
#include <netdb.h>        // Needed for sockets stuff
#include <sys/select.h>   // extra stuff select & timeout
#include <time.h>
#include <sys/time.h>
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"
//#include "rdbe_addr.h"

//----- Defines ---------------------------------------------------------------
#define  PORT_DBE         5000     // Port number used at the server
#define  PORT_ADC         5050     // Port number used at the server
#define  BUFFER_SIZE       1024

//extern struct stcom *st;
//extern struct fscom *fs;


#define CONTROL_FILE "/usr2/control/rdbead.ctl"
//char dbe0_add[20],dbe1_add[20],t450_add[20],essr_add[20];
//=====rdbe communication ======================================================================
//==start with this to read addr of dbe0,1, t450 and essr unit to common====
//NBNB should multicast addr also be in here??????????????????????????????????????????????
void rdbe_init(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
//    char rdbe_command[COMMAND_LEN];
//    char data[DATA_SIZE];  
//    char  timout [10] , dbe_ip [30];
    char me [] ="rdbe";
//    char *ptok;
//    int itimeout;
    char output[200],data[200];
//    int i,idummy,len;
//    char                 out_buf[200];    // output buffer for data
//    fd_set in_fdset;
//    struct itimerval value;
//    int retval;
    int ierr,i;

    FILE *fp;
    char rd1[40],rd2[40];
    char dbe0_add[20],dbe1_add[20],t450_add[20];
//===================================
    ierr=0;
    if ( (fp = fopen(CONTROL_FILE,"r")) == NULL) {
        printf("cannot open RDBE address file %s\n",CONTROL_FILE);
        ierr=-539; goto error; 
    }
    // Empty adresses
    stm_addr->dbe0_add[0] = stm_addr->dbe1_add[0] = stm_addr->t450_add[0] = stm_addr->essr_add[0] = '\0';
    //
    while (!feof(fp)){
       fscanf(fp, "%s %s", &rd1,&rd2);
//       printf("%s %s\n", rd1,rd2);
       if(strncmp(rd1,"RDBE0",5)==0)strcpy(stm_addr->dbe0_add,rd2);
       if(strncmp(rd1,"RDBE1",5)==0)strcpy(stm_addr->dbe1_add,rd2);
       if(strncmp(rd1,"T450",4)==0)strcpy(stm_addr->t450_add,rd2);
       if(strncmp(rd1,"ESSR",4)==0)strcpy(stm_addr->essr_add,rd2);
    }
    sprintf(data,"%s#%s#%s#%s",stm_addr->dbe0_add,stm_addr->dbe1_add,stm_addr->t450_add,stm_addr->essr_add); 
    strcpy(output,"rdbe_addr/");
    strcat(output,data);
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
