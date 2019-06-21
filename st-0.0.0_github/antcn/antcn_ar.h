/*------------------------------------------------------------
 * Include for Phoenix Socket Link
 *------------------------------------------------------------*/



#ifndef YES
#define YES 1
#define NO  0
#endif

int ar_antcn_radec_B1950(char *, double, double );
int ar_antcn_radec_J2000(char *, double, double );
int ar_antcn_radec_now(char *, double, double );
int ar_antcn_radec_offset(char *, char *, double, double, double, double );

int ar_antcn_init(void);
int ar_antcn_onsource_elog(void);
int ar_antcn_direct(char *,char *,int);
int ar_antcn_onsource_noelog(void);
int ar_antcn_onsource_extra(void);

int ar_timeout_enable(void);
int ar_timeout_disable(void);
