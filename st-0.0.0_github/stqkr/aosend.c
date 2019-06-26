/* routines for communicating with Arecibo CIMA,
 *  aosend + lo commands
 *
 * Copied from vlbis1.naic.edu:/usr2/st-0.0.0_rdbe/stqkr/stqkr.c
 *          on 21Jun2019
 *          by Harro Verkouter
 *  and modified by same
 */
#include <string.h>
#include <stdio.h>
#include "../socklib/sock.h"

/* Do initialization once */
void aoinit( void ) {
    static int ao_init = 0;

    if( ao_init )
        return;
    setenv( "MAILBOXDEFS",  "/usr2/st/mailboxdefs", 1 );
    sock_bind("VLBISTQKR");
    ao_init = 1;
}

/* aosend=..... */
int aosend(char* cmd, char* reply) {
    int     err;
    char*   command;

    reply[0] = '\0';
    if( (command=strtok(cmd, "="))==NULL ) {
        strcpy(reply, "no equal sign");
        return -1;
    }
    if( strcmp(command, "aosend") ) {
        strcpy(reply, "command is not aosend");
        return -1;
    }
    /* get the real command to send */
    if( (command=strtok(NULL, "\0"))==NULL || !strlen(command) ) {
        strcpy(reply, "no actual command to send");
        return -1;
    }
    printf("aosend()/command='%s'\n", command);
    /* make sure initialized */
    aoinit();
    if( (err=sock_send(sock_connect("EXECUTIVE"), command))<0 ) {
        sprintf(reply, "sock_send fails - %d", err);
        return -1;
    }
    strcpy(reply, "aosend has no status");
    return 0;
}

/* lo=..... */
int aolo(char* cmd, char* reply) {
    int     err;
    char    send[2048];
    char*   command;

    reply[0] = '\0';
    if( (command=strtok(cmd, "="))==NULL ) {
        strcpy(reply, "no equal sign");
        return -1;
    }
    if( strcmp(command, "lo") ) {
        strcpy(reply, "command is not lo");
        return -1;
    }
    /* Get whatever's after the '=' */
    if( (command=strtok(NULL, "\0"))==NULL || !strlen(command) ) {
        strcpy(reply, "missing arguments");
        return -1;
    }
    /* build the real command to send */
    build_lostr(command, send);
    printf("aolo()/lo string='%s'\n", send);

    /* make sure initialized */
    aoinit();
    if( (err=sock_send(sock_connect("EXECUTIVE"), send))<0 ) {
        sprintf(reply, "sock_send fails - %d", err);
        return -1;
    }
    strcpy(reply, "lo has no status");
    return 0;
}

/* sloppy, but works
 * Note: HV 21Jun2019 - the "lo=" has already been stripped
 *                      by the caller 
 */
build_lostr(char* in, char* out) {
    char *s;

    *out = 0;
    strcat( out, "vlbi_lo " );

    s = strtok( in, ",");
    if( s )
        strcat( out, s );

    while(s = strtok( NULL, ",")) {
        strcat( out, " " );
        strcat( out, s );
    }
}
