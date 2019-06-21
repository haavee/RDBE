
/* new one file version of library with open/close hooks */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>

#include "sock.h"

/*
    Jeffs socket library

    -  is used for easy re-connect socket communication 

    -  is simpler - without signals and the asynchronous maintainance 
       of structures.

    -  Only one bound socket is permitted

    -  Binding must be done to receive messages

    Routines:
      sock_bind(name)  - binds this process to name as defined in mailboxdefs
      sock_connect(name)  - returns an integer that is passed to sock_send
      sock_send( s, message ) - sends message to mailbox associated with s
        from sock_connect
      sock_write( s, data, n ) - sends data to mailbox associated with s
        from sock_connect
      sock_sel(message, l, p, n, tim, rd) - blocks on connections and messages.
        message is the buffer where a socket message will be put.
        l is the length of message (returned);
        p is an array of file descriptors that sock_sel will select on
        n is the length of p;
        tim is a timeout in seconds
        rd true means that sock_sel will read stdin
        sock_sel returns:
          -1 if a timeout.
          -2 if an error
          -3 if interrupted by a signal
          0 if standard in is ready for input
          or the file descriptor from 'p' that is ready.
          or the file descriptor of the socket that filled in message.
      sock_ssel(s, ns, message, l, p, n, tim, rd) - Same as sock_sel, but
	does not block on messages for the sockets in array, s (sock_connect 
	return values).
	ns is the length of s
	rest of parameters same as sock_sel.
      sock_close(name) - close opened connection - if NULL close all
      sock_name(last_msg()) - returns a status string pointing to the mbname
      sock_find(name) - returns handle for name
      sock_only(handle, message, tim) - blocks only on handle or timeout
      sock_poll(handle, message, tim, plen) - polls one socket and returns len
      sock_fd(handle) - returns a fd associated with handle 
      sock_intr(flag) - sock_sel will return when select is interrupted if flag
        is true.
      sock_bufct(n) - set out-of-sync size normally 4096

      window manager support is gained with calls to the open/close  
      file descriptor 

      sock_openclose( open, close );
      sock_seterror(err);
*/

struct BOUND sock_bound = { 0, NULL, NULL, {0}, 4096, 0, 0, 0 };

/*
    sock_bind is used to establish a destination socket
      that is defined in the mailboxdefs file
      you should only call this once.
*/

struct BOUND *bs = &sock_bound;
int accept_sock();
static int readtm();
static struct DEST sock_ano = { 0, "anonymous", "ANONYMOUS" };
struct SOCK sock_kbd = { 0, &sock_ano, 0, {0} };
int last_conn;
static init_dest();

static int destflag = 0;
static struct DEST mailboxes[MAXDEST];

static struct DEST def_mailboxes[MAXDEST] = {
  0,    "",          "" 
};


