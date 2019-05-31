/* get time setting information from mk5cn */

#include <memory.h>
#include <stdio.h>
#include <sys/types.h>    // Needed for system defined identifiers.
#include <netinet/in.h>   // Needed for internet address structure.
#include <sys/socket.h>   // Needed for socket(), bind(), etc...
#include <arpa/inet.h>    // Needed for inet_ntoa()
#include <fcntl.h>        // Needed for sockets stuff
#include <netdb.h>        // Needed for sockets stuff
#include <sys/select.h>   // extra stuff select & timeout
#include <time.h>
#include <sys/time.h>
#include "/usr2/st/stqkr/rdbe_addr.h"

#define BUFSIZE 2048
#define  PORT_DBE         5000   
#define  BUFFER_SIZE      1024  unsigned int         dbe_fd,adc_fd;   // Server socket descriptors

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"

get_rdbe_issync(ip,m5sync,sz_m5sync)
long ip[5];                          /* ipc array */
char *m5sync;
int sz_m5sync;
{
  unsigned int         dbe_fd;
  struct sockaddr_in   dbe_addr;     // Server Internet address
  unsigned char       in_buf[1024];     // input buffer for data
  fd_set in_fdset;
  struct itimerval value;
  long centsc; //XXX
  struct tm *utime; //XXX
  int retval,iret;
    char dbe_cmd[200];
      int out_recs, nrecs, i, ierr,fd_width;
      long out_class, iclass;
      char *str;
      char *ptok;
      struct pps_source_cmd pps_lclc;
      struct clock_set_cmd clock_lclc;
      struct dot_mon lclm;
      char inbuf[BUFSIZE];
      int rtn1;    /* argument for cls_rcv - unused */
      int rtn2;    /* argument for cls_rcv - unused */
      int msgflg=0;  /* argument for cls_rcv - unused */
      int save=0;    /* argument for cls_rcv - unused */
      int nchars;
      double secs;
      int isec;
      char part_0[200],part_1[200], part_2[200],part_3[200],part_4[200];
      /* get 1pps_source? and clock_set? */
      dbe_fd = socket(AF_INET, SOCK_STREAM, 0); //first connect dbe main port for DBE control
      if(dbe_fd < 0){ierr =-402; goto error2;}
      dbe_addr.sin_family      = AF_INET;            // Address family to use
      dbe_addr.sin_port        = htons(PORT_DBE);    // Port num to use
      dbe_addr.sin_addr.s_addr = inet_addr(RDBE0); // IP address to use
      iret=connect(dbe_fd, (struct sockaddr *)&dbe_addr, sizeof(dbe_addr));
      if(iret < 0){ierr =-402; goto error2;}

      FD_ZERO(&in_fdset); FD_SET(dbe_fd,&in_fdset); 
      sprintf(dbe_cmd,"dbe_dot?;\n");
      send(dbe_fd, dbe_cmd, (strlen(dbe_cmd) + 1), 0);
      value.it_value.tv_usec = 500000;
      value.it_value.tv_sec = 0;
      fd_width=dbe_fd+1;
      retval = select(fd_width,&in_fdset,NULL,NULL,&value.it_value);
      if(retval < 0){ierr =-402; goto error2;}
      else if (retval ==0 ){ ierr =-402; goto got_timeout;} //timeout
      else {  
        if(FD_ISSET(dbe_fd,&in_fdset)){
          iret=recv(dbe_fd, in_buf, sizeof(in_buf), 0);
          ptok=strtok(in_buf,";");
          close(dbe_fd);
        }
      }   
//!dbe_dot?0:2012027170013:syncerr_eq_0:2012027170013:0:95361213
      ptok=strtok(in_buf,":"); strcpy(part_0,ptok);
      ptok=strtok(NULL,":"); strcpy(part_0,ptok); //2012027170013
      ptok=strtok(NULL,":"); strcpy(part_1,ptok); //syncerr_eq_0
      ptok=strtok(NULL,":"); strcpy(part_2,ptok); //2012027170013
      ptok=strtok(NULL,":"); strcpy(part_3,ptok); //0
      ptok=strtok(NULL,":"); strcpy(part_4,ptok); //95361213
      strcpy(m5sync,part_1); //XXXX
      strcpy(lclm.status.status,part_1); //XXXX
      if(!lclm.status.state.known) 
	strncpy(lclm.status.status,"sync unknown",sizeof(lclm.status.status));
      strncpy(m5sync,lclm.status.status,sz_m5sync);
      m5sync[sz_m5sync-1]=0;

      return 0;
got_timeout:
//happened quite often , so just close and try again
      close(dbe_fd);
      return 0;
error2:
      close(dbe_fd);
      ip[0]=0;
      ip[1]=0;
      ip[2]=ierr;
      memcpy(ip+3,"55",2);
error:
      return 0;
}
