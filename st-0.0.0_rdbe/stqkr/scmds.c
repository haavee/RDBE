#include <signal.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../../fs/include/dpi.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/fscom.h"

extern struct fscom *shm_addr;

#include "sample_ds.h"

void scmds(mess)
     char *mess;
{
  long ip[5];
//  printf("DBSCCMDS run procedure >%s<\n",mess);
  cls_snd( &(shm_addr->iclopr), mess, strlen(mess) , 0, 0);
  skd_run("boss ",'n',ip);

  return;
}
