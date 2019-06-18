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

//----- Defines ---------------------------------------------------------------
#define  BUFFER_SIZE       1024

extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 512
#define DATA_SIZE 512
#define PORT_ESSR 2620
//===== Main program ==========================================================

// modes:
//    1 - Duplicate one input stream to 2 output streams
//    2 - Operate a pair of straight thru connections
//    3 - Receive data on two ports and splice into one output
//    4 - Operate a single straight thru connection


void essr(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
    char out_buf[COMMAND_LEN];
    char data[DATA_SIZE];  
    char  timout [10] , essr_ip [30];
    char me [] ="essr";
    char *ptok;
    int itimeout;
    char output[200];
    int i,ierr,idummy,il,ipp;
    unsigned int         essr_fd;        // Server socket descriptor
    struct sockaddr_in   server_addr;     // Server Internet address
    fd_set in_fdset;
    struct itimerval value;
    int retval;
    static int fd_width;	/* width of file descriptors being used */
//===================================
    itimeout=2; //default
    strcpy(essr_ip,stm_addr->essr_add);
//==NB for certain commands may want to parse and check status of machine
    if(command->argv [0] !=NULL ){
      strcpy(out_buf,command->argv [0]);
    }
    if(command->argv [1] !=NULL ){
      strcat(out_buf,":");
      strcat(out_buf,command->argv [1]);
    }
    if(command->argv [2] !=NULL ){
      strcat(out_buf,":");
      strcat(out_buf,command->argv [2]);
    }
//=================================================================================
    strcat(out_buf,";\n");    //add ";" stripped off by FS and nl
    essr_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family      = AF_INET;            // Address family to use
    server_addr.sin_port        = htons(PORT_ESSR);    // Port num to use
    server_addr.sin_addr.s_addr = inet_addr(essr_ip); // IP address to use
//printf("DBCMD %s %d %s\n",essr_ip,itimeout,out_buf);
    retval=connect(essr_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (retval == -1) {ierr=-547; goto error;} //connect failed
    retval=send(essr_fd, out_buf, (strlen(out_buf) + 1), 0);
    FD_ZERO(&in_fdset);
    FD_SET(essr_fd,&in_fdset);
    value.it_value.tv_usec = 0;
    value.it_value.tv_sec = itimeout;
    fd_width=essr_fd+1;
    retval = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);
    if (retval == -1) {ierr=-548; goto error;} //select failed
    else if (retval ==0 ){ ierr=-549; goto error;} //timeout
    else {                              //data on descriptor from port 5000
      if(FD_ISSET(essr_fd,&in_fdset)){
        recv(essr_fd, data, sizeof(data), 0);
      }
    } 
    close(essr_fd);
    strcpy(output,"essr");
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
