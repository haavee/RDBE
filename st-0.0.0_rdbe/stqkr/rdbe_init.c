#define _BSD_SOURCE
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
#include <stdlib.h>
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../include/stm_util.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"
//#include "rdbe_addr.h"

//----- Defines ---------------------------------------------------------------
// Used as default port numbers
#define  PORT_DBE         5000     // Port number used at the server
#define  PORT_ADC         5050     // Port number used at the server
#define  PORT_T450        7000     // from rdbe_if.c
#define  PORT_ESSR        2620     // from essr.c
#define  BUFFER_SIZE       1024

//extern struct stcom *st;
//extern struct fscom *fs;


#define CONTROL_FILE "/usr2/control/rdbead.ctl"


void clear_addresses( void ) {
    int    i;
    /* Clear out all addresses */
    for(i=0; i<NRDBE; i++) {
        memset(&stm_addr->rdbe[i].addr,     0x0, sizeof(struct sockaddr_in));
        memset(&stm_addr->rdbe[i].tsys_mon, 0x0, sizeof(struct sockaddr_in));
        memset(&stm_addr->rdbe[i].pps_mon,  0x0, sizeof(struct sockaddr_in));
    }
    memset(&stm_addr->t450_addr,     0x0, sizeof(struct sockaddr_in));
    memset(&stm_addr->essr_addr,     0x0, sizeof(struct sockaddr_in));
}


