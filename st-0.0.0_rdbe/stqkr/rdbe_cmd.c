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

extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200
#define DATA_SIZE 512
//===== Main program ==========================================================


void rdbe_cmd(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
    char rdbe_command[COMMAND_LEN];
    char data[DATA_SIZE];  
    char  timout [10] , dbe_ip [30],dbe[20];
    char me [] ="rdbe";
    char *ptok;
    int itimeout;
    char output[200];
    int i,ierr,idummy,il,ipp;
    unsigned int         dbe_fd;        // Server socket descriptor
    struct sockaddr_in   server_addr;     // Server Internet address
    char                 out_buf[DATA_SIZE];    // output buffer for data
    fd_set in_fdset;
    struct itimerval value;
    int retval;
    static int fd_width;	/* width of file descriptors being used */
    int dbe_val[512];
//===================================
    itimeout=2; //default
    strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0 default
//    strcpy(dbe_ip,RDBE0);  //dbe 0
    if(command->argv [0] !=NULL ){
      strcpy(dbe,command->argv[0]);
      if((strncmp(dbe,"dbe0",4)==0)||(strncmp(dbe,"0",1)==0))strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0
      if((strncmp(dbe,"dbe1",4)==0)||(strncmp(dbe,"1",1)==0))strcpy(dbe_ip,stm_addr->dbe1_add);  //dbe 1
//      if(strncmp(command->argv [0],"0",1) == 0)strcpy(dbe_ip,RDBE0);  //dbe 0
//      if(strncmp(command->argv [0],"1",1) == 0)strcpy(dbe_ip,RDBE1);  //dbe 1
//      if(strncmp(command->argv [0],"dbe0",4) == 0)strcpy(dbe_ip,RDBE0);  //dbe 0
//      if(strncmp(command->argv [0],"dbe1",4) == 0)strcpy(dbe_ip,RDBE1);  //dbe 1
      if(command->argv [1] !=NULL ){
         strcpy(timout,command->argv [1]);
         sscanf(timout,"%d",&itimeout);
      } else {
         itimeout=2; 
      }
    } 
//===================================
    strcpy(out_buf,command->argv [2]);   //dbe command
//=========particular care with ioch_assign command needed, cut off extra :;=======
    if(strncmp(out_buf,"dbe_ioch_assign=",16) ==0){
      il=strlen(out_buf);
      for (ipp=(il-1); ipp>0; ipp--){
         if((out_buf[ipp] == ';') || (out_buf[ipp] == ':'))out_buf[ipp]='\0';
         if(isalpha( out_buf[ipp])) break;
         if(isdigit( out_buf[ipp])) break;
         if(out_buf[ipp] == '-') break;
      }
    }
//=================================================================================
    strcat(out_buf,";\n");    //add ";" stripped off by FS and nl
    dbe_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family      = AF_INET;            // Address family to use
    server_addr.sin_port        = htons(PORT_DBE);    // Port num to use
    server_addr.sin_addr.s_addr = inet_addr(dbe_ip); // IP address to use
//printf("DBCMD %s %d %s\n",dbe_ip,itimeout,out_buf);
    retval=connect(dbe_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (retval == -1) {ierr=-540; goto error;} //connect failed
    retval=send(dbe_fd, out_buf, (strlen(out_buf) + 1), 0);
    FD_ZERO(&in_fdset);
    FD_SET(dbe_fd,&in_fdset);
    value.it_value.tv_usec = 0;
    value.it_value.tv_sec = itimeout;
    fd_width=dbe_fd+1;
    retval = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);
    if (retval == -1) {ierr=-541; goto error;} //select failed
    else if (retval ==0 ){ ierr=-542; goto error;} //timeout
    else {                              //data on descriptor from port 5000
      if(FD_ISSET(dbe_fd,&in_fdset)){
        recv(dbe_fd, data, sizeof(data), 0);
      }
    } 
    close(dbe_fd);
    strcpy(output,"rdbe_cmd");
    strcat(output,"/");
    ptok=strtok(data,";");
    strcat (output,ptok);
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
