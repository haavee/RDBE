/* stcom.h - dummy
 */

typedef struct stcom {
  int dummy;              /* just a dummy */
  int ieffcal;            /* 0 for frontend cal off, 1 for cal on */
  int ieffpcal;           /* 0 for phase cal off, 1 for pcal on */
  long ieffrx;            /* what receiver (now lo frequency)*/
  int rx_bits;            /* these are the 16bits sent to the SBC ,
                             top bit is cal offon */
  int antenna_sentoff;    /* if =1, antenna has just got new source command or azel offset, 
			     set to 0 by antrcv as soon as antenna starts to move and reports slewing*/
  int raw_ionsor;         /* =0 if ant off, 1 if on: cook to get ionsor special for on/off*/
  int ant_state ;          /*like ionsor in fscom, but more states: 
                            0=off and likely to remain off  (ionsor=0)
			    1=just told to move             (ionsor=0)
			    2=told to move and started motion (ionsor=0)
			    3= reached source but may still wobble off again (ionsor=1 if
			      in normal VLBI mode but but 0 if on/off or fivpt)
			    4=stable on source (ionsor=1)*/
  int ant_scanning;       /*from vaxtime, -ve if waiting, + if scanning (scanmsec)*/
  long ant_rfcentre;      /* from vaxtime broadcast: centre freq reported back from antenna,
			     should agree with value in rxlist table , if not, try to resend..*/
  double fsynth1,fsynth2; /*front end synthesizer(ULO) and 2nd LO synth in MHz*/
  double gpsdiff; /*to hold result from gps counter*/
  int ant_is_listening;   /*controlled and read by rxvt, =1 if antenna is receiving commands from 
			    VME and mk4fs, =0 if F1 pressed to disconnect from antenna*/
  char ant_command[150];  /* source,freq request string received via rpc from VME or this machine,
			     this replaces direct socket connection to eff_rxvt used previously*/
  char dbbchost[150];  /*new for bbc version*/
  int slack[128]; //
//---------------------------d-------
  int  rdbe0_if [32]; //only need 16 really...
  int  rdbe0_tk [32];
  double rdbe0_chanfreq [32]; //read in if, tkno, chan freq, poln 
  char rdbe0_pol [32];
  float rdbe0_tcal[32];
//----------------
  int  rdbe1_if [32];
  int  rdbe1_tk [32];
  double rdbe1_chanfreq [32]; 
  char rdbe1_pol [32];
  float rdbe1_tcal[32];
//----------------
  char rdbe0_if0_iflet[8]; //which of abcd is dbe0 if0 connected to
  char rdbe0_if1_iflet[8]; 
  char rdbe1_if0_iflet[8]; 
  char rdbe1_if1_iflet[8]; 
//----------------
  double rdbe0_if0_caloff[32]; //Tsys readout from RDBE
  double rdbe0_if0_calon[32];
  double rdbe0_if1_caloff[32];
  double rdbe0_if1_calon[32];
//----------------
  double rdbe1_if0_caloff[32];
  double rdbe1_if0_calon[32];
  double rdbe1_if1_caloff[32];
  double rdbe1_if1_calon[32];
//-----------------------------------
//----cfg settings stored here-----
  int dcfgdeci[8],dcfgmode[8];//dbe_cfg numbers for 8 channels (dbe0/1) 
  float dcfgfq[8];                        //frequencies
  float dxbar[8];                         //xbar numbers
//-----------------------------------
  char dbe0_add[20],dbe1_add[20],t450_add[20],essr_add[20];  //put machines here at init
  char dbe_personality[20];

} Stcom;