//char dbe0_add[20],dbe1_add[20],t450_add[20],essr_add[20];
//=====rdbe communication ======================================================================
//==start with this to read addr of dbe0,1, t450 and essr unit to common====
//NBNB should multicast addr also be in here??????????????????????????????????????????????
void rdbe_init(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
//    char rdbe_command[COMMAND_LEN];
//    char data[DATA_SIZE];  
//    char  timout [10] , dbe_ip [30];
    char me [] ="rdbe";
//    char *ptok;
//    int itimeout;
/*
    char output[200],data[200];
    */
//    int i,idummy,len;
//    char                 out_buf[200];    // output buffer for data
//    fd_set in_fdset;
//    struct itimerval value;
//    int retval;
    int         ierr, i, linenr = 1;

    FILE*       fp;
    char*       line  = NULL;
    size_t      nchar = 0;
    ssize_t     nread;
    const char* separators = " \t\v\f";


    clear_addresses();

/*
    char rd1[40],rd2[40];
    char dbe0_add[20],dbe1_add[20],t450_add[20];
    */
//===================================
    ierr=0;
    if ( (fp = fopen(CONTROL_FILE,"r")) == NULL) {
        printf("cannot open RDBE address file %s\n",CONTROL_FILE);
        ierr=-539;/* goto error; */
    }
    /* cf.
     * http://pubs.opengroup.org/onlinepubs/9699919799/functions/getdelim.html
     */
    while( ierr==0 && (nread=(getline(&line, &nchar, fp)))!=-1 ) {
        int                 rdbe = -1;
        char                *ptr = line;
        char                *comment = strchr(line, '*');
        char                *item, *line_state, *tok_state;    /* for tokenizing */
        char                *host;
        unsigned short      port = (unsigned short)-1; 

        struct sockaddr_in* addrptr = NULL;

        /* Discard comment */
        if( comment!=NULL )
            *comment = '\0';
        /* Skip whitespace and if nothing left move on to next line */
        while( isspace(*ptr) )
            ptr++;
        if( !*ptr )
            continue;

        /* Start tokenizing the line by whitespace */

        /* Expect line format:
         * RDBEx IP/HOST[:PORT] [init commands]
         * T450  IP/HOST[:PORT]
         * ESSR  IP/HOST[:PORT]
         **/

        /* First item: RDBEx, T450 or ESSR */
        item = strtok_r(ptr, separators, &line_state);

        if( sscanf(item, "RDBE%d", &rdbe)==1 ) {
            /* Check for valid RDBE number */
            if( rdbe<0 || rdbe>=NRDBE ) {
                printf("RDBE number %d in config file is invalid\n", rdbe);
                ierr = -539;
                continue;
            }
            port    = PORT_DBE;
            addrptr = &stm_addr->rdbe[rdbe].addr;
        } else if( strcmp(item, "T450")==0 ) {
            port    = PORT_T450;
            addrptr = &stm_addr->t450_addr;
        } else if( strcmp(item, "ESSR")==0 ) {
            port    = PORT_ESSR;
            addrptr = &stm_addr->essr_addr;
        } 

        if( addrptr==NULL || port==-1 ) {
            printf("Unrecognized statement in RDBE control file line #%d\n", linenr);
            ierr = -539;
            continue;
        }

        /* We must have at least HOST/IP[:PORT] */
        if( (host=strtok_r(NULL, separators, &line_state))==NULL ) {
            printf("Missing IP/HOST[:PORT] for %s\n", item);
            ierr = -539;
            continue;
        }
        
        host = strtok_r(item, ":", &tok_state);
        if( resolve_host(host, SOCK_STREAM, IPPROTO_TCP, addrptr)!=0 ) {
            ierr = -539;
            continue;
        }

        /* backward compatibility for now.
         * strncpy() does not guarantee to terminate */
        if( rdbe==0 ) {
            strncpy(stm_addr->dbe0_add, host, sizeof(stm_addr->dbe0_add));
            stm_addr->dbe0_add[ sizeof(stm_addr->dbe0_add)-1 ] = '\0';
        } else if( rdbe==1 ) {
            strncpy(stm_addr->dbe1_add, host, sizeof(stm_addr->dbe1_add));
            stm_addr->dbe1_add[ sizeof(stm_addr->dbe1_add)-1 ] = '\0';
        } else if( port==PORT_T450 ) {
            strncpy(stm_addr->t450_add, host, sizeof(stm_addr->t450_add));
            stm_addr->t450_add[ sizeof(stm_addr->t450_add)-1 ] = '\0';
        } else if( port==PORT_ESSR ) {
            strncpy(stm_addr->essr_add, host, sizeof(stm_addr->essr_add));
            stm_addr->essr_add[ sizeof(stm_addr->essr_add)-1 ] = '\0';
        }

        /* port number given? */
        if( (item=strtok_r(NULL, separators, &tok_state))!=NULL &&
            sscanf(item, "%d", &port)!=1 ) {
                printf("Invalid port number %s at line %d\n", item, linenr);
                ierr = -539;
                continue;
        }
        addrptr->sin_port = htons(port);

        /* Anything else is optional, only supported for RDBEx */
        while( ierr==0 && rdbe>=0 && (item=strtok_r(NULL, separators, &line_state))!=NULL ) {
            char  *command, *broadcast, *port_s;
            /* Expect rdbe init commands like
             *   dbe_tsys_mon=enable:HOST:PORT[...]
             *   dbe_1pps_mon=enable:HOST:PORT[...]
             *   dbe_if_config=....
             *   dbe_arp=...
             *   dbe_data_connect=...
             * etc
             *  We ignore most of them but filter out tsys_mon
             */
            command = strtok_r(item, "=", &tok_state);
            if( strcmp(command, "dbe_tsys_mon") )
                /* not interested in not tsys mon */
                continue;
            /* break down by ":" */
            broadcast = strtok_r(NULL, ":", &tok_state);
            port_s    = strtok_r(NULL, ":", &tok_state); 

            /* Don't even try to resolve if nothing given */
            if( strlen(broadcast)==0 )
                continue;
            if( resolve_host(broadcast, SOCK_DGRAM, IPPROTO_UDP, &stm_addr->rdbe[rdbe].tsys_mon)!=0 ) {
                printf("Unresolvable tsys_mon broadcast addr '%s' at line #%d\n", broadcast, linenr);
                ierr = -539;
                continue;
            }
            if( strlen(port_s) ) {
                if( sscanf(port_s, "%d", &port)!=1 ) {
                    printf("Invalid port number '%s' for tsys_mon broadcast at line #%d\n", port_s, linenr);
                    ierr = -539;
                    continue;
                }
                stm_addr->rdbe[rdbe].tsys_mon.sin_port = htons( port );
            }
        }
        linenr++;
    }
    /* If any data was malloc'ed, free it */
    if( line )
        free(line);

/*
    while (!feof(fp)){
       fscanf(fp, "%s %s", &rd1,&rd2);
//       printf("%s %s\n", rd1,rd2);
       if(strncmp(rd1,"RDBE0",5)==0)strcpy(stm_addr->dbe0_add,rd2);
       if(strncmp(rd1,"RDBE1",5)==0)strcpy(stm_addr->dbe1_add,rd2);
       if(strncmp(rd1,"T450",4)==0)strcpy(stm_addr->t450_add,rd2);
       if(strncmp(rd1,"ESSR",4)==0)strcpy(stm_addr->essr_add,rd2);
    }
*/
    if( ierr==0 ) {
        int  nc;
        char reply[200];
        char ipa[4][INET_ADDRSTRLEN];

        strcpy(ipa[0], inet_ntoa(stm_addr->rdbe[0].addr.sin_addr));
        strcpy(ipa[1], inet_ntoa(stm_addr->rdbe[1].addr.sin_addr));
        strcpy(ipa[2], inet_ntoa(stm_addr->t450_addr.sin_addr));
        strcpy(ipa[3], inet_ntoa(stm_addr->essr_addr.sin_addr));

        nc = snprintf(reply, sizeof(reply), "rdbe_addr/%s#%s#%s#%s", ipa[0], ipa[1], ipa[2], ipa[3]);
        if( nc>sizeof(reply) )
            printf("Reply string was too long, lost %d characters from reply\n", nc-sizeof(reply));
        /* Set up return values */
        for (i=0;i<5;i++) ip[i]=0;
        cls_snd(&ip[0], reply, nc, 0, 0);
        ip[1]=1;
    } else {
        clear_addresses();
        ip[0]=0;
        ip[1]=0;
        ip[2]=ierr;
        memcpy(ip+3,"rd",2);
    }
    return;
}
