/*
 * receive and interpret Tsys information from RDBE PPC
 * multicast code fm A Courtney/F Bastien 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <byteswap.h>
#include "../../fs/include/params.h"     /* FS parameters            */
#include "../../fs/include/fs_types.h"   /* FS header files        */
#include "../../fs/include/fscom.h"      /* FS shared mem. structure */
#include "../../fs/include/shm_addr.h"   /* FS shared mem. pointer*/
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"   /* Station shared mem. pointer*/
//  20020 for 1pps , 20021 for Tsys?? At moment 20021 is fixed in rdbe_server
#define HELLO_PORT 20021 
#define HELLO_GROUP "239.0.2.25"
#define MSGBUFSIZE 1024

//FS----------------------
struct stcom *st;
//------------------------

int64_t __builtin_bswap64(int64_t x) ;
int32_t __builtin_bswap32(int32_t x) ;

u_int16_t bswap16(x)
	u_int16_t x;
{
	return ((x << 8) & 0xff00) | ((x >> 8) & 0x00ff);
}
void exit();
main(int argc, char *argv[])
{
//Hichem seine Struktur======================
     struct tsys_data_test
     {
          uint16_t interface;
          uint16_t channel;
          uint32_t crap; //hat er nicht angegeben, ist aber drin. Sack.
          uint64_t tsys_on;
          uint64_t tsys_off;
     };

     struct tsys {
          char read_time[14];
          uint16_t interval;
          struct tsys_data_test tsys_data_1[32];
     };
     struct tsys rdbe_tsys;
//============================================
     struct sockaddr_in addr;
     int fd, nbytes,addrlen,i;
     uint16_t x16;
     uint32_t x32;
     uint64_t x64,x64_on,x64_off;
     long long int x64_diff;
     float tsratio;
     struct ip_mreq mreq;
     unsigned char msgbuf[MSGBUFSIZE];
     u_int yes=1;            /*** MODIFICATION TO ORIGINAL */

     setup_st();  //FS common

     /* create what looks like an ordinary UDP socket */
     if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  perror("socket");
	  exit(1);
     }


    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
       perror("Reusing ADDR failed");
       exit(1);
       }

     /* set up destination address */
     memset(&addr,0,sizeof(addr));
     addr.sin_family=AF_INET;
     addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
     addr.sin_port=htons(HELLO_PORT);
     
     /* bind to receive address */
     if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	  perror("bind");
	  exit(1);
     }
     
     /* use setsockopt() to request that the kernel join a multicast group */
     mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
     mreq.imr_interface.s_addr=htonl(INADDR_ANY);
     if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
	  perror("setsockopt");
	  exit(1);
     }

     /* now just enter a read-print loop */
     while (1) {
	  addrlen=sizeof(addr);
	  if ((nbytes=recvfrom(fd,&rdbe_tsys,sizeof(rdbe_tsys),0,(struct sockaddr *) &addr,&addrlen)) < 0) {
	       perror("recvfrom");
	       exit(1);
	  }
          
//          printf("NBYTE=%d  ",nbytes);
//byteswap as PPC is big endian, x86 little endian
//interface is 0 or 1 for if0,if1, channel is 0-15
          x16=bswap16 (rdbe_tsys.interval);rdbe_tsys.interval=x16;
          x16=bswap_16(rdbe_tsys.tsys_data_1[0].interface); rdbe_tsys.tsys_data_1[0].interface=x16;
          for(i=0;i<16;i++){
            x16=bswap_16(rdbe_tsys.tsys_data_1[i].channel); rdbe_tsys.tsys_data_1[i].channel=x16;
            x64_on=bswap_64(rdbe_tsys.tsys_data_1[i].tsys_on); 
            x64_off=bswap_64(rdbe_tsys.tsys_data_1[i].tsys_off); 
            stm_addr->rdbe0_if0_caloff[i]=x64_off; //to st common
            stm_addr->rdbe0_if0_calon [i]=x64_on ;
          }
          x16=bswap_16(rdbe_tsys.tsys_data_1[16].interface); rdbe_tsys.tsys_data_1[16].interface=x16;
          for(i=16;i<32;i++){
            x16=bswap_16(rdbe_tsys.tsys_data_1[i].channel); rdbe_tsys.tsys_data_1[i].channel=x16;
            x64_on=bswap_64(rdbe_tsys.tsys_data_1[i].tsys_on); 
            x64_off=bswap_64(rdbe_tsys.tsys_data_1[i].tsys_off); 
            stm_addr->rdbe0_if1_caloff[i-16]=x64_off; //to st common
            stm_addr->rdbe0_if1_calon [i-16]=x64_on ;
          }
     }
}
//----------------------------------
//  int  rdbe0_if[32]; //only need 16 really...
//  int  rdbe0_tk[32];
//  float rdbe0_chanfreq [32]; //read in if, tkno, chan freq, poln 
//  char rdbe0_pol[32];
//  float rdbe0_tcal[32];
//----------------
//  double rdbe0_if0_caloff[32]; //Tsys readout from RDBE
//  double rdbe0_if0_calon[32];
//  double rdbe0_if1_caloff[32];
//  double rdbe0_if1_calon[32];
//2012223125202 0 : 0 791128132 791071725  1 402738107 402619438  2 438859533 438825127  3 469366393 469347715  4 454586042 454606426  5 466784598 466794148  6 487241913 487053924  7 488059537 487942565  8 529624018 529629347  9 459115110 459015412  10 462848949 462746180  11 483308225 483340323  12 487123095 487108534  13 479384002 479435005  14 459012464 458872382  15 426863854 426840102  
//
//2012223125202 1 : 0 751515413 752059081  1 363755126 363898393  2 397406528 397470880  3 370989633 371136561  4 396314888 396430199  5 386927496 387070727  6 347829941 347917674  7 389422658 389565937  8 386757894 386910026  9 361548399 361672314  10 398624217 398562860  11 374532016 374555262  12 360512859 360672325  13 370738966 370801962  14 362709280 362790855  15 368835794 368899449  
