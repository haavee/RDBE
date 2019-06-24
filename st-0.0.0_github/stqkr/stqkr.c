/* stqkr - C version of station command controller */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/fscom.h"
#include "../../fs/include/shm_addr.h"     /* shared memory pointer */

#include "../include/stparams.h"
#include "../include/stcom.h"

#include "aosend.h"

struct stcom *st;
struct fscom *fs;

#define MAX_BUF   257

main() {
    long ip[5];
    long ipsave[5];
    long outclass;
    int isub,itask,it1,idum,ierr,nchars,i;
    char buf[MAX_BUF];
    char copy_buf[MAX_BUF];
    struct cmd_ds command;
    int cls_rcv(), cmd_parse();
    void skd_wait();

    /*   char effstat[10]; *//* holds message >>RX etc. */

    /* Set up IDs for shared memory, then assign the pointer to
     * "fs", for readability.
     */
    setup_ids();
    fs = shm_addr;
    setup_st();
loop:
    skd_wait("stqkr",ip,(unsigned) 0);
    if(ip[0]==0) {
        ierr=-1;
        goto error;
    }
    /* cls_rcv() gives us at most MAX_BUF characters
     * and it might be NUL terminated or not */
    nchars=cls_rcv(ip[0],buf,MAX_BUF,&idum,&idum,0,0);
    if( nchars==MAX_BUF && buf[nchars-1] != '\0' ) { /*does it fit?*/
        ierr=-2;
        goto error;
    }
    /* null terminate to be sure */
    if(nchars < MAX_BUF && buf[nchars-1] != '\0') buf[nchars]='\0';
    strncpy(copy_buf,buf,nchars);//make 2nd copy to pass on
    copy_buf[nchars]='\0';
    if(0 != (ierr = cmd_parse(buf,&command))) { /* parse it */
        ierr=-3;
        goto error;
    }

    isub = ip[1]/100;
    itask = ip[1] - 100*isub;

    switch (isub) {
        //--------------------------------------------------------------------
#if 0
        case 4: 
            if(itask == 7 )effcal(&command,ip,isub,itask);  /* 407 is cal */
            if(itask == 4 )effwx(&command,ip,isub,itask);  /* 404 is wx */
            break;
#endif
                //--------------------------------------------------------------------
        case 5:  /* DBBC Testversion, see usr2/control/stcmd.ctl, commands 5xx*/
                /*RDBE added*/
                switch (itask) {
                    case 39:  rdbe_init(&command,itask,ip); break;  //new: read in addresses to common, have to start with this
                    case 40:  rdbe_cmd(&command,itask,ip); break;
                              /*could  call the python alc_adjust here, in fact this is now a call
                                to a standalone c program  */
                    case 41:  p_rdbe_alc(&command,itask,ip); break;
                    case 42:  rdbe_adc(&command,itask,ip); break;
                    case 43:  rdbe_time(&command,itask,ip); break;
                    case 44:  rdbefclr(&command,itask,ip); break;
                    case 45:  rdbe_tsys(&command,itask,ip); break;
                              //                     case 46:  rdbe_assign(&command,itask,ip); break;
                              //commands to enter rdbe frequency channels to table (needed for tsys)
                              //these do not send any commands to rdbe
                    case 60:  rdbe_fq(&command,itask,ip); break; //rdbef01
                    case 61:  rdbe_fq(&command,itask,ip); break;
                    case 62:  rdbe_fq(&command,itask,ip); break;
                    case 63:  rdbe_fq(&command,itask,ip); break;
                    case 64:  rdbe_fq(&command,itask,ip); break;
                    case 65:  rdbe_fq(&command,itask,ip); break;
                    case 66:  rdbe_fq(&command,itask,ip); break;
                    case 67:  rdbe_fq(&command,itask,ip); break;
                    case 68:  rdbe_fq(&command,itask,ip); break;
                    case 69:  rdbe_fq(&command,itask,ip); break;
                    case 70:  rdbe_fq(&command,itask,ip); break;
                    case 71:  rdbe_fq(&command,itask,ip); break;
                    case 72:  rdbe_fq(&command,itask,ip); break;
                    case 73:  rdbe_fq(&command,itask,ip); break;
                    case 74:  rdbe_fq(&command,itask,ip); break;
                    case 75:  rdbe_fq(&command,itask,ip); break; //rdbef16
                    case 80:  rdbe_if(&command,itask,ip); break; 
                    case 81:  rdbe_sync(&command,itask,ip); break; 
                    case 82:  rdbe_person(&command,itask,ip); break; 
                    case 83:  rdbe_dc_cfg(&command,itask,ip); break; 
                    case 84:  rdbe_send(&command,itask,ip); break; 
                    case 85:  rdbe_fchk(&command,itask,ip); break; 
                    case 86:  essr(&command,itask,ip); break; //soft switch machine
                }
                break;
#if 0
        case 14: 
                    if(itask == 2 )efflo(&command,ip,isub,itask);  /* 1402 is lo */
                    break;
#endif
        case 14: 
                    /* 1402 is lo */
                    /* 1403 is aosend */
                    {
                        int   n;
                        char  reply[1024] = {0};

                        n = sprintf(reply, "%s/", buf);
                        for(i=0; i<5; i++) ip[i]=0;

                        if( itask==2 )
                            ierr = (aolo(copy_buf, reply+n) < 0 ? -601 : 0);
                        else if( itask==3 )
                            ierr = (aosend(copy_buf, reply+n) < 0 ? -602 : 0);
                        else {
                            ierr = -600;
                        }

                        if( ierr<0 )
                            goto error_ao;

                        cls_snd(&ip[0], reply, strlen(reply), 0, 0);
                        ip[1]=1;
                    } 
                    break;
#if 0
        case 16: 
                    effrx(&command,ip,isub,itask);  /* 1601 is rx */
                    break;
        case 30:
                    effpcal(&command,ip,isub,itask);  /* 3000(1)  pcal off(on)*/
                    break;
#endif
        case 91:
                        /*getgps gets what truetime has written to st common and logs it*/
                        if(itask == 1)getgps(&command,ip,isub,itask); /*9101 gps */
                        break;
                        //--------------------------------------------------------------------
                        //  aosend + aolo

                        //--------------------------------------------------------------------
        default:
                        ierr=-4;
                        goto error;
    }
    goto loop;
error:
    for (i=0;i<5;i++) ip[i]=0;
    ip[2]=ierr;
    memcpy(ip+3,"s@",2);
    goto loop;
error_ao:
    ip[2]=ierr;
    memcpy(ip+3,"ao",2);
    goto loop;
}
