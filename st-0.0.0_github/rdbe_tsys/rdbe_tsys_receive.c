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
#include <errno.h>
#include <byteswap.h>
#include "../../fs/include/params.h"     /* FS parameters            */
#include "../../fs/include/fs_types.h"   /* FS header files        */
#include "../../fs/include/fscom.h"      /* FS shared mem. structure */
#include "../../fs/include/shm_addr.h"   /* FS shared mem. pointer*/
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"   /* Station shared mem. pointer*/

#if 0


//  20020 for 1pps , 20021 for Tsys?? At moment 20021 is fixed in rdbe_server
#define HELLO_PORT 20021 
//GROUP 239.0.2.34 for rdbe0, 35 for rdbe1
#define HELLO_GROUP "239.0.2.34"
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
#endif

#define DEFAULT_PORT "20021"
/*
 * rdbe_tsys_receive <rdbe:int> <multicastIP> [<multicastPort(20021)]
 */
main(int argc, char *argv[]) {
    int                 i, rdbe, fd, reuse_addr = 1;
    long                tmp;
    char                opts[3][256] = {{0}, {0}, {0}};
    char*               p;
    unsigned short      port;
    struct ip_mreq      mreq;
    struct sockaddr_in  addr;    

    /* require rdbe number and multicast IP*/
    if( argc<3 ) {
        printf("Usage: %s <rdbe:int> <multicast:IPv4> [<multicast:port(%s)>]\n", argv[0], DEFAULT_PORT);
        return 1;
    }
    /* default port */
    strcpy(opts[2], DEFAULT_PORT);
    strncpy(opts[0], argv[1], sizeof(opts[0])-1);
    strncpy(opts[1], argv[2], sizeof(opts[1])-1);
    if( argc>3 )
        strncpy(opts[2], argv[3], sizeof(opts[2])-1);
    /* Make sure everything is null terminated (strncpy does not guarantee this */
    for(i=0; i<3; i++)
        opts[0][sizeof(opts[0])-1] = '\0';
   
    /* base 10 rdbe index */
    errno = 0;
    rdbe = (int)strtol(opts[0], &p, 10);
    if( errno==ERANGE || errno==EINVAL || *p!='\0' ){
        printf("Please give a valid RDBE index (0,1) in stead of '%s'\n", opts[0]);
        return 1;
    }
    /* IPv4 address directly into mreq multicast request structure */
    if( inet_aton(opts[1], &mreq.imr_multiaddr)==0 ) {
        printf("Please give a valid IPv4 address in stead of '%s'\n", opts[1]);
        return 1;
    }
    /* port number */
    errno = 0;
    tmp = strtol(opts[2], &p, 10);
    if( errno==ERANGE || errno==EINVAL || *p!='\0' || tmp<=0 || tmp>65534 ){
        printf("Please give a valid port number in stead of '%s'\n", opts[2]);
        return 1;
    }
    port = (unsigned short)tmp;

    /* Fill in the socket's address in preparation for the bind */
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
    addr.sin_port=htons(port);

    /* Part of mreq has already been filled in from the command line
     * so only do the missing bit here */
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    /* Set up the socket */
    if( (fd=socket(AF_INET, SOCK_DGRAM, 0))<0 ) {
        printf("Failed to create socket - %s\n", strerror(errno));
        return 1;
    }
    /* enable reuseaddr, bind and join multicast */
    if( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr))<0 ) {
        printf("Failed to enable SO_REUSEADDR - %s\n", strerror(errno));
        return 1;
    }
    if( bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))<0 ) {
        printf("Failed to bind socket - %s\n", strerror(errno));
        return 1;
    }
    if( setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq))<0 ) {
        printf("Failed to join multicast - %s\n", strerror(errno));
        return 1;
    }

    /* And we should be up-and-running. Tell user exactly what we're doing.
     * That's why we use inet_ntoa/ntohs because we transform the values
     * that we gave to the O/S back into human readable form as double
     * check*/
    printf("RDBE_TSYS_RECEIVE[%d] listening for TSYS on %s:%d\n", rdbe, inet_ntoa(mreq.imr_multiaddr), ntohs(addr.sin_port));
    while( 1 ) {
        ssize_t            n;
        socklen_t          addrlen = sizeof(struct sockaddr_in);
        unsigned char      tsys_msg[65536];
        struct sockaddr_in sender;

        if( (n=recvfrom(fd, (void*)tsys_msg, sizeof(tsys_msg), MSG_WAITALL, (struct sockaddr*)&sender, &addrlen))<0 ) {
            printf("recvfrom on fd failed, terminating - %s\n", strerror(errno));
            break;
        }
        /*printf("RDBE[%d] %d bytes of TSYS\n", rdbe, n);*/
    }
    return 0;
}
#if 0
//============================================
     struct sockaddr_in addr;
     int fd, nbytes,addrlen,i,j;
     uint16_t ui161,ui162;
     uint32_t ui321,ui322;
     int32_t i321,i322;
     uint64_t ui641,ui642;
     int64_t i641,i642;
     char read_time[20];
     uint16_t channel,interface,interval;
