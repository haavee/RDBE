#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <sys/types.h>    // Needed for system defined identifiers.
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"

extern struct stcom *st;
extern struct fscom *fs;

#define COMMAND_LEN 200

// make list of rdbe channels, freqs, polarisations and find Tcal from rxgain list
// now for dbe0 and 1..
void rdbe_fq(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
//input is zB    rdbef01=dbe0,if0,8512,32,lsb,r,4
//                       arg0 1   2     3  4  5 6 
{
    char data[180];  
    char *ptok;
    char output[200];
    char psb[4],fname[4];
    int i,ir,j,k,i1,i2,ifound,ierr,idummy;
    int ib,ibm1; //number 1-16 of our channel, also 0-15 same
    static int ibm10,ibm11; //0-15 for PFB or 0-3,0-3 for dbe0/dbe1 in DDC
    int iif; //if number 0 or 1
    int itk; //track number
    int dbe_num; //dbe number
    float tcal1,tcal2;
    char pol; //r or l 
    double rfq; //rf frequency
    float bw; //band width
    double fwant,f1,f2,f1f,f2f;
    float tcal_wanted;

    ibm1=itask-60;   //which of 16 channels (560 is code for rdbef01)
    ib=ibm1+1;
//as rdbef01 will always be called, presumably first, use this to clear chan counter
    if(ibm1 ==0){
          ibm10=-1;ibm11=-1;
    }
    if( sscanf(command->argv[0], "dbe%d", &dbe_num)==0 )
        sscanf(command->argv[0], "%d", &dbe_num);
/*
    if(strncmp(command->argv [0],"0",1) == 0){dbe_num=0;}  //dbe 0
    if(strncmp(command->argv [0],"1",1) == 0){dbe_num=1;}  //dbe 1
    if(strncmp(command->argv [0],"dbe0",4) == 0){dbe_num=0;}  //dbe 0
    if(strncmp(command->argv [0],"dbe1",4) == 0){dbe_num=1;}  //dbe 0
*/

/*make a DDC number*/
//    printf("DBGFQIBM1 dbe_num=%d ibm1/10/11 %d %d %d\n",dbe_num,ibm1,ibm10,ibm11);
    if(dbe_num == 0 ){
      ibm10++;   
      if(ibm10 <0)ibm10=0; /*just in case*/
      if(ibm10 >15)ibm10=15; 
    }
    if(dbe_num == 1 ){
      ibm11++;   
      if(ibm11 <0)ibm11=0; 
      if(ibm11 >15)ibm11=15; 
    }
//    printf("DBGFQIBM2 dbe_num=%d ibm1/10/11 %d %d %d\n",dbe_num,ibm1,ibm10,ibm11);

//    if(strncmp(command->argv[1],"if0",3) == 0){ iif=0; } else { iif=1; }
    sscanf(command->argv[1],"if%d",&iif);
    sscanf(command->argv[2],"%lf",&rfq);
    sscanf(command->argv[3],"%f",&bw);
//    if(strncmp(command->argv[5],"r",1) == 0){pol='r';} else {pol='l';}
    sscanf(command->argv[5],"%c", &pol);
    sscanf(command->argv[6],"%d",&itk);
    if(strncmp(command->argv[4],"lsb",3) == 0){
      if(dbe_num == 0)stm_addr->rdbe0_chanfreq[ibm10]=rfq-bw/2.0;
      if(dbe_num == 1)stm_addr->rdbe1_chanfreq[ibm11]=rfq-bw/2.0;
      strcpy(psb,"l");
    } else {
      if(dbe_num == 0)stm_addr->rdbe0_chanfreq[ibm10]=rfq+bw/2.0;
      if(dbe_num == 1)stm_addr->rdbe1_chanfreq[ibm11]=rfq+bw/2.0;
      strcpy(psb,"u");
    }
    if(dbe_num == 0){
      stm_addr->rdbe0_tk[ibm10]=itk;
      stm_addr->rdbe0_if[ibm10]=iif;
      stm_addr->rdbe0_pol[ibm10]=pol;
      fwant=stm_addr->rdbe0_chanfreq[ibm10];
//printf("FQQQ0 %d %d %d %c %f\n",ibm10,itk,iif,pol,fwant); 
    }
    if(dbe_num == 1){
      stm_addr->rdbe1_tk[ibm11]=itk;
      stm_addr->rdbe1_if[ibm11]=iif;
      stm_addr->rdbe1_pol[ibm11]=pol;
      fwant=stm_addr->rdbe1_chanfreq[ibm11];
//printf("FQQQ1 %d %d %d %c %f\n",ibm11,itk,iif,pol,fwant); 
    }
//    printf("DBFQ added tk,if,pol=%d %d %c\n",stm_addr->rdbe0_tk[ibm1],stm_addr->rdbe0_if[ibm1],stm_addr->rdbe0_pol[ibm1]);//DEB
//-----------------------------------------go looking for tcal-----------
    for(i=0;i<MAX_RXGAIN;i++) {
//    printf("DEBFQtcal rx=%d, entries=%d\n",i,shm_addr->rxgain[i].tcal_ntable);
      ir=shm_addr->rxgain[i].tcal_ntable;
      ifound=0;
      if(ir>0){
        i1=0; i2=ir;
        for(j=0;j<(ir-1);j++){
          if(shm_addr->rxgain[i].tcal[j].pol == pol){
             f1=shm_addr->rxgain[i].tcal[j].freq;
             f2=shm_addr->rxgain[i].tcal[j+1].freq;
             if((fwant >= f1) && (fwant <  f2)){
               i1=j;i2=j+1;ifound=1;
               tcal1=shm_addr->rxgain[i].tcal[i1].tcal;
               tcal2=shm_addr->rxgain[i].tcal[i2].tcal;
               f1f=f1; f2f=f2;
             }
          }
        }   
        tcal_wanted=tcal1+(tcal2-tcal1)*(fwant-f1f)/(f2f-f1f);
//        if(ifound == 1)printf("DBEFq found %d %d %f %f %f %f : %f\n",i1,i2,f1f,f2f,tcal1,tcal2,tcal_wanted);
        if(ifound == 1){
//printf("DBEFQfound %d %d %d %f %f\n",dbe_num,ibm10,ibm11,fwant,tcal_wanted);
           if(dbe_num == 0)stm_addr->rdbe0_tcal[ibm10]=tcal_wanted;
           if(dbe_num == 1)stm_addr->rdbe1_tcal[ibm11]=tcal_wanted;
        }
      }   
    }   
//-----------------------------------------------------------------------
    if(dbe_num == 0)sprintf(data,"midf=%f,bw=%2.0f,sb=%s,if=%d,tk=%d,pol=%c,tcal=%f",stm_addr->rdbe0_chanfreq[ibm10],
      bw,psb,iif, itk, pol, stm_addr->rdbe0_tcal[ibm10]);
    if(dbe_num == 1)sprintf(data,"midf=%f,bw=%2.0f,sb=%s,if=%d,tk=%d,pol=%c,tcal=%f",stm_addr->rdbe1_chanfreq[ibm11],
      bw,psb,iif, itk, pol, stm_addr->rdbe1_tcal[ibm11]);
    sprintf(fname,"%02d",ib);
    strcpy(output,"rdbef");
    strncat(output,fname,2);
    strcat(output,"/");
    strcat (output,data);
    for (i=0;i<5;i++)ip[i]=0;
    cls_snd(&ip[0],output,strlen(output),0,0);
    ip[1]=1;
    return;
error:
    ip[0]=0;
    ip[1]=0;
    ip[2]=ierr;
    memcpy(ip+3,"rd",2);
    return;
}
////---------------------------d-------
//  int  rdbe0_if [32]; //only need 16 really...
//  int  rdbe0_tk [32];
//  float rdbe0_chanfreq [32]; //read in if, tkno, chan freq, poln 
//  char rdbe0_pol [32];
//  float rdbe0_tcal[32];
//----------------
//  double rdbe0_if0_caloff[32]; //Tsys readout from RDBE
//  double rdbe0_if0_calon[32];
//  double rdbe0_if1_caloff[32];
//  double rdbe0_if1_calon[32];
//----------------
