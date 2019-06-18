#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "../include/stparams.h"
#include "../include/stcom.h"
#include "../include/stm_addr.h"
#include "../../fs/include/params.h"
#include "../../fs/include/fs_types.h"
#include "../../fs/include/shm_addr.h"
#include "../../fs/include/fscom.h"

extern struct stcom *st;
extern struct fscom *fs;

#define MAX_OUT 256
//Beware: may overflow screen on 16-channel single pol, 16-channels
void rdbe_tsys(command,itask,ip)
struct cmd_ds *command;
long ip[5];
int itask;
{
   int i ,ierr;
   int ich,itk,tkno,ifno;
   int nactive_0,nactive_1,nddc;
   char output[MAX_OUT];
   char tsysch[220];
   double pon,poff,ratio,tsys;
   float ftsys;
   int dbe_num,iif;
//==========================bookkeep===============
//===================================
   strcpy(tsysch,"");
//===================================
   dbe_num=0; iif=0; //dbe 0,if0 default
   if(command->argv [0] !=NULL ){
     strcpy(tsysch,command->argv[0]);
     strcat(tsysch,",");
     if(strncmp(command->argv [0],"0",1) == 0)dbe_num=0;  //dbe 0
     if(strncmp(command->argv [0],"1",1) == 0)dbe_num=1;  //dbe 1
     if(strncmp(command->argv [0],"dbe0",4) == 0)dbe_num=0;  //dbe 0
     if(strncmp(command->argv [0],"dbe1",4) == 0)dbe_num=1;  //dbe 0
     if(command->argv [1] !=NULL ){
       strcat(tsysch,command->argv[1]);
       strcat(tsysch,",");
       if(strncmp(command->argv [1],"if0",3) == 0)iif=0;  //if 0
       if(strncmp(command->argv [1],"if1",3) == 0)iif=1;  //if 1
     }
   } 
//===================================
//======look which dbes, ifs allocated to abcd=====
//=================================================
//printf("DBtsys dbe, if=%d %d\n",dbe_num,iif);
//now for both dbe#s, given as 0 and 1
   nactive_0=0; nactive_1=0;
   for(itk=0;itk<16;itk++){ 
       if(dbe_num == 0){
         if(stm_addr->rdbe0_if0_caloff[itk] > 0.0)nactive_0++;
         if(stm_addr->rdbe0_if1_caloff[itk] > 0.0)nactive_1++;
//printf("DB0 if0 itk %d cal %f nac 01=%d %d : rdbe0_if %d\n",itk,stm_addr->rdbe0_if0_caloff[itk],nactive_0,nactive_1,stm_addr->rdbe0_if[itk]  );
//printf("DB0 if1 itk %d cal %f nac 01=%d %d : rdbe0_if %d\n",itk,stm_addr->rdbe0_if1_caloff[itk],nactive_0,nactive_1,stm_addr->rdbe0_if[itk]  );
       } else {
         if(stm_addr->rdbe1_if0_caloff[itk] > 0.0)nactive_0++;
         if(stm_addr->rdbe1_if1_caloff[itk] > 0.0)nactive_1++;
//printf("DB1 if0 itk %d cal %f nac 01=%d %d : rdbe1_if %d\n",itk,stm_addr->rdbe1_if0_caloff[itk],nactive_0,nactive_1,stm_addr->rdbe1_if[itk]  );
//printf("DB1 if1 itk %d cal %f nac 01=%d %d : rdbe1_if %d\n",itk,stm_addr->rdbe1_if1_caloff[itk],nactive_0,nactive_1,stm_addr->rdbe1_if[itk]  );
       }
   }
//in case of DDC, nactive_0 should=4 , _1=0. For PFB, both are 16
   nddc=0; if((nactive_0 <16 ) && (nactive_1 ==0))nddc=1;
//printf("DB dben=%d  nactive=%d %d: %d\n",dbe_num,nactive_0,nactive_1,nddc);
//===============================================================
   if(nddc ==0){               //This is PFB case
     for(itk=0;itk<16;itk++){ 
       if(iif==0){
         if(dbe_num ==0)poff=stm_addr->rdbe0_if0_caloff[itk];
         if(dbe_num ==0)pon =stm_addr->rdbe0_if0_calon [itk];
         if(dbe_num ==1)poff=stm_addr->rdbe1_if0_caloff[itk];
         if(dbe_num ==1)pon =stm_addr->rdbe1_if0_calon [itk];
       } else {
         if(dbe_num ==0)poff=stm_addr->rdbe0_if1_caloff[itk];
         if(dbe_num ==0)pon =stm_addr->rdbe0_if1_calon [itk];
         if(dbe_num ==1)poff=stm_addr->rdbe1_if1_caloff[itk];
         if(dbe_num ==1)pon =stm_addr->rdbe1_if1_calon [itk];
       }
       ratio=-2.0;
       if(pon > poff)ratio=poff/(pon-poff); //this is what you want
       if(pon < poff)ratio=poff/(poff-pon); //but cal phases may be other way round...
//what channel does this tk correspond to?
       for(i=0;i<16;i++){
         if(dbe_num ==0)ifno=stm_addr->rdbe0_if[i];
         if(dbe_num ==0)tkno=stm_addr->rdbe0_tk[i];
         if(dbe_num ==1)ifno=stm_addr->rdbe1_if[i];
         if(dbe_num ==1)tkno=stm_addr->rdbe1_tk[i];
//printf("IFNO IIF TKNO ITK=%d %d %d %d",ifno,iif,tkno,itk);
         if((ifno == iif) && (tkno == itk)){
//printf(" OK");
           if(dbe_num ==0)tsys=stm_addr->rdbe0_tcal[i]*ratio;
           if(dbe_num ==1)tsys=stm_addr->rdbe1_tcal[i]*ratio;
           ftsys=tsys;                        //just in case...
           int2str(tsysch,itk,2);
           strcat(tsysch,",");
           flt2str(tsysch,ftsys,8,1);
           strcat(tsysch,",");
         }
//printf("\n");
       }
     }
//===============================================================
   } else {    //DDC case
//===============================================================
//NBNBNB this nactive+1 is a hilfskonstruktion, should be fixed XXXXX
//for DDC, all the 4 channels have been stuffed in the if0 slots...
//     for(i=0;i<(nactive_0+1);i++){ 
// NASTY FIX NEXT LINE
     for(i=0;i<8;i++){ 
       if(dbe_num ==0)poff=stm_addr->rdbe0_if0_caloff[i];
       if(dbe_num ==0)pon =stm_addr->rdbe0_if0_calon [i];
       if(dbe_num ==1)poff=stm_addr->rdbe1_if0_caloff[i];
       if(dbe_num ==1)pon =stm_addr->rdbe1_if0_calon [i];
       ratio=-2.0;
       if(pon > poff)ratio=poff/(pon-poff); 
       if(pon < poff)ratio=poff/(poff-pon); 
//printf("DBDDC i ifno iif =%d %d %d\n",i,ifno,iif);
       ifno=stm_addr->rdbe0_if[i];
       if(dbe_num ==0)ifno=stm_addr->rdbe0_if[i];
       if(dbe_num ==1)ifno=stm_addr->rdbe1_if[i];
       if(ifno == iif) {
         if(dbe_num ==0)tsys=stm_addr->rdbe0_tcal[i]*ratio;
         if(dbe_num ==1)tsys=stm_addr->rdbe1_tcal[i]*ratio;
//printf("XXXXX %d %d %fi %f\n",i,ifno,tsys=stm_addr->rdbe0_tcal[i],tsys=stm_addr->rdbe1_tcal[i]);
         ftsys=tsys;   
         int2str(tsysch,i,2);
         strcat(tsysch,",");
         flt2str(tsysch,ftsys,8,1);
         strcat(tsysch,",");
       }
     }
   }
//===============================================================
   strcpy(output,command->name);
   strcat(output,"/");
   i=strlen(tsysch);
//   i=75; //TD051 Problem
   strncat(output,tsysch,(i-1));//cut off last comma
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
////-----List of station variables for reference--------------
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