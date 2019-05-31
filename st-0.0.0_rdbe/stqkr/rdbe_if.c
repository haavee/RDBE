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
//115 or 150??
//#define T450_IP "134.104.79.150"
// New local address
//#define T450_IP "169.254.254.100"
#define T450_PORT 7000

extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200

// copy input params to log, check for 4x4 switch and command if present
//without params, check 4x4 state

void rdbe_if(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
//input is zB    rdbe_if=loa,7600,usb,lcp,dbe0,if0
//                       arg0 1   2    3   4   5  
{
    char data[180];  
    char *ptok;
    char output[200];
    char t450_cmd [40];
    char t450_reply[4096]; //verbose machine, sends back lots of xml
    char t450_text[100];
    char mc[4];
    int loif0,loif1;
    char vlba_if[4],rdbe_if[4];
    int iif; //if number 0 or 1
    int idb; //db number 0 or 1
    int t450_in; //t450 IF number in 0 to 3 corresponding to abcd
    int t450_out; //t450 IF number out 0 to 3
    int timeout_secs=1;
    unsigned int     t450_fd;   // Server socket descriptors
    struct sockaddr_in   t450_addr;     // Server Internet address
    static int fd_width;	/* width of file descriptors being used */
    fd_set in_fdset;
    struct itimerval value;

    int i,ierr,iret;
    int j,k,l,m,l1,l2,l3,l4,l5,l6,len;
    char mname[12],mval[12];
    unsigned int swval;

//-----------------------------------------------------------------------
    idb=0; iif=0; //default
    strcpy(t450_reply,"");
    if (command->equal != '=') {             //if no equals, just readout state
        for(i=0;i<4;i++){mc[i]='X';} //set to "unknown" as switch position report
        strcpy(t450_cmd,"get T450.*\n");
    } else {
      if(strncmp(command->argv[4],"dbe0",4) == 0){ idb=0; } else { idb=1; }
      if(strncmp(command->argv[5],"if0",3) == 0){ iif=0; } else { iif=1; }
      strcpy(vlba_if,command->argv[0]);
      strcpy(rdbe_if,command->argv[5]);
//first store in common what vlba if to if0/if0 of dbe0/1, this is used by tsys
      if((idb == 0) && (iif == 0))strncpy(stm_addr->rdbe0_if0_iflet,&vlba_if[2],1);
      if((idb == 0) && (iif == 1))strncpy(stm_addr->rdbe0_if1_iflet,&vlba_if[2],1);
      if((idb == 1) && (iif == 0))strncpy(stm_addr->rdbe1_if0_iflet,&vlba_if[2],1);
      if((idb == 1) && (iif == 1))strncpy(stm_addr->rdbe1_if1_iflet,&vlba_if[2],1);
//abcd to 0123 commands of t450
      if(strncmp(vlba_if,"loa",3) == 0){ t450_in=0; }
      if(strncmp(vlba_if,"lob",3) == 0){ t450_in=1; }
      if(strncmp(vlba_if,"loc",3) == 0){ t450_in=2; }
      if(strncmp(vlba_if,"lod",3) == 0){ t450_in=3; }
      if(strncmp(command->argv[4],"dbe0",4) == 0){ idb=0; } else { idb=1; }
      if(strncmp(command->argv[5],"if0",3) == 0){ iif=0; } else { iif=1; }
      t450_out=idb*2+iif; // 0-3 for IF outputs 1-4
      sprintf(t450_cmd,"set T450.SW_%1d_CMD=\'%1d\'\nset T450.SW_%1d_MON=\'%1d\'\n",t450_out,t450_in,t450_out,t450_in);
    }
//    printf("T450_CMD=%s",t450_cmd);
//-----------------------------------------------------------------------
    t450_fd = socket(AF_INET, SOCK_DGRAM, 0);
    t450_addr.sin_family      = AF_INET;            // Address family to use
    t450_addr.sin_port        = htons(T450_PORT);    // Port num to use
    t450_addr.sin_addr.s_addr = inet_addr(stm_addr->t450_add); // IP address to use
    FD_ZERO(&in_fdset); FD_SET(t450_fd,&in_fdset); 
    iret=sendto(t450_fd, t450_cmd, (strlen(t450_cmd) + 1), 0,(struct sockaddr *)&t450_addr, sizeof(t450_addr));
    if(iret <0){strcpy(t450_text,"T450_NOT_PRESENT");goto noswitch;}
    value.it_value.tv_usec = 0;
    value.it_value.tv_sec = timeout_secs;
    fd_width=t450_fd+1;
    iret = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);
    if(iret <0){strcpy(t450_text,"T450_NOT_PRESENT");goto noswitch;}
    if(iret ==0){strcpy(t450_text,"T450_NOT_PRESENT");goto noswitch;}
    if(FD_ISSET(t450_fd,&in_fdset)){
       recvfrom(t450_fd, t450_reply, sizeof(t450_reply), 0,NULL,NULL);
       l=strlen(t450_reply);
       for(i=0;i<l;i++){
         if(strncmp(&t450_reply[i],"monitor name",11)==0){
//           j=strncmp("monitor name",&t450_reply[i],11);
//           if(j==0){
             k=i;
             l1=locate('\'',t450_reply,k);
             l2=locate('\'',t450_reply,(l1+1));
             l3=locate('\'',t450_reply,(l2+1));
             l4=locate('\'',t450_reply,(l3+1));
             l5=locate('\'',t450_reply,(l4+1));
             l6=locate('\'',t450_reply,(l5+1));
             len=l2-l1-1; strncpy(mname,&t450_reply[l1+1],len); mname[len]='\0';
             len=l6-l5-1; strncpy(mval,&t450_reply[l5+1],len); mval[len]='\0';
             if(strncmp(mname,"SW_",3) ==0){
               sscanf(mval,"%x",&swval);
               if(strcmp(mname,"SW_0_MON") == 0)m=0;
               if(strcmp(mname,"SW_1_MON") == 0)m=1;
               if(strcmp(mname,"SW_2_MON") == 0)m=2;
               if(strcmp(mname,"SW_3_MON") == 0)m=3;
               if(swval ==0)mc[m]='A';
               if(swval ==1)mc[m]='B';
               if(swval ==2)mc[m]='C';
               if(swval ==3)mc[m]='D';
               if(swval ==4)mc[m]='T';
//               printf("NAME,VAL=%s %d %d %c SWPS=%c:%c:%c:%c\n",mname,swval,m,mc[m],
//                           mc[0],mc[1],mc[2],mc[3]);
             }
//           }
         }
       }
    }   
//    sprintf(t450_text, "swpos=%c:%c:%c:%c",mc[0],mc[1],mc[2],mc[3]);
    noswitch:      //here if switch does not reply
//---------------------------
//        for(i=0;i<4;i++){mc[i]='X';} //set to "unknown" as switch position report
    sprintf(t450_text, "SWPOS=%c:%c:%c:%c",mc[0],mc[1],mc[2],mc[3]);
//---------------------------
//  printf("T450says:%s:\n",t450_reply);
//-----------------------------------------------------------------------
    close(t450_fd);
    if (command->equal != '=') {             //if no equals, just readout state
      strcpy(data,t450_text);
    } else {
      sprintf(data,"%s,%s,%s,%s,%s,%s",command->argv[0],command->argv[1],command->argv[2],command->argv[3],command->argv[4],command->argv[5]);
    }
    strcpy(output,"rdbe_if/");
    strcat (output,data);
    for (i=0;i<5;i++)ip[i]=0;
    cls_snd(&ip[0],output,strlen(output),0,0);
    ip[1]=1;
    return;
error:
    close(t450_fd);
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"rd",2);
    return;
}
int locate(t,s,n) /*find posn of char t in s starting at posn n*/
char s[],t; int n;
{
  int i,j,k;
  for (i=n;s[i] != '\0';i++) {
      if(s[i]==t)
         return(i);
      }
  return(-1);
}
