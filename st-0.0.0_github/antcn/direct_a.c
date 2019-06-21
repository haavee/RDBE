/*--------------------------------------------------------------------*
 *  Direct Control of antcn_ar file
 *   This routine calles the routines of antcn_ar from the command line
 *   to test the station program.  
 *--------------------------------------------------------------------*/
 #include <stdio.h>
 #include "phoenix.h"
 
 const char *help = "\
  0                - Initialization\n\
  1 x.xxx y.yyy    - Source Command x=RA y=DEC\n\
  2 x.xxx y.yyy    - Offset Command x=RAoff y=DEC\n\
  3                - Onsource with Error Logging \n\
  4 string         - Direct Antenna Command(string)\n\
  5                - On Source Command NO error Logging\n\
  6                - RESERVED\n\
  7                - On Source Command Additional Logging\n\
  Q = Quit\n";
 
 logit(char *msg,int num, char *type)
 {
   printf("Logit:%s\n",msg);
   printf("Logit: Number %d ,Type %s\n",num,type);
 }
 
 main(int argc, char *argv[])
 {
   double ra,dec;
   char buf[256];
   char acmd[256];
   char ack[256];
   int status;
   
   do
   {
     putchar(':');
     fgets(buf,255,stdin);
     
     ar_timeout_enable();
     switch(*buf)
     {
       case '0':
         status = ar_antcn_init();
         printf("Init Status %d\n",status);
         break;
       case '1':
         sscanf(buf,"1 %lg %lg",&ra,&dec);
         status= ar_antcn_radec_now(ra,dec);
         printf("Source Status %d\n",status);
         break;
       case '2':
         sscanf(buf,"2 %lg %lg",&ra,&dec);
         status= ar_antcn_radec_offset(ra,dec);
         printf("Offset Status %d\n",status);
         break;
       case '3':
         status = ar_antcn_onsource_elog();
         printf("On Source No Error Log Status %d\n",status);
         break;
       case '4':
         sscanf(buf,"4 %s",acmd);
         status = ar_antcn_direct(acmd,ack,sizeof(ack));
         printf("Antenna Direct: %d\n",status);
         break;
       case '5':
         status = ar_antcn_onsource_noelog();
         printf("OnSource with NO Error Log Status %d\n",status); 
         break;
       case '6':
         printf("RESERVED\n");
         break; 
       case '7':
         status = ar_antcn_onsource_extra();
         printf("OnSource with EXTRA Error Log Status %d\n",status);
         break;
       default:
         printf("Command NO Recognized\n",help);
         puts(help);
     }
     ar_timeout_disable();
   }
   while(*buf != 'Q');
 }