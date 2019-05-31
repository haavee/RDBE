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
//----- Defines ---------------------------------------------------------------
#define  PORT_DBE         5000     // Port number used at the server
#define  PORT_ADC         5050     // Port number used at the server
#define  BUFFER_SIZE      1024
#define  VAR_REF          20       //variance should be less than this
#define  ATTN_MAX         31       //start value for attenuation current
#define  ATTN_MIN          0       //min value for attenuation current
#define   STEP_SIZE        2      // 1 takes too long..
//===== Main program ==========================================================
int main(int argc, char **argv)
{
  unsigned int         dbe_fd,adc_fd;   // Server socket descriptors
  struct sockaddr_in   dbe_addr;     // Server Internet address
  struct sockaddr_in   adc_addr;     // Server Internet address
  char                 out_buf[200];    // output buffer for data
  int retval;
  char  dbe_cmd[200],dbe_ip[40];
  char *ptok;
  unsigned char                 in_buf[BUFFER_SIZE];     // input buffer for data
  fd_set in_fdset;
  struct itimerval value;
  int iret,i,j,lattn,if_chan;
  int timeout_secs;
  static int fd_width;	/* width of file descriptors being used */
  int adc_val[512];
  float sq, sq_mean,mean,variance;
//Command line
  timeout_secs=30; if_chan=0; //default
  if(argv[1] !=NULL){strcpy(dbe_ip,argv[1]);
    if(argv[2] !=NULL){sscanf(argv[2],"%d",&timeout_secs);
      if(argv[3] !=NULL){
         if(strncmp(argv[3],"if0",3) == 0)if_chan=0;
         if(strncmp(argv[3],"if1",3) == 0)if_chan=1;
         if(strncmp(argv[3],"0",1) == 0)if_chan=0;
         if(strncmp(argv[3],"1",1) == 0)if_chan=1;
      }
    }
  }
  for(lattn=ATTN_MAX; lattn > ATTN_MIN;lattn=lattn-STEP_SIZE){
    dbe_fd = socket(AF_INET, SOCK_STREAM, 0); //first connect dbe main port for DBE control
    dbe_addr.sin_family      = AF_INET;            // Address family to use
    dbe_addr.sin_port        = htons(PORT_DBE);    // Port num to use
// Now we call it with the IP directly
      dbe_addr.sin_addr.s_addr = inet_addr(dbe_ip); // IP address to use
    iret=connect(dbe_fd, (struct sockaddr *)&dbe_addr, sizeof(dbe_addr));
    if(iret <0){printf ("connect dbe %s failed ",dbe_ip);return(-1);}

    FD_ZERO(&in_fdset); FD_SET(dbe_fd,&in_fdset); 
    sprintf(dbe_cmd,"dbe_alc=%d:%d:off;\n",if_chan,lattn);
    send(dbe_fd, dbe_cmd, (strlen(dbe_cmd) + 1), 0);
    value.it_value.tv_usec = 0;
    value.it_value.tv_sec = timeout_secs;
    fd_width=dbe_fd+1;
    retval = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);
    if (retval == -1) {printf("select dbe failed");} //select failed
    else if (retval ==0 ){ printf("dbe_timeout");} //timeout
    else {  
      if(FD_ISSET(dbe_fd,&in_fdset)){
        recv(dbe_fd, in_buf, sizeof(in_buf), 0);
        ptok=strtok(in_buf,";");
//        printf("FMDBE=%s\n",ptok);
        close(dbe_fd);
      }
    } 
    sleep(2);
    adc_fd = socket(AF_INET, SOCK_STREAM, 0);
    adc_addr.sin_family      = AF_INET;            // Address family to use
    adc_addr.sin_port        = htons(PORT_ADC);    // Port num to use
    adc_addr.sin_addr.s_addr = inet_addr(dbe_ip); // IP address to use
    iret=connect(adc_fd, (struct sockaddr *)&adc_addr, sizeof(adc_addr));
    if(iret <0){printf ("connect adc failed %d",iret);return(-1);}
    FD_ZERO(&in_fdset); FD_SET(adc_fd,&in_fdset); 
    sprintf(out_buf,"%d;\n",if_chan);
    send(adc_fd, out_buf, (strlen(out_buf) + 1), 0);
    value.it_value.tv_usec = 0;
    value.it_value.tv_sec = 10;
    fd_width=adc_fd+1;
    iret = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);
    if(iret <0){printf ("select failed ");return(-1);}
    if(iret ==0){printf ("select timeout ");return(-1);}
//    printf("FDs set=%d\n",iret);
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
//      printf("DBGlattn mn sqmn var=%d %f %f %f",lattn,mean,sq_mean,variance);
      if(variance >= VAR_REF)goto breakout;
      close(adc_fd);
    }  
  }
breakout:
      printf("FINAL:attn=%d, mn sqmn VAR=%.4f %.2f %.2f",lattn,mean,sq_mean,variance);
//----------------------------------
}