//     uint16_t x16;
//     uint32_t x32;
//     uint64_t x64,x64_on,x64_off;
     uint64_t x64_on,x64_off;
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
	  if ((nbytes=recvfrom(fd,&msgbuf,sizeof(msgbuf),0,(struct sockaddr *) &addr,&addrlen)) < 0) {
	       perror("recvfrom");
	       exit(1);
	  }
          
//          printf("NBYTE=%d  \n",nbytes);
//byteswap as PPC is big endian, x86 little endian, extra bytes on receive because of issues with padding
//interface is 0 or 1 for if0,if1, channel is 0-15
//==========================================================================A===========
//Expect 784 bytes PFB and 152 for 4-channel DDC
//if ~ 600 bytes it is PFB otherwise DDC
           if(nbytes >300){
//========================================PFB===============================A===========
             memcpy(read_time,&msgbuf[0],14);
             memcpy(&ui161,&msgbuf[14],2);
             interval=bswap_16(ui161);
             for(i=16;i<nbytes;i=i+24){
               memcpy(&ui161,&msgbuf[i],2); memcpy(&ui162,&msgbuf[2+i],2);
               channel=bswap16(ui161); interface=bswap16(ui162);
               memcpy(&ui641,&msgbuf[8+i] ,8); ui641=bswap_64(ui641);
               memcpy(&ui642,&msgbuf[16+i],8); ui642=bswap_64(ui642);
               /*channel =0 or 1 for if0,1, interface is 0-15 */
               if(channel == 0){
                 stm_addr->rdbe0_if0_caloff[interface]=ui641; //to st common
                 stm_addr->rdbe0_if0_calon [interface]=ui642 ;
               }
               if(channel == 1){
                 stm_addr->rdbe0_if1_caloff[interface]=ui641; 
                 stm_addr->rdbe0_if1_calon [interface]=ui642 ;
               }
//             printf("%d %d %lld %lld\n",channel,interface,ui641,ui642); /*channel =0 or 1 for if0,1, interface is 0-15 */
             }
           } else {
//=========================================DDC============================================
             memcpy(read_time,&msgbuf[0],15);
             memcpy(&ui321,&msgbuf[15],4);
             memcpy(&ui322,&msgbuf[19],4);
             ui321=bswap_32(ui321);
             ui322=bswap_32(ui322);
             for(i=0;i<4;i++){
               j=i*32;
               memcpy(&ui641,&msgbuf[24+j],8); ui641=bswap_64(ui641);
               memcpy(&ui642,&msgbuf[32+j],8); ui642=bswap_64(ui642);
               memcpy(&i641,&msgbuf[40+j],8); i641=bswap_64(i641);
               memcpy(&i642,&msgbuf[48+j],8); i642=bswap_64(i642);
//               diffi64=ui641-ui642;
//               idiffi64=i641-i642;
//put everything in if0, sort out later which IF belongs to
               stm_addr->rdbe0_if0_caloff[i]=ui642; //to st common
               stm_addr->rdbe0_if0_calon [i]=ui641 ;
//               printf("%s : %d %lld %lld %lld %lld \n",read_time,i,ui641,ui642,i641,i642);
             }
             for(i=4;i<16;i++)stm_addr->rdbe0_if0_caloff[i]=0; //zero unused for DDC
             for(i=4;i<16;i++)stm_addr->rdbe0_if0_calon [i]=0; 
             for(i=0;i<16;i++)stm_addr->rdbe0_if1_caloff[i]=0; //zero all these for DDC, serves also  as flag
             for(i=0;i<16;i++)stm_addr->rdbe0_if1_calon [i]=0; 
//=====================================================================================
//             for(i=0;i<5;i++){
//                printf("DB%d %f %f\n", i,stm_addr->rdbe0_if0_caloff[i] ,stm_addr->rdbe0_if1_calon [i]);
//             }
           }
//=====================================================================================
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
#endif