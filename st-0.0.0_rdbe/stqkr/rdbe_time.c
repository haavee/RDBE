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
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"
//#include "rdbe_addr.h"

//----- Defines ---------------------------------------------------------------
#define  PORT_DBE         5000     // Port number used at the server
#define  BUFFER_SIZE       1024
//-----------------MULTICAST  STUFF-rdbe as 0 and 1 (not 1 and 2..)-------
#define MULTI_PORT 20020
#define MULTI_GROUP_0 "239.0.2.34"
#define MULTI_GROUP_1 "239.0.2.35"
#define MSGBUFSIZE 256
//------------------------------------------
extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200
#define DATA_SIZE 512
//===== Main program ==========================================================


void rdbe_time(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
    char rdbe_command[COMMAND_LEN];
    char data[DATA_SIZE];  
    char  timout [10] , dbe_ip [30];
    char me [] ="rdbe";
    char *ptok;
    int fmtime[6];
    int itimeout;
    char output[200];
    int i,j,ierr,idummy;
    unsigned int         dbe_fd;        // Server socket descriptor
    unsigned int         multi_fd;        // for timecast
    int multi_addrlen,nbytes;
    struct sockaddr_in   server_addr;     // Server Internet address
    struct sockaddr_in   multi_addr;     // timecast
    u_int yes=1;
    struct ip_mreq mreq;
    char                 out_buf[200];    // output buffer for data
    unsigned char       in_buf[1024];     // input buffer for data
    unsigned char        multi_msgbuf[1024]; //multicast in here
    unsigned char        compare_buf[1024]; //compose time comparison here for output
    fd_set in_fdset;
    fd_set multi_fdset;
    struct itimerval value;
    struct itimerval multi_value;
    int retval;
    char shms[30]; //home system time XXX
    char rdb_hms[20];
    static int fd_width;	/* width of file descriptors being used */
    char part_0[200],part_1[200], part_2[200],part_3[200],part_4[200];
    int dbe_val[512];
    int dbe_num; //to look for dbe0 or 1
    time_t tim; //XXX to get system time, hopoefully ntp..
    strcpy(dbe_ip,stm_addr->dbe0_add); itimeout=2; dbe_num=0;//dbe 0
    if(command->argv [0] !=NULL ){
      if(strncmp(command->argv [0],"0",1) == 0){strcpy(dbe_ip,stm_addr->dbe0_add);dbe_num=0;}  //dbe 0
      if(strncmp(command->argv [0],"1",1) == 0){strcpy(dbe_ip,stm_addr->dbe1_add);dbe_num=1;}  //dbe 1
      if(strncmp(command->argv [0],"dbe0",4) == 0){strcpy(dbe_ip,stm_addr->dbe0_add);dbe_num=0;}  //dbe 0
      if(strncmp(command->argv [0],"dbe1",4) == 0){strcpy(dbe_ip,stm_addr->dbe1_add);dbe_num=1;}  //dbe 0
      if(command->argv [1] !=NULL ){
         strcpy(timout,command->argv [1]);
         sscanf(timout,"%d",&itimeout);
      } else {
         itimeout=2; 
      }
    } 
//===================================
    tim=time(NULL);
    char *s=ctime(&tim) ;  //get system time for comparison
    strncpy(shms,&s[strlen(s)-14],8);
    shms[8]='\0';
//===================================
    strcpy(out_buf,"dbe_dot?;\n");    //
    dbe_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family      = AF_INET;            // Address family to use
    server_addr.sin_port        = htons(PORT_DBE);    // Port num to use
    server_addr.sin_addr.s_addr = inet_addr(dbe_ip); // IP address to use
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
        recv(dbe_fd, in_buf, sizeof(in_buf), 0);
      }
    } 
    close(dbe_fd);
//-----------------------------------
    //!dbe_dot?0:2012027170013:syncerr_eq_0:2012027170013:0:95361213
      ptok=strtok(in_buf,":"); strcpy(part_0,ptok);
      ptok=strtok(NULL,":"); strcpy(part_0,ptok); //2012027170013
      ptok=strtok(NULL,":"); strcpy(part_1,ptok); //syncerr_eq_0
      ptok=strtok(NULL,":"); strcpy(part_2,ptok); //2012027170013
      ptok=strtok(NULL,":"); strcpy(part_3,ptok); //0
      ptok=strtok(NULL,":"); strcpy(part_4,ptok); //95361213
      sscanf(part_0,"%04d%03d%02d%02d%02d",&fmtime[0],&fmtime[1],&fmtime[2],
                                           &fmtime[3],&fmtime[4]);
//      printf("TIMEREAD %d %d %d %d %d \n",fmtime[0],fmtime[1],fmtime[2],fmtime[3],fmtime[4]);
        sprintf(compare_buf,"ntp=%s, rdbe=%02d%02d%02d  ",shms,fmtime[2],fmtime[3],fmtime[4]);
//        printf("C1%s\n",compare_buf);
//-------------multi-----if multi not running, exit...
      if ((multi_fd=socket(AF_INET,SOCK_DGRAM,0)) >= 0) {
        if (setsockopt(multi_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) >=0) {
          memset(&multi_addr,0,sizeof(multi_addr));
          multi_addr.sin_family=AF_INET;
          multi_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
          multi_addr.sin_port=htons(MULTI_PORT);
          FD_ZERO(&multi_fdset); FD_SET(multi_fd,&multi_fdset);
          multi_value.it_value.tv_usec = 0; multi_value.it_value.tv_sec = itimeout; //looking for multicast 1pps should timeout after 2 secs..
          if (bind(multi_fd,(struct sockaddr *) &multi_addr,sizeof(multi_addr)) >= 0) {
            if(dbe_num == 0)mreq.imr_multiaddr.s_addr=inet_addr(MULTI_GROUP_0);
            if(dbe_num == 1)mreq.imr_multiaddr.s_addr=inet_addr(MULTI_GROUP_1);
            mreq.imr_interface.s_addr=htonl(INADDR_ANY);
            if(setsockopt(multi_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) >=0){
              fd_width=multi_fd+1; retval = select(fd_width,&multi_fdset,NULL,NULL,&multi_value.it_value);
              if (retval == -1) {ierr=-541; goto error;} //select failed
              else if (retval ==0 ){ ierr=-544; goto so_what;} //timeout, just take what we got from poll
              else {                              //data on descriptor from port 5000
                 if(FD_ISSET(multi_fd,&multi_fdset)){
                   nbytes=recvfrom(multi_fd,multi_msgbuf,MSGBUFSIZE,0,(struct sockaddr *) &multi_addr,&multi_addrlen) ;
                   usleep(10000); //just came off socket wait, so wait for system to catch up
	           time(&tim);
                   tim=time(NULL);
                   char *s=ctime(&tim);   //get system time for comparison
                   strncpy(shms,&s[strlen(s)-14],8);
                   multi_msgbuf[13]='\0'; //could be crud here
                   sprintf(compare_buf,"ntp=%s, rdbe=%s  ",shms,multi_msgbuf);
//                   printf("C2%s\n",compare_buf);
              }
//result:2012181132920
              }
            }
          }
        }
      }
so_what:
    close(multi_fd);
//-----------------------------------
//      printf("TIMEREAD %d %d %d %d %d \n",fmtime[0],fmtime[1],fmtime[2],fmtime[3],fmtime[4]);
//                 printf("%s\n",multi_msgbuf); 
//-----------------------------------
    strcpy(output,"rdbe_time");
    strcat(output,"/");
    strcat (output,compare_buf);
    strcat (output,part_1); //add in sync message
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
