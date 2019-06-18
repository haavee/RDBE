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

#define COMMAND_LEN 200
#define DATA_SIZE 1024


//=====rdbe communication ======================================================================
//==rest of code should be using this, but currently they mostly have communication built in====
void rdbe_talk(dbe,to_send,got_back,itime,ierr)
char *dbe;
char *to_send;
char *got_back;
int itime;
int ierr;
{
    char rdbe_command[COMMAND_LEN];
    char data[DATA_SIZE];  
    char   dbe_ip [30];
    char me [] ="rdbe";
    char *ptok;
    int itimeout;
    char output[200];
    int i,idummy,len;
    unsigned int         dbe_fd;        // Server socket descriptor
    struct sockaddr_in   server_addr;     // Server Internet address
    char                 out_buf[200];    // output buffer for data
    fd_set in_fdset;
    struct itimerval value;
    int retval;
    static int fd_width;	/* width of file descriptors being used */
//===================================
//===================================
    ierr=0;
    strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0 default
    if((strncmp(dbe,"dbe0",4)==0)||(strncmp(dbe,"0",1)==0))strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0
    if((strncmp(dbe,"dbe1",4)==0)||(strncmp(dbe,"1",1)==0))strcpy(dbe_ip,stm_addr->dbe1_add);  //dbe 1
    if(itime !=0){itimeout=itime;} else {itimeout=2;} 
//===================================
    strcpy(out_buf,to_send);   //dbe command
    strcat(out_buf,";\n");    //add ";nl" stripped off by FS
//printf("DBT %s %s\n",dbe_ip,to_send);
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
    len=strlen(data);
    ptok=strtok(data,";");
    strcpy (got_back,ptok);
//printf("DBT1 %s %d %d\n",got_back,len,ierr);
    return;
error:
//    printf("DBERR %d\n",ierr);
    close(dbe_fd);
    strcpy(got_back," ");
    return;
}