sock_bind(mbname)
char *mbname;
{
  int i, on, count=0, anon;
  struct hostent *hploc;
  char host[80];
  void standard_error( char *, char *);

  if( !bs->error )
    bs->error = standard_error;

  if( bs->bind_fd >0 )
    return((int)bs);

  anon = strcmp(mbname, "ANONYMOUS") == 0;

  if( anon ) {
    bs->dest = &sock_ano;
  } else if( (bs->dest = find_dest(mbname)) == NULL ) {
    (*bs->error)("sock_bind:unknown mailbox",0);
    return(0);
  }
 
  gethostname(host, 80);
/*
  if( strcmp( host, bs->dest->host )) {
    (*bs->error)( "sock_bind: %s is not localhost", bs->dest->host );
    return(0);
  }
*/

  if( !anon && (hploc = gethostbyname(bs->dest->host)) == NULL ) {
    (*bs->error)( "sock_bind:can't find %s", bs->dest->host );
    return(0);
  }


  if( (bs->bind_fd = socket( AF_INET, SOCK_STREAM, 0)) <0 ) {
    (*bs->error)("socket error", 0);
    return(0);
  }

  bs->sin.sin_port = htons(bs->dest->dport);
  bs->sin.sin_addr.s_addr = INADDR_ANY;

  on = 1;
  setsockopt(bs->bind_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

  while( bind( bs->bind_fd, (void *)&bs->sin, sizeof(struct sockaddr_in) ) <0 ){
    (*bs->error)("bind", 0);
    if( ++count > 12 )
      exit(0);
    sleep(5);
  }
  if( count >0 )
    (*bs->error)( "finally bound\n", 0);

  if( listen( bs->bind_fd, 5 ) < 0  ) {
    (*bs->error)("listen", 0);
    exit(0);
  }
  signal( SIGPIPE, SIG_IGN );
  if( bs->open )
    (*bs->open)( NULL, bs->bind_fd );
 
  return((int)bs);
}

/*
    sock_connect returns a pointer to a structure that can be later passed to
       sock_send for sending a message
*/

sock_connect(mbname)
char *mbname;
{
  struct SOCK *s;
  struct DEST *d;
  struct hostent *hp;
  int bflag = 1, anon;
  extern struct DEST sock_ano;

  s = bs->head;

  if(strcmp( mbname, "ANONYMOUS" )) {
    while(s) {
      if( s->dest && strcmp( mbname, s->dest->dname )==0 )
        return((int)s);
      s = s->next;
    }
    if((d= find_dest(mbname)) == NULL ) {
      (*bs->error)( "sock_connect:unknown mailbox %s", mbname );
      return(0);
    }
  } else {
    while(s) {
      if( s->dest && strcmp( mbname, s->dest->dname )==0 && s->fd < 0 )
        break;
      s = s->next;
    }
    d = &sock_ano;
    bflag = 0;
  }

  if( !s ) {
    s = (struct SOCK *)malloc(sizeof(struct SOCK));
    bzero(s, sizeof(struct SOCK ));
    s->fd = -1;
    s->dest = d;
    s->next = bs->head;
    bs->head = s;
  }

  if( bflag ) { /* if anon then it wont ever connect */
    if( (hp = gethostbyname(s->dest->host)) == NULL ) {
      (*bs->error)( "sock_connect:can't find host", 0);
      return(0);
    }

    s->sin.sin_family = hp->h_addrtype;
    bcopy( hp->h_addr, &s->sin.sin_addr, hp->h_length );
    s->sin.sin_port = htons(s->dest->dport);
  }
 
  return((int)s);
}

read_sock(s, msg, l)
struct SOCK *s;
char *msg;
int *l;
{
  unsigned long count, ct, net;
  int n;
  
  last_conn = (int)&sock_kbd;
  if( (n=read(s->fd, &net, sizeof(unsigned int) )) >0 ) {
    count = ntohl(net);
    if( count <= sock_bound.bufct ) {
      ct = count;
      while( (n=read(s->fd, &msg[count - ct], ct )) < ct && n>0 )
        ct -= n;
    } else
      (*bs->error)( "read count out of sync\n", 0);
  }

  *l = count;
  if( n<0 )
    (*bs->error)("read_sock, read",0);

  if( n<=0 || count > sock_bound.bufct ) {
    if( bs->close )
      (*bs->close)(s->fd);
    close(s->fd);
    s->fd = -1;
    return(-2);
  }
  msg[count] = '\0';
  last_conn = (int)s;
  return(s->fd);
}

sock_close(name)
char *name;
{
  struct SOCK *s, *p;

  if(name) { /* close name and de-queue */
    s = bs->head;
    p = NULL;
    while(s) {
      if( s->dest && strcmp( name, s->dest->dname )==0 )
        break;
      p = s;
      s = s->next;
    }
    if( s ) {
      close(s->fd);
      if( p )
        p->next = s->next;
      else
        bs->head = NULL;
      free(s);
    }
   } else { /* close all */
    s = bs->head;
    while(s) {
      close( s->fd );
      p = s->next;
      free(s);
      s = p;
    }
    close(bs->bind_fd);
    bs->bind_fd = -1;
    bs->dest = NULL;
  }
}

last_msg() { return(last_conn); }

char *sock_name(handle)
struct SOCK *handle;
{
  return( handle ? handle->dest->dname: NULL );
}

int sock_fd(handle)
struct SOCK *handle;
{
  return( handle ? handle->fd: -1 );
}


int sock_bufct(bufct)
int bufct;
{
  if(bufct)
    sock_bound.bufct = bufct;
  return(bufct);
}


int sock_find(mbname)
char *mbname;
{
struct SOCK *s;
  s = bs->head;
  while(s) {
    if( s->dest && strcmp( mbname, s->dest->dname )==0 )
      return((int)s);
    s = s->next;
  }
  return(0);
}

int sock_findfd(fd)
int fd;
{
  struct SOCK *s;
  s = bs->head;
  while(s) {
    if( s->dest && s->fd == fd )
      return((int)s);
    s = s->next;
  }
  return(0);
}

sock_intr(flag) { sock_bound.interrupt = flag; }

/*
   blocks for a message on all bound sockets
     p is a pointer to an array of additional sockets to block on.
     n is the length of p.
      returns -1 on timeout.
              or the socket number on success
     timeout is in seconds
     the size of message is up to the caller
     returns without reading when the 'p' descriptors are detected
*/

sock_sel(message, l, p, n, tim, rd_in)
char *message;
int *l;
int *p;
int n, tim, rd_in;
{
  return sock_ssel(NULL, 0, message, l, p, n, tim, rd_in);
}

/*
   blocks for a message on all bound sockets except those listed in ss.
     ss is a pointer to an array of sockets (sock_connect return values)
     ns is the length of ss
     p is a pointer to an array of additional sockets to block on.
     n is the length of p.
      returns -1 on timeout.
              or the socket number on success
     timeout is in seconds; if <0, does a poll only (returns 0 if nothing ready)
     the size of message is up to the caller
     returns without reading when the 'p' descriptors are detected
*/

sock_ssel(ss, ns, message, l, p, n, tim, rd_in)
struct SOCK *ss[];
int ns;
char *message;
int *l;
int *p;
int n, tim, rd_in;
{
  fd_set rfd;
  int sel, ret, i;
  static struct timeval timeout;
  static int rd_dead = 0;
  struct timeval *pt;
  struct SOCK *s;

  if( !message && tim >= 0)
    return(-3);

  if( bs->isxview ) {
   (*bs->error)( "Can't call sock_ssel if xview enabled", 0);
   return(-3);
  }

  while(1) {

    FD_ZERO(&rfd);
    if( rd_in && !rd_dead )
      FD_SET(0, &rfd );

    if(bs->bind_fd > 0 )
      FD_SET(bs->bind_fd, &rfd );

    for(s=bs->head; s; s = s->next ) {
      if( s->fd > 0 ) {
        for(i=0; i < ns && ss[i] != s; i++ )
	  continue;
	if (i >= ns)
          FD_SET( s->fd, &rfd );
      }
    }
    if( p && n > 0 )
      for(i=0; i<n; i++ )
        FD_SET( p[i], &rfd );

    if(tim > 0 ) {
      if( tim<10000 ) {
        timeout.tv_sec = tim;
        timeout.tv_usec = 0; 
      }else {
        timeout.tv_sec = 0;
        timeout.tv_usec = tim; 
      }
      pt = &timeout;
    } else if( tim == 0 )
      pt = NULL;
    else {
      timeout.tv_sec = 0;		/* Do a poll only */
      timeout.tv_usec = 0; 
      pt = &timeout;
    }
    sel = select( FD_SETSIZE, &rfd, NULL, NULL, pt );

    if(sel < 0 ) {
      if( errno != EINTR)
        (*bs->error)("select",0);
      else if( bs->interrupt )
        return(-3);
      continue;
    }
    if (tim < 0)
    	return sel;			/* Was only polling */

    if( sel == 0 )
      return(-1);

    if( bs->bind_fd > 0 && FD_ISSET(bs->bind_fd, &rfd ))
      accept_sock();

    if( p && n > 0 )
      for(i=0; i<n; i++ )
        if(FD_ISSET( p[i], &rfd ))
          return(p[i]);

    for(s=bs->head; s; s = s->next )
      if( s->fd > 0 ) 
        if( FD_ISSET( s->fd, &rfd ))
          if((sel = read_sock(s, message, l))>0 )
            return(sel);

    if(rd_in && ! rd_dead && FD_ISSET(0, &rfd )) {
      last_conn = (int)&sock_kbd;
      if( (i=read(0, message, 256 ))<=0) {
          rd_dead = 1;
          return(-2);
      }
      *l = i;
      message[i] = '\0';
      return(0);
    }
  }
}

sock_fastsel(message, l, p, n, tim, rd_in)
char *message;
int *l;
int *p;
int n; 
struct timeval *tim;
int rd_in;
{
  fd_set rfd;
  int sel, ret, i;
  static int rd_dead = 0;
  struct SOCK *s;

  if( !message )
    return(-3);

  if( bs->isxview ) {
   (*bs->error)( "Can't call sock_ssel if xview enabled", 0);
   return(-3);
  }

  while(1) {

    FD_ZERO(&rfd);
    if( rd_in && !rd_dead )
      FD_SET(0, &rfd );

    if(bs->bind_fd > 0 )
      FD_SET(bs->bind_fd, &rfd );

    for(s=bs->head; s; s = s->next ) {
      if( s->fd > 0 ) {
        FD_SET( s->fd, &rfd );
      }
    }
    if( p && n > 0 )
      for(i=0; i<n; i++ )
        FD_SET( p[i], &rfd );

    sel = select( FD_SETSIZE, &rfd, NULL, NULL, tim );

    if(sel < 0 ) {
      if( errno != EINTR)
        (*bs->error)("select",0);
      else if( bs->interrupt )
        return(-3);
      continue;
    }

    if( sel == 0 )
      return(-1);

    if( bs->bind_fd > 0 && FD_ISSET(bs->bind_fd, &rfd ))
      accept_sock();

    if( p && n > 0 )
      for(i=0; i<n; i++ )
        if(FD_ISSET( p[i], &rfd ))
          return(p[i]);

    for(s=bs->head; s; s = s->next )
      if( s->fd > 0 ) 
        if( FD_ISSET( s->fd, &rfd ))
          if((sel = read_sock(s, message, l))>0 )
            return(sel);

    if(rd_in && ! rd_dead && FD_ISSET(0, &rfd )) {
      last_conn = (int)&sock_kbd;
      if( (i=read(0, message, 256 ))<=0) {
          rd_dead = 1;
          return(-2);
      }
      *l = i;
      message[i] = '\0';
      return(0);
    }
  }
}

sock_only(handle, message, tim )
struct SOCK *handle;
char *message;
int tim;
{
  fd_set rfd;
  int sel, ret, i, len;
  static struct timeval timeout;
  struct timeval *pt;

  if( !handle )
    return(-3);

  if( bs->isxview ) {
   (*bs->error)( "Can't call sock_only if xview enabled", 0);
   return(-3);
  }

  while(1) {

    FD_ZERO(&rfd);
    if( bs->bind_fd > 0 )
      FD_SET(bs->bind_fd, &rfd );

    if( handle->fd > 0 )
      FD_SET( handle->fd, &rfd );

    if(tim > 0 ) {
      if( tim<10000 ) {
        timeout.tv_sec = tim;
        timeout.tv_usec = 0; 
      }else {
        timeout.tv_sec = 0;
        timeout.tv_usec = tim; 
      }
       pt = &timeout;
    } else if( tim == 0 )
      pt = NULL;
    else
      pt = &timeout;

    if( (sel = select( FD_SETSIZE, &rfd, NULL, NULL, pt )) <0 ) {
      if( errno == EINTR )
        return(-3);
      (*bs->error)("select",0);
      continue;
    }

    if( sel == 0 )
      return(-1);

    if( bs->bind_fd > 0 && FD_ISSET(bs->bind_fd, &rfd ))
      accept_sock();

    if( handle->fd > 0 )
      if( FD_ISSET( handle->fd, &rfd ))
        if((sel = read_sock(handle, message, &len))>0 )
          return(sel);
  }
}

sock_poll(handle, message, plen )
struct SOCK *handle;
char *message;
int *plen;
{
  fd_set rfd;
  int sel, ret, i;
  static struct timeval timeout;
  struct timeval *pt;

  *plen = 0;
  if( !handle )
    return(-3);

  if( bs->isxview ) {
   (*bs->error)( "Can't call sock_only if xview enabled",0);
   return(-3);
  }

  while(1) {

    FD_ZERO(&rfd);
    if( bs->bind_fd > 0 )
      FD_SET(bs->bind_fd, &rfd );

    if( handle->fd > 0 )
      FD_SET( handle->fd, &rfd );

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    pt = &timeout;

    if( (sel = select( FD_SETSIZE, &rfd, NULL, NULL, pt )) <0 ) {
      if( errno == EINTR )
        return(-3);
      (*bs->error)("select",0);
      continue;
    }

    if( sel == 0 )
      return(-1);

    if( bs->bind_fd > 0 && FD_ISSET(bs->bind_fd, &rfd ))
      accept_sock();

    if( handle->fd > 0 )
      if( FD_ISSET( handle->fd, &rfd ))
        if((sel = read_sock(handle, message, plen))>0 )
          return(sel);
  }
}

int accept_sock()
{
  int fd, len;
  char dname[4096];
  struct SOCK sock, *s;
  unsigned long count, ct, net;
  int n;

  if((sock.fd = accept( bs->bind_fd, NULL, 0 ))<0) {
    (*bs->error)("accept_sock, accept",0);
    return(-1);
  }
  
/* can't use read_sock because I want a timeout on read */

  if( (n=readtm(sock.fd, &net, sizeof(unsigned long) )) >0 ) {
    count = ntohl(net);
    if( count <= bs->bufct ) {
      ct = count;
      while( (n=readtm(sock.fd, &dname[count - ct], ct )) < ct && n>0 )
        ct -= n;
    } else
      (*bs->error)( "read count out of sync\n",0);
  }

  if( n<0 )
    (*bs->error)("read_sock, read",0);

  if( n<=0 || count > bs->bufct ) {
    close(sock.fd);
    sock.fd = -1;
    return(-1);
  }
  dname[count] = '\0';

  if( (s = (struct SOCK *)sock_connect(dname)) == 0 )
    s = (struct SOCK *)sock_connect("ANONYMOUS"); 

  if( s->fd >= 0 ) {
    if( bs->close )
      (*bs->close)(s->fd);
    close(s->fd);
  }

  s->fd = sock.fd; 
  if( bs->open )
    (*bs->open)(s, s->fd);

  return(sock.fd);
}

/* just like read with a 5 second timeout */

static readtm( fd, buf, len )
int fd;
char *buf;
int len;
{
  fd_set rfd;
  int sel;
  struct timeval timeout;

  FD_ZERO(&rfd);
  FD_SET( fd, &rfd );

  timeout.tv_sec = 5;
  timeout.tv_usec = 0; 

  if( (sel = select( FD_SETSIZE, &rfd, NULL, NULL, &timeout )) <0 ) {
    (*bs->error)("select",0);
    return(-3);
  }

  if( sel == 0 )
    return(-1);

  return(read( fd, buf, len ));
}


sock_send( s, message )
struct SOCK *s;
char *message;
{
  return( sock_write( s, message, strlen(message)));
}

/*
  like sock_send but takes pointer and count as in write
*/

sock_write( s, message, l )
struct SOCK *s;
char *message;
int l;
{
  int ret, err, len, flag = 0;
  unsigned long count;
  unsigned long net;
  char *name;

  if( (int)s == -1 || !message || l <= 0 )
    return(0);

  if( !s ) {
    (*bs->error)("no mailbox for: %s", message );
    return(0);
  }

  while( flag < 2 ) {
    if( s->fd < 0 ) {
      if( strcmp( s->dest->dname, "ANONYMOUS") == 0 )
        return(0);

      if( (s->fd = socket(s->sin.sin_family, SOCK_STREAM, 0 )) <0) {

	if(s->dest->dname) {
          (*bs->error)( "sock_write, socket(%s)", s->dest->dname);
	}
	else
          (*bs->error)( "sock_write, socket",0);

        return(0);
      }
      if( bs->open )
        (*bs->open)( s, s->fd );
      bind_any(s->fd);

      if( connect( s->fd, (struct sockaddr *)&s->sin, sizeof(struct sockaddr_in) ) <0 ) {
        if( bs->close )
          (*bs->close)(s->fd);
        close(s->fd);
        s->fd = -1;
	if(s->dest->dname)
          (*bs->error)( "sock_write, connect(%s)", s->dest->dname);
	else
          (*bs->error)( "sock_write, connect",0);

        return(0);
      }

      if( bs->bind_fd >0 ) {
        count = strlen(bs->dest->dname);
        name =  bs->dest->dname;
      } else { /* if he never bound then send ANONYMOUS */
        count = 9;
        name = "ANONYMOUS";
      }
      net = htonl(count);
      if( (ret = write( s->fd, &net, sizeof(unsigned int)))>0 )
        ret = write( s->fd, name, count);
    }

    count = l;
    net = htonl(count);
    if( (ret = write( s->fd, &net, sizeof(unsigned int)))>0 )
      ret = write( s->fd, message, count);

/*
   this checks to see if vxworks box (or sun) has rebooted 
*/
    usleep(1);
    len = 4;
    err = 0;
    if( getsockopt( s->fd, SOL_SOCKET, SO_ERROR, (char *)&err, &len ) < 0 || err != 0 )
      ret = count + 1;

    if( ret != count ) {
      if(ret <0 )
      {
	if(s->dest->dname)
          (*bs->error)( "sock_write(%s)", s->dest->dname);
	else
          (*bs->error)("sock_write", 0);
      }
      (*bs->error)("closing out mailbox: %s", s->dest->dname );
      if( bs->close )
        (*bs->close)(s->fd);
      close( s->fd);
      s->fd = -1;
      flag += 1;
   } else
      break;
  }
  return(1);
}


struct DEST *find_dest(name)
char *name;
{
  int i;

  if( destflag == 0 )
    init_dest();

  for( i=0; i<MAXDEST; i++ )
    if( mailboxes[i].dname[0] == '\0' )
      break;
    else if( strcmp( mailboxes[i].dname, name ) == 0 )
      return( &mailboxes[i] );

  for( i=0; i<MAXDEST; i++ )
    if( def_mailboxes[i].dname[0] == '\0' )
      break;
    else if( strcmp( def_mailboxes[i].dname, name ) == 0 )
      return( &def_mailboxes[i] );

  return(NULL);
}

struct DEST *find_port(p)
int p;
{
  int i;

  if( destflag == 0 )
    init_dest();

  for( i=0; i<MAXDEST; i++ )
    if( mailboxes[i].dname[0] == '\0' )
      break;
    else if( mailboxes[i].dport == p )
      return( &mailboxes[i] );

  for( i=0; i<MAXDEST; i++ )
    if( def_mailboxes[i].dname[0] == '\0' )
      break;
    else if( def_mailboxes[i].dport == p )
      return( &def_mailboxes[i] );

  return(NULL);
}

static init_dest()
{
  static char delim[] = ", \t\n";
  struct DEST *d;
  FILE *fd;
  int i, count;
  char buf[256], *q, *p;

  if( (p = (char *)getenv("MAILBOXDEFS")) == NULL )
    p = "./mailboxdefs";

  bzero( &mailboxes[0], sizeof(struct DEST)*MAXDEST);
  if( (fd = fopen( p, "r" ) )== NULL ) {
    destflag = 1;
    return;
  }

  count = 0;
  while( fgets(buf, 256, fd ) != NULL ) {
    if( buf[0] == '#' || buf[0] == '\n' )
      continue;
    if( (i = atoi(strtok( buf, delim ))) <= 0 )
      continue;
    mailboxes[count].dport = i;
    strncpy(mailboxes[count].host,  strtok(NULL, delim), MAXNAME );
    strncpy(mailboxes[count].dname, strtok(NULL, delim), MAXNAME );

    if( ++count >= MAXDEST ) {
      printf("too many entries in mailboxdefs\n");
      break;
    }
  }
  fclose(fd);
  destflag = 1;
}

/*
    bind before connect to avoid known OS hang bug 
*/

int bind_any(fd)
int fd;
{

#ifndef VXWORKS
  int port, ret;
  struct sockaddr_in sin;

  port = 30000;

  while(1) {
    bzero( &sin, sizeof( struct sockaddr_in ) );
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    if((ret = bind(fd, (struct sockaddr *)&sin, sizeof (sin)))>= 0)
      break;

    if( port-- < 1800 || errno != EADDRINUSE )
      break;
  }
  if( ret < 0 )
    (*bs->error)("bind_any",0);
#endif
}

#ifdef VXWORKS

/* 
  Jeff version of the unix standard call
*/

static char *strtok(s, delim)
char *s, *delim;
{
  static char *last = NULL, lastchar;
  char *d, *start, looking;

  if( s == NULL )  {
    if( last == NULL )
      return(NULL);
    s = last;
    *s = lastchar;
  }

  if( s == NULL || delim == NULL || *delim == '\0' )
    return(NULL);

  last = start = NULL;
  looking = 0;
  d = delim;
  while(*s) {
    while(*d) {
      if( *s == *d )
        break;
      d++;
    }
    if( *d ) {
      if( looking ){ /* found it */
        last = s;
        lastchar = *s;
        *s = '\0';
        break; 
      }
    } else if( start == NULL ) {
      start = s; 
      looking = 1;
    }
    
    d = delim;
    s++;
  }

  return(start); 
}

#endif

sock_openclose( open, close )
int (*open)(struct SOCK *,  int);
int (*close)(int);
{
  bs->open = open;
  bs->close = close;
}

sock_seterror( err )
void (*err)(char *,  char * );
{
   bs->error = err; 
}

void standard_error( a, b )
char *a, *b;
{
  fprintf(stderr, a, b );
}


