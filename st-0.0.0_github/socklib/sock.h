
struct BOUND {
  int bind_fd;
  struct DEST *dest;
  struct SOCK *head; /* head of list */
  struct sockaddr_in sin;
  unsigned int bufct;
  int interrupt;
  int isxview; /* true if xview */
  int isaio;   /* true if aioread is turned on (Solaris2) */
  int (*open)(struct SOCK *, int);
  int (*close)(int);
  void (*error)(char *, char * );
};

struct SOCK {
  struct SOCK *next;
  struct DEST *dest;
  int fd;
  struct sockaddr_in sin;
};

/*
 * structure of mailboxdefs file
 */

/* every destination has a port # as defined by the mailboxdefs file */

#define MAXPORT  10 
#define MAXDEST 100
#define MAXNAME  30

struct DEST {
  int dport;
  char host[MAXNAME];
  char dname[MAXNAME];
};

struct DEST *find_dest();
struct DEST *find_port();

