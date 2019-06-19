
#include <stdio.h>
#include <math.h>
#include <tk.h>

/*
  Tcl/TK procedure including
    - socket library connections
    - refresh timer
*/

void have_msg( void *, int);
void socktk_error( char *, char *);
char *sock_name();

int init_sockettk(i)
Tcl_Interp *i;
{
  int tk_socksend();
  int tk_bindsock();
  int starttimer();
  int sockmsg();
  int sockname();
  int tcl_open(), tcl_close();

  sock_openclose( tcl_open, tcl_close );
  Tcl_CreateCommand(i, "socksend", tk_socksend, NULL, NULL );
  Tcl_CreateCommand(i, "bindsock", tk_bindsock, NULL, NULL );
  Tcl_CreateCommand(i, "starttimer", starttimer, NULL, NULL );
  Tcl_CreateCommand(i, "sockmsg", sockmsg, NULL, NULL );
  Tcl_CreateCommand(i, "sockname", sockname, NULL, NULL );

  return( TCL_OK );
}

/* socksend MAILBOX value */

int tk_socksend( ClientData cld, Tcl_Interp *interp, int argc, char *argv[] )
{
  int handle;

  if( argc < 3 ) {
    sprintf( interp->result, "usage: socksend MAILBOX string" );
    return(TCL_ERROR);
  }

  if( !(handle = sock_connect( argv[1] ))) {
    sprintf( interp->result, "bad mailbox name" );
    return(TCL_ERROR);
  }

  if( !sock_send( handle, argv[2]) ) {
    sprintf( interp->result, 
       "unable to send message to %s, %s", argv[1], argv[2] );
    return(TCL_ERROR);
  }

 return( TCL_OK );
}

struct SOCKETGOO {
  Tcl_Interp *interp;
  char socket_proc[80]; /* procedure to call when socket is on */
  unsigned char *message;
  int len;
  int alloc_len;
  char error_msg[256]; /* last error message */
  int error;
} sgoo = {0};  

/* getsockmsg MAILBOX proc */

int tk_bindsock( ClientData cld, Tcl_Interp *interp, int argc, char *argv[] )
{
  extern int *bs; /* really BOUND structure but first element is desired int */
  void do_accept();
  void socktk_error( char *, char *);

  sock_seterror( socktk_error );

  if( argc < 3 || !sock_bind( argv[1] ) ) {
    sprintf( interp->result, "usage: getsockmsg MAILBOX proc" );
    return(TCL_ERROR);
  }

  strncpy( sgoo.socket_proc, argv[2], 80 );

  sgoo.alloc_len = 8192;
  if( argv[3] ) {
    if( (sgoo.alloc_len = atoi(argv[3])) < 8192 )
      sgoo.alloc_len = 8192;
  }

  strncpy( sgoo.socket_proc, argv[2], 80 );
  sgoo.message = (unsigned char *)malloc( sgoo.alloc_len );
  sock_bufct( sgoo.alloc_len + 1024 );

  sgoo.interp = interp;
  Tcl_CreateFileHandler( *bs, TCL_READABLE, do_accept, NULL );

  return(TCL_OK);
}

void do_accept(ign)
int ign;
{
  extern int accept_sock();
  int s;

  accept_sock();
}


void have_msg( s, mask )
void *s;
int mask;
{
  if( read_sock(s, sgoo.message, &sgoo.len ) < 0 )
    socktk_error( "closing socket %s", sock_name(s) );
    
  Tcl_GlobalEval( sgoo.interp, sgoo.socket_proc );
}

int sockmsg( ClientData cld, Tcl_Interp *interp, int argc, char *argv[] )
{
  int handle;

  Tcl_AppendResult(interp, sgoo.message, NULL ); 

  if( sgoo.error ) {
    sgoo.error = 0;
    strcpy( interp->result, sgoo.error_msg );
    return(TCL_ERROR);
  } else
    return( TCL_OK );
}

/* to be called from outside */

int sock_msg( pp )
unsigned char **pp;
{
  *pp = sgoo.message;
  return(sgoo.len);
}


int sockname( ClientData cld, Tcl_Interp *interp, int argc, char *argv[] )
{
  int handle;

  Tcl_AppendResult(interp, sock_name(last_msg()), NULL ); 

 return( TCL_OK );
}


/*
   starttimer secs proc
*/

struct TIMERGOO {
  Tcl_Interp *interp;
  char cmd[80];
  int timeout;
};

static struct TIMERGOO goo;

int starttimer( ClientData cld, Tcl_Interp *interp, int argc, char *argv[] )
{
  void do_cmd( void *); 

  if( argc < 3 ) {
    sprintf( interp->result, "usage: starttimer secs proc" );
    return(TCL_ERROR);
  }
  
  goo.timeout = atoi( argv[1] ); 
  goo.interp = interp;
  strncpy( goo.cmd,  argv[2], sizeof(goo.cmd) );
  Tcl_CreateTimerHandler( goo.timeout*1000, do_cmd, &goo );
  return(TCL_OK);
}

void do_cmd(s)
void *s;
{
  Tcl_GlobalEval( ((struct TIMERGOO *)s)->interp,((struct TIMERGOO *)s)->cmd );
  Tcl_CreateTimerHandler( ((struct TIMERGOO *)s)->timeout*1000, do_cmd, &goo );
}


tcl_open(s, fd)
int s;
int fd;
{
  if( s )
    Tcl_CreateFileHandler( fd, TCL_READABLE, have_msg, (void *)s );
}

tcl_close(fd)
int fd;
{
  Tcl_DeleteFileHandler(fd);
}


void socktk_error( char *a, char *b)
{
  sprintf( sgoo.error_msg, a, b );
  sgoo.error = 1;
}

