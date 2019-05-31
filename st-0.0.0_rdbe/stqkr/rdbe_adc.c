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


void rdbe_adc(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
    char rdbe_command[COMMAND_LEN];
    char data[DATA_SIZE];  
    char  timout [10] , dbe_ip [30];
    char *ptok;
    char me []="rdbe";
    int itimeout;
    char output[200];
    unsigned char                 in_buf[BUFFER_SIZE];     // input buffer for data
    int i,ierr,idummy;
    unsigned int         adc_fd;        // Server socket descriptor
    struct sockaddr_in   server_addr;     // Server Internet address
    char                 out_buf[200];    // output buffer for data
    fd_set in_fdset;
    struct itimerval value;
    int retval;
    int if_chan;
    static int fd_width;	/* width of file descriptors being used */
    int adc_val[512];
    float mean,sq,sq_mean,variance;
    
//===================================
    strcpy(dbe_ip,stm_addr->dbe0_add); itimeout=2; //dbe 0
    strcpy(out_buf,"0");
    if(command->argv [0] !=NULL ){
      if(strncmp(command->argv [0],"0",1) == 0)strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0
      if(strncmp(command->argv [0],"1",1) == 0)strcpy(dbe_ip,stm_addr->dbe1_add);  //dbe 1
      if(strncmp(command->argv [0],"dbe0",4) == 0)strcpy(dbe_ip,stm_addr->dbe0_add);  //dbe 0
      if(strncmp(command->argv [0],"dbe1",4) == 0)strcpy(dbe_ip,stm_addr->dbe1_add);  //dbe 0
      if(command->argv [1] !=NULL ){
         strcpy(timout,command->argv [1]);
         sscanf(timout,"%d",&itimeout);
      } else {
         itimeout=2; 
      }
    } 
    if(strncmp(command->argv [2],"0",1) == 0)if_chan=0;
    if(strncmp(command->argv [2],"1",1) == 0)if_chan=1;
    if(strncmp(command->argv [2],"if0",3) == 0)if_chan=0;
    if(strncmp(command->argv [2],"if1",3) == 0)if_chan=1;
    sprintf(out_buf,"%d;\n",if_chan,command->argv [1]);
//==================================
//    strcpy(out_buf,command->argv [2]);   //dbe command  (which should be a "0" or "1")
    adc_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family      = AF_INET;            // Address family to use
    server_addr.sin_port        = htons(PORT_ADC);    // Port num to use
    server_addr.sin_addr.s_addr = inet_addr(dbe_ip); // IP address to use
    connect(adc_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (retval == -1) {ierr=-543; goto error;} //connect failed
    send(adc_fd, out_buf, (strlen(out_buf) + 1), 0);
    FD_ZERO(&in_fdset);
    FD_SET(adc_fd,&in_fdset);
    value.it_value.tv_usec = 0;
    value.it_value.tv_sec = 1;
    fd_width=adc_fd+1;
    retval = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);
    if (retval == -1) {ierr=-543; goto error;} //select failed
    else if (retval){
      if(FD_ISSET(adc_fd,&in_fdset)){
        recv(adc_fd, in_buf, sizeof(in_buf), 0);
        mean=0.0; sq_mean=0;
        for(i=0; i<512; i++){
          adc_val[i]=in_buf[i]-128;
          mean=mean+adc_val[i];
          sq=adc_val[i]*adc_val[i];
          sq_mean=sq_mean+sq;
        }
        mean=mean/512; sq_mean=sq_mean/512;
        variance=sq_mean-mean*mean;
        sprintf(data,"mean,var=%.4f, %.2f",mean,variance);
      }
    }
    else {ierr=-543; goto error;} //timeout
    close(adc_fd);
    strcpy(output,"rdbe_adc");
    strcat(output,"/");
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
