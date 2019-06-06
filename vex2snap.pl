#!/usr/bin/perl
# Vex to FS for RDBE as simple as possible
$our_station="Eb";
$allow_early=0;  #=1 does early start of setup before previous scan end
$allow_late_start=0; #=1 will only drudg part of experiment is stil to come
$version="2014_nov_06";
use POSIX qw(strftime);
$date = strftime("%Y %j %H:%M:%S %z %Z", localtime(time));
print "DATE NOW=$date\n";
#=========================================$date="2013 001 23:01:00 +0000 UTC"; #test date
($yn,$dn,$hmsn,$hz,$zone)=split(' ',$date);
$dn=int($dn); $hz=int($hz/100);
($hn,$mn,$sn)=split(':',$hmsn);
#print "$dn $hn $mn $sn : $hz\n";
#inset test date in middle of exper:
#$dn=87; $hn=12; $mn=8; $sn=10; $hz=0;
#print "$dn $hn $mn $sn : $hz\n";
$secofyrnow=$dn*86400+($hn-$zn)*3600+$mn*60+$sn;  #seconds of year when we start to run script
#=========================================
#preob, collect not in
#timecheck not in, take acct of 2 machines and call it in midob, not here
#Must write stuff at start of snp
#consider whether repeat calls of setup should be out
#time drudging, i.e. if drudged withing schedule run time, always call setup
#============below this line, points may be fixed......
#perpfb etc not called
#dbeoff/on not called??
#NBNBN No pcal interpretation for pfb
#NBNB if allocation to diff dbe wrong for xd067
#Fix so clock checked at start, only necessary setup=first after start
#zap :; at end of ioch should just end with a ;
#should mark data send in initl of expt as start and end of expt
#look for correct time to send out preob, may have to split into several parts
#nb can initl tsys for dbe1, but there is no mulicast addr or recv pgm for that
#, no space allocated in stn common etc..
# check correct Verhalten on pointing scans, no recording wanted
# ALWAYS stop/start recording w stream stop as well (wg problem 4Gbps)
# BEWARE possible case of swap inside 1 schedule PFB<->DDC!!A
# essr procedures  and VDIF/5B procedures. essr should be off when modes change, so
# same time as streaming..........need to put that in snap
# now must call procedures....
# new "format" in make_proc to pass that
# .............rdbe commands to think about:-------------
#print (fip "enddef\n");
# dbe_vdif_threads_id = <thid0><thid1><thid2><thid3><thid4><thid5><thid6><thid7>
# fpga_wr=W:0x800008:0xDEAD:  # Station ID
# fpga_wr=W:0x80000A:0x000D:  # DBE num
# fpga_wr=W:0x800160:0x0200:  # ThreadID 0
# fpga_wr=W:0x800162:0x0211:  # ThreadID 1
# fpga_wr=W:0x800164:0x0222:  # ThreadID 2
# fpga_wr=W:0x800166:0x0233:  # ThreadID 3
# .........................................................
#=========================================
#chet's 'upper edge' table, modified to add 1024, leave out 528
(@chet_fh)=(1024,1008,976,944,912,880,848,816,784,752,720,688,656,624,592,560); 
open(fi,"$ARGV[0]");
$section="";$ns=0; $early_set_now=0; $early_set_next=0;
while(<fi>){
  chomp;
  s/[;=\r]/ /g;
  $orig_line=$_;
  (@ss)=split();
  if(substr($_,0,1) eq "\$"){
    s/ //g;
    $l=length($_); $section=substr($_,1,($l-1));
  }
  if(($section eq "EXPER") && ($ss[0] eq "def")){
    $exper=lc($ss[1]); 
    $lexp=length($exper);
#shorten name if necessary
    if($lexp >6){
	$local_name=substr($exper,0,1).substr($exper,2,($lexp-2));
    } else {
	$local_name=$exper;
    }
    $exper_name=$local_name;
    $local_name=$local_name.lc($our_station);
    $snp_name=$local_name.".snp";
    $prc_name=$local_name.".prc";
    $lst_name=$local_name.".lst";
    open(fis, ">$snp_name");
    open(fip, ">$prc_name");
    open(fil, ">$lst_name");
    print (fip "define  proc_library  00000000000x\n");
    print (fip "\" ",uc($exper),"      EB_RDBE2  ",$our_station,"\n");
    print (fip "\" vex2snap version $version by DG and UB, MPIfR\n");
    print (fip "\"< RDBE  rack >< Mark5C   recorder 1>\n");
    print (fip "enddef\n");
    print (fip "define  exper_initi   00000000000x  \n");
    print (fip "proc_library\n");
    print (fip "sched_initi\n");
    print (fip "enddef\n");
  }
#--------------------------------------------------------
  if($section eq "SOURCE"){
    if($ss[0] eq "source_name"){$source_def=$ss[1];}
 #ra = 00h41m41.2986000s; dec =  41d03'32.995800"; ref_coord_frame = J2000;
    if($ss[0] eq "ra"){$ra=$ss[1];}
    if($ss[2] eq "dec"){$dec=$ss[3];}
    $sgn="";if(substr($dec,0,1) eq "-"){$sgn="-";}
    $ra=~ s/[hms]//g; $dec=~ s/[-d\'\""]//g;
    $ra=substr($ra,0,9); $dec=$sgn.substr($dec,0,8);
    if($ss[4] eq "ref_coord_frame"){
       $ep=$ss[5]; 
#      print "CC $source_def $ep >$ra<>$dec>\n";
       if($ep eq "J2000"){$coords{$source_def}=$ra.",".$dec.",2000.0,"; }
    }
  }
#--------------------------------------------------------
#def VLBA_SC;
#    site_name = VLBA_SC;
#    site_ID = Sc;
#    site_position = 2607848.60460 m:-5488069.57930 m: 1932739.66740 m;
  if($section eq "SITE"){
      if($ss[0] eq "site_ID"){
         $site_shortname=$ss[1];
	 if($site_shortname eq $our_station){ $got_site=1; } else {$got_site=0;}
      }
      if(($ss[0] eq "site_position") && ($got_site == 1)){$xst=$ss[1];$yst=$ss[3];$zst=$ss[5];}
  }
#--------------------------------------------------------
  if($section eq "MODE"){
    if($ss[0] eq "def"){$mode_def=$ss[1];$nmode++;$mode_list[$nmode]=$mode_def;}
    if($ss[0] eq "ref"){
	$mode_sect=$ss[1]; 
	$mode_sect=~ s/\$//; 
	(@stns)=split(':',$ss[2]);
	$mode_ref=$stns[0];
#e.g.:geodetic#FREQ#22050.00MHz4x32MHz
	for($ist=1;$ist<($#stns+1);$ist++){
          if($stns[$ist] eq "$our_station"){
          if($mode_sect eq "FREQ"){$freq_wanted{$mode_def}=$mode_ref;}
          if($mode_sect eq "IF"){$if_wanted{$mode_def}=$mode_ref;}
          if($mode_sect eq "BBC"){$bbc_wanted{$mode_def}=$mode_ref;}
          if($mode_sect eq "TRACKS"){$track_wanted{$mode_def}=$mode_ref;}
	  }
	}
     }
  }
#--------------------------------------------------------
  if($section eq "FREQ"){
    if($ss[0] eq "def"){$freq_def=$ss[1];}
    $key=$freq_def;
    if($ss[0] eq "chan_def"){
       $str=index($_,"\*");
       $chan_line=substr($_,0,($str-1));
       $chan_line=~s/ //g;
       $chan_line=~s/chan_def://;
       $chan_defs{$key}=$chan_defs{$key}.$chan_line."\#";
    }
  }
#--------------------------------------------------------
  if($section eq "IF"){
    if($ss[0] eq "def"){$if_def=$ss[1];}
    $key=$if_def;
    if($ss[0] eq "if_def"){
       $str=index($_,"\*");
       $if_line=$_;
       if($str != -1){$if_line=substr($_,0,($str-1));}
       $if_line=~s/ //g;
       $if_line=~s/if_def//;
       $if_defs{$key}=$if_defs{$key}.$if_line."\#";
    }
  }
#--------------------------------------------------------
  if($section eq "BBC"){
    if($ss[0] eq "def"){$bbc_def=($ss[1]);}
    $key=$bbc_def;
    if($ss[0] eq "BBC_assign"){
       $str=index($_,"\*");
       $bbc_line=$_;
       if($str != -1){$bbc_line=substr($bbc_line,0,($str-1));}
       $bbc_line=~s/ //g;
       $bbc_line=~s/BBC_assign//;
       $bbc_defs{$key}=$bbc_defs{$key}.$bbc_line."\#";
    }
  }
#--------------------------------------------------------
  if($section eq "TRACKS"){
    if($ss[0] eq "def"){$track_def=($ss[1]);}
    $key=$track_def;
    if($ss[0] eq "track_frame_format"){
       $str=index($_,"\*");
       $track_line=$_;
       if($str != -1){$track_line=substr($_,0,($str-1));}
       $track_line=~s/ //g;
       $track_line=~s/track_frame_format//;
#      print "$key>$track_line< \n";
       $track_defs{$key}=$track_defs{$key}.$track_line."\#";
    }
#Nasty: type is given as a comment!, could also deduce from number of chans...
#But that comment is not in all schedules......
#*    firmware_type = RDBE_DDC;
#   if($ss[1] eq "firmware_type"){
#      $firmware_type=$ss[2];
#   }
  }
#--------------------------------------------------------
}
#reopen and read sched section
$section="";$last_mode_def="";$netup=0;
close(fi);
open(fi,"$ARGV[0]");
while(<fi>){
  chomp;
  s/[;=\r]/ /g;
  $orig_line=$_;
  (@ss)=split();
  if(substr($_,0,1) eq "\$"){
    s/ //g;
    $l=length($_);
    $section=substr($_,1,($l-1));
  }
  if($section eq "SCHED") {
    if($ss[0] eq "scan"){$scan_number[$ns]=$ss[1]; i}
    for($i=0;$i<6;$i=$i+2){
      if($ss[$i] eq "start"){$scan_start[$ns]=$ss[$i+1];}
      if($ss[$i] eq "mode"){$scan_mode[$ns]=$ss[$i+1];}
      if($ss[$i] eq "source"){$scan_source[$ns]=$ss[$i+1];}
    }
    if(($ss[0] eq "station") && (substr($ss[1],0,2) eq $our_station)){
      $sec_to_start=$ss[2]; $sec_to_stop=$ss[4];
      $tim1[$ns]=&vextime_2_secs($scan_start[$ns]);   #nominal scan start
      $tim2[$ns]=$tim1[$ns]+$sec_to_start;       #tim ant is on source
      $tim3[$ns]=$tim1[$ns]+$sec_to_stop;        # tim scan end
      $fmt_start_time[$ns]=&secs_2_fstime($tim2[$ns]);
      if($exper_start eq ""){$exper_start=$scan_start[$ns];}
      $spare_sec1[$ns]=$tim1[$ns]-$last_stop_sec;
      $spare_sec2[$ns]=$tim2[$ns]-$last_stop_sec;
#spare_sec1 is time free for setup between scans to nom start
#spare_sec2 is time free for setup between scans to rec start
      if($spare_sec1[$ns] < 0){$spare_sec1[$ns]=$spare_sec1[$ns]+86400.0;}
      if($spare_sec2[$ns] < 0){$spare_sec2[$ns]=$spare_sec2[$ns]+86400.0;}
      $fmt_stop_time[$ns]=&secs_2_fstime($tim3[$ns]);
      $last_stop_sec=$tim3[$ns];
      $motion[$ns]=$ss[10];
      $poi=""; if($motion[$ns] == 0){$poi[$ns]="POINT";} #only for debug
#     print "SPARE $scan_number[$ns] $fmt_start_time[$ns] $fmt_stop_time[$ns] : $spare_sec1[$ns] $spare_sec2[$ns] $poi[$ns]\n";
#     print "SS2 >$scan_source< $coords{$scan_source}\n";
#     print "$fmt_start_time $fmt_stop_time $sec_to_stop $motion\n";
#station=Eb:    0 sec:  620 sec:   42.961 GB:   :       : 0;
      $ns++;
    }
  }
}
#=================================================================
$nscan=$ns;
$exper_finish=$fmt_stop_time[$ns-1];
#XXX===========================================
#===========================================================
$exper_start=~ s/[ydhms]//g;
$exper_finish=~ s/[\.:]//g;
#print "FIRSTLAST >$exper_start<>$exper_finish<\n";
#some of this probably better in station.prc, except for data_send
#but sched_initi depends on type of obs..hmmm
#==================================================
#assume there will only be one personality type in this expt and set in sched_initi
#contents of setpers will be determined later
print (fip "define  sched_initi   00000000000x\n");
print (fip "\"setpers\n");    
print (fip "\!+1s\n");
print (fip "rdbe_cmd=0,3,dbe_data_send=on:$exper_start:$exper_finish:0;\n");
print (fip "\!+1s\n");
print (fip "rdbe_cmd=0,3,dbe_1pps_mon=enable:239.0.2.34:20020;\n");
print (fip "\!+1s\n");
print (fip "rdbe_cmd=0,3,dbe_tsys_mon=enable:239.0.2.34:20021:10;\n");
print (fip "\!+1s\n");
print (fip "rdbe_cmd=0,3,dbe_dot?;\n");
print (fip "\!+1s\n");
print (fip "rdbe_time=dbe0,3\n");
print (fip "setdbe1\n");
print (fip "getgps\n");
print (fip "\!+1s\n");
print (fip "ready_disk\n");
print (fip "\!+1s\n");
print (fip "enddef\n");
#==================================================
#print header of snp file
$yy=substr($scan_start[0],0,4);
print (fis "\" ",uc($exper),"     ",$yy," EB_RDBE2 E ",$our_station,"\n");
print (fis "\" E EB_RDBE2 AZEL  .0130  25.0    9  -90.0  450.0  15.0    9    5.0   88.0   .0 Eb\n");
print (fis "\" Eb EB_RDBE2  4033947.32710   486990.70440  4900430.93840\n");
print (fis "\"      EB_RDBE2    0     8820\n");
print (fis "\" Produced by vex2snap $version\n");
print (fis "\" Rack=RDBE     Recorder 1=Mark5C    Recorder 2=none\n");
#XXX===========================================
#have collected scans, now read through them
for($ns=0; $ns<$nscan;$ns++){
# print "SCN $ns $scan_number[$ns] $scan_start[$ns] $scan_mode[$ns] $scan_source[$ns] $scan_start[$ns] $fmt_start_time[$ns] $fmt_stop_time[$ns] $motion[$ns]\n";
# ================================check if already past section===
$doscn=1; if($tim1[$ns] <$secofyrnow){$doscn=0;}
#print "DBG279 now=$secofyrnow scan_start=$tim1[$ns]  $scan_start[$ns] DOSCAN=$doscn\n";
if(($doscn == 0) && ($allow_late_start == 1)){goto skipscan;}    #IF TIME FOR SCAN IS PAST, SKIP OVER!!!!!
# ================================================================
  $tscn=$tim3[$ns]-$tim1[$ns]; #from nom start to scan end
  print (fis "scan_name=",$scan_number[$ns],",",$exper_name,",",$our_station,",",$tscn,",",$tscn,"\n");
  print (fis "source=",$scan_source[$ns],",",$coords{$scan_source[$ns]}," \n");
  if($ready_disk eq ""){$ready_disk="done"; print(fis "ready_disk \n");}
#===========================================================
# look ahead to see how much time before next scan, so new freqs can be
# for next scan set ahead of scan end, with time stamp that gets freqs
# taken over on scan end +1 sec
# (unless last scan)
# $early_execute=0; 
  if($ns < ($nscan-1)){
     if($allow_early == 1){
#      print "LAHD_SECS $spare_sec1[$ns+1] $spare_sec2[$ns+1]\n";  
       if($spare_sec1[$ns]   < 6){ $early_set_now =1; }
       if($spare_sec1[$ns+1] < 6){ $early_set_next=1; }
     }
  }
#===========================================================
  $mode_nr=0;
  for($imode=1;$imode <($nmode+1);$imode++){
    if($scan_mode[$ns] eq $mode_list[$imode]){$mode_nr=$imode;}
#   print "$imode $mode_list[$imode] $scan_mode : $mode_nr\n";  #HERE
  }
  $nsetup=$mode_nr;
  $setupname=sprintf("setup%02d",$nsetup); #force setup
# $checkname=sprintf("chkup%02d",$nsetup); #check cfg freqs, set if incorrect
# print (fis $setupname,"\n"); #snap call for this setup
# if($scan_mode[$ns] eq $last_mode_def){
#   $whatset=$checkname;
# } else {
    $whatset=$setupname;
    if($proc_done[$nsetup] != 1){
       $freq_key=$freq_wanted{$scan_mode[$ns]};
       $if_key=$if_wanted{$scan_mode[$ns]};
       $bbc_key=$bbc_wanted{$scan_mode[$ns]};
       $track_key=$track_wanted{$scan_mode[$ns]};
#      $firmware_type=~ s/\#//;
       (@track_chans)=split("\#",$track_defs{$track_key});
       (@bbc_chans)=split("\#",$bbc_defs{$bbc_key});
       (@if_chans)=split("\#",$if_defs{$if_key});
       (@fq_chans)=split("\#",$chan_defs{$freq_key});
#================summary====================================
       $chanstring="";
       for($ic=0;$ic<($#fq_chans+1);$ic++){
	  $fline=$fq_chans[$ic];
	  $fline=~ s/MHz//g;
	  ($rf,$sb,$bw,$chanref,$bbcref,$nix)=split(':',$fline);
	  $ifwant=""; $ifabcd=""; $lo1=""; $chanpol=""; $bbcn="";
	  for($ib=0;$ib<($#bbc_chans+1);$ib++){
	    ($bbckey,$bbcnum,$ifref)=split(':',$bbc_chans[$ib]);
            if($bbckey eq $bbcref){$ifwant=$ifref;$bbcn=$bbcnum;}
	  }
	  for($ii=0;$ii<($#if_chans+1);$ii++){
	    ($ifkey,$ifchan,$pol,$lofq,$sblo,$pcl)=split(':',$if_chans[$ii]);
	    $lofq=~ s/MHz//g; $pcl=~ s/MHz//g; if($pcl eq ""){$pcl="off";}
	    $pc0=substr($pcl,0,1);
            if(($pc0 eq "1") || ($pc0 eq "5")){$pcl="1"; }
	    if($ifkey eq $ifwant){
	      $ifabcd=$ifchan; $lo1=$lofq; $chanpol=$pol;
	      $losb=$sblo;
## 	      print "DBG283isPcal=>$pc0<>$pcl<\n";
	    }
	  }
	  $chanstring=$chanstring.$ic.":".$bbcn.":".$rf.":".$sb.":".$bw.":".$chanpol.":".$ifabcd.":".$lo1.":".$losb.":".$pcl."#";
       }
#      print "$chanstring\n";
#At present, must force firmware type according to number of chans
       if($#fq_chans >8){$firmware_type="PFB";}
       if($firmware_type eq ""){$firmware_type="DDC";}
       if(($firmware_type eq "DDC") &&($#fq_chans >4)){$firmware_type="DDC8";}
#      print "DBG335 firm=$firmware_type\n";
#=========forgot===================
#      print "TRACKCHAN $track_chans[0]\n";
       if(substr($track_chans[0],0,4) eq "VDIF"){
          $output_format="VDIF";
       } else {
          $output_format="MARK5B";
       }
#==================================
       if(substr($firmware_type,0,3) eq "DDC"){
          &make_fqlist($setupname,$chanstring);
	  &make_cfg_ddc($setupname,$chanstring);
	  &make_proc_ddc($setupname,$chanstring,$output_format);
       }
       if($firmware_type eq "PFB") {
          &make_fqlist($setupname,$chanstring);
	  &make_proc_pfb($setupname,$chanstring,$output_format);
       }
       $proc_done[$nsetup] = 1; #mark procedure as printed
    }
#===========================================================
#only do recording start and stop if this is not a pointing
#if 10 sec until next scan, allow checkmk5 on previous
  if(($spare_sec1[$ns] >10) && ($ns != 0)){
#     print (fis "stop_data \n");
     print (fis "checkmk5 \n");
  }
  if($motion[$ns] == 1){
     if($early_set_now == 0){
#       if($scan_mode[$ns] ne $last_mode_def){
        if($whatset ne $last_whatset){
           print (fis $whatset," \n"); #snap call for this setup(move outside loop to force setup each time)
#	   print "DBG385 $whatset\n";
#   $last_mode_def=$scan_mode[$ns];
 	   $last_whatset=$whatset;
        }
     }
#If 10 sec to spare before nominal start(sec1 until tim1)  ,
#    phaps unitil real start(sec2 until tim2)?? allow preob
     if(($spare_sec1[$ns] > 10)){
        $preob_time=&secs_2_fstime($tim1[$ns]-10); #10 sec before nom start
	print(fis "\!",$preob_time," \n");
	print(fis "preob \n");
#        $startdata_time=&secs_2_fstime($tim1[$ns]-4); #10 sec before nom start
#	print(fis "\!",$startdata_time," \n");
#	print(fis "start_data \n");
     }
     print (fis "\!",$fmt_start_time[$ns]," \n");
#    print (fis "essr=run,on\n"); #soft_switch start
     print (fis "disk_pos \n");
     print (fis "disk_record=on \ndisk_record \n");
     print (fis "data_valid=on \n");
     print (fis "midob \n");
     if($early_set_next == 1){
	 $execute_time=&secs_2_fstime($tim3[$ns]-4); #4 secs before scan end
	 print(fis "\!",$execute_time,"\n");
#	 print "EARLY EXE $execute_time\n";
#        if($scan_mode[$ns] ne $last_mode_def){
         if($whatset ne $last_whatset){
           print (fis $whatset," \n"); #snap call for this setup(move outside loop to force setup each time)
#	   print "DBG405 $whatset\n";
#	   $last_mode_def=$scan_mode[$ns]
 	   $last_whatset=$whatset;
         }
     }
     print (fis "\!",$fmt_stop_time[$ns]," \n");
     print (fis "data_valid=off \n");
     print (fis "disk_record=off \n");
     print (fis "disk_pos \n");
#    print (fis "essr=run,off\n"); #soft_switch stop
     print (fis "postob \n");
  } else {
     print (fis "\!",$fmt_start_time[$ns]," \ndata_valid=on\n");
     print (fis "\!",$fmt_stop_time[$ns]," \ndata_valid=off\n");
  }
  $last_mode_def=$scan_mode[$ns];
  $early_set_now=0; $early_set_next=0; 
  skipscan:
}
print (fis "checkmk5 \n");
print (fis "sched_end \n");
#===========================================================
#put in re-init procedure for when DDC freq change needed
#use rdbe_sync command, checks sync which only takes time if sync bad
#   if sync is wrong, attempt up to 3*resync
#2012.279.13:27:42.63;rdbe_sync=dbe0
#2012.279.13:27:43.64/rdbe_sync/dbe0,sync_state=syncerr_eq_0,tries=1
##print(fip "define  dbe0ini       00000000000x\n");
##print(fip "rdbe_send=dbe0,off\n\!+2s\n");
##print(fip "rdbe_cmd=dbe0,3,dbe_execute=init\n\!+3s\n"); #takes 0.5 sec
#print(fip "rdbe_cmd=dbe0,3,dbe_dot_set=\n\!+2s\n");
#print(fip "rdbe_cmd=dbe0,3,dbe_dot?\n");
##print(fip "rdbe_sync=dbe0\n");
##print(fip "enddef\n");
##print(fip "define  dbe1ini       00000000000x\n");
##print(fip "rdbe_send=dbe1,off\n\!+2s\n");
##print(fip "rdbe_cmd=dbe1,3,dbe_execute=init\n\!+3s\n");
##print(fip "rdbe_sync=dbe1\n");
##print(fip "enddef\n");
#===========================================================
##$exper_start=~ s/[ydhms]//g;
##$exper_finish=~ s/[\.:]//g;
#print "FIRSTLAST >$exper_start<>$exper_finish<\n";
#some of this probably better in station.prc, except for data_send
#but sched_initi depends on type of obs..hmmm
#==================================================
#personality setup and Xcube commands
#assume only vdif in DDC mode
print (fip "define  setpers        00000000000x\n");
if($firmware_type eq "PFB"){
  print (fip "rdbe_cmd=dbe0,50,dbe_personality=pfbg:pfbg_1_4.bin\n");
  print (fip "rdbe_cmd=dbe0,3,dbe_execute=init\n\!+2s\n");
  print (fip "\"essr=run,off\n");     
  print (fip "\"essr=mode,4\n");     # one in, one out
  print (fip "\"essr=route,2,4\n"); # incoming traffic from input 1 to output 1
  print (fip "\"essr=pace,4,3\n");  # set port 4 pacing to 5
  print (fip "\"essr=run,on\n");     
  print (fip "mk5=fill_pattern=464374526;\n");
  print (fip "mk5=mode=mark5b:0xffffffff:1;\n");
  print (fip "mk5=packet=36:0:5008:0:0;\n");
}
#...........
if($firmware_type eq "DDC"){
  print (fip "rdbe_cmd=dbe0,50,dbe_personality=ddc:ddc_1601583.bin\n");
  print (fip "rdbe_cmd=dbe0,3,dbe_execute=init\n\!+2s\n");
  print (fip "\"essr=run,off\n");     
  print (fip "\"essr=mode,4\n");     # one in, one out
  print (fip "\"essr=route,2,4\n"); # incoming traffic from input 1 to output 1
  print (fip "\"essr=pace,4,3\n");  # set port 4 pacing to 5
  print (fip "\"essr=run,on\n");     
  print (fip "mk5=mode=vdifl_20000-512-4-2;\n");
  print (fip "mk5=fill_pattern=464374526;\n");
  print (fip "mk5=packet=28:0:5032:0:0;\n");
}
#...........
if($firmware_type eq "DDC8"){
  print (fip "rdbe_cmd=dbe0,50,dbe_personality=ddc:ddc_1601583.bin\n");
  print (fip "rdbe_cmd=dbe1,50,dbe_personality=ddc:ddc_1601583.bin\n");
  print (fip "rdbe_cmd=dbe0,3,dbe_execute=init\n\!+2s\n");
  print (fip "rdbe_cmd=dbe1,3,dbe_execute=init\n\!+2s\n");
  print (fip "\"essr=run,off\n");     
  print (fip "\"essr=mode,3\n");      # two in, one out
  print (fip "\"essr=route,2,4\n");  # route incoming traffic from input 1 and 2
  print (fip "\"essr=route,3,4\n");  # to output 1
  print (fip "\"essr=pace,4,3\n");   # set port 4 packet pacing to 5
  print (fip "\"essr=run,on\n");     
  print (fip "mk5=mode=vdifl_40000-1024-8-2;\n");
  print (fip "mk5=fill_pattern=464374526;\n");
  print (fip "mk5=packet=28:0:5032:0:0;\n");
}
print (fip "enddef\n");
#....
#==================================================
print (fip "define  preob         00000000000x\n");
print (fip "ifread\n");
if($firmware_type eq "PFB"){
 print (fip "rdbe_cmd=0,3,dbe_quantize=hold_set;\n")
 }
else {
 print (fip "bbcread\n");
 if($firmware_type eq "DDC8"){
   print (fip "ifread2\n");
   print (fip "bbcread2\n");
 }
 }
print (fip "!+1s\n");
print (fip "op_stream=start\n");
print (fip "enddef\n");
print (fip "define  start         00000000000\n");
print (fip "collect@!,10s\n");
print (fip "enddef\n");
print (fip "define  collect       00000000000x\n");
print (fip "rdbe_tsys=dbe0,if0\n");
print (fip "rdbe_tsys=dbe0,if1\n");
#this will  crash, no receiver yet for dbe1 tsys, also addr for dbe1 multicast not set
if($firmware_type eq "DDC8"){
  print (fip "rdbe_tsys=dbe1,if0\n");
  print (fip "rdbe_tsys=dbe1,if1\n");
}
print (fip "enddef\n");
print (fip "define  setdbe1       00000000000x\n");
if($firmware_type eq "DDC8"){
print (fip "rdbe_cmd=1,3,dbe_data_send=on:$exper_start:$exper_finish:0;\n");
print (fip "\!+1s\n");
print (fip "rdbe_cmd=1,3,dbe_1pps_mon=enable:239.0.2.35:20020;\n");
print (fip "\!+1s\n");
print (fip "rdbe_cmd=1,3,dbe_tsys_mon=enable:239.0.2.35:20021:10;\n");
print (fip "\!+1s\n");
print (fip "rdbe_cmd=1,3,dbe_dot?;\n");
print (fip "\!+1s\n");
print (fip "rdbe_time=dbe1,3\n");
}
print (fip "enddef\n");
#============================================================
#=============================make scan list=========================================
#     J0431+20   04:28: 6.85  +20:31: 9.0     04:31:55.40  +20:39:16.5     04:31: 3.76  +20:37:34.2     
#SCHEDULE FOR EB_VLBA  EXPERIMENT BB321W2 ON TUES, DEC. 24, 2013 (DAY 358)          Page   1
# START  - STOP     -SOURCE-  -RA(1950)-- -DEC(1950)- --HA-- -AZ--  -EL- CABLE SLEW CAL 
# THE FOLLOWING SOURCE IS OUTSIDE TELESCOPE LIMITS.  INFORM THE SCHEDULER!
#03:15:00-03:23:00  0234+285  02:34:55.58 +28:35:11.2  07:15 302.3  11.2       ****  10 
#03:27:40-03:31:40  J0437+24  04:34: 1.01 +24:50: 5.8  05:28 280.9  23.9        1.1  10 
print(fil  "SOURCES IN THIS SCHEDULE  (DATE=",substr($fmt_start_time[0],0,8),")\n\n");
print(fil  "    NAME    RA(2000)   DEC(2000)\n");
for($ns=0; $ns<$nscan;$ns++){
   $fsrc=sprintf("%10s",$scan_source[$ns]);
   ($x,$y,$z)=split(',',$coords{$scan_source[$ns]});
   $lra[$ns]=substr($x,0,2).":".substr($x,2,2).":".substr($x,4,4);
   $py=0; $ldec[$ns]="+"; if(substr($y,0,1) eq "-"){$ldec[$ns]="-";$py=1;}
   $ldec[$ns]=$ldec[$ns].substr($y,$py,2).":".substr($y,($py+2),2).":".substr($y,$py+4,4);
   if($srclistdone{$scan_source[$ns]} eq "" ){
#    print ( $fsrc," ", $lra[$ns]," ",$ldec[$ns],"\n");
     print (fil $fsrc," ", $lra[$ns]," ",$ldec[$ns],"\n");
     $srclistdone{$scan_source[$ns]} ="done";
   }
}
print (fil "\n");
print (fil " SCHEDULE FOR STATION ",uc($our_station)," EXPERIMENT ",uc($exper)," ON ",substr($fmt_start_time[0],0,8),"\n");
print (fil "\n");
print (fil " START ->  STOP>     -SOURCE-  -RA(2000)- -DEC(2000)-   -AZ--   -EL- \n");
print (fil "\n");
#
for($ns=0; $ns<$nscan;$ns++){
   $hms1=substr($fmt_start_time[$ns],9,15);
   $hms2=substr($fmt_stop_time[$ns],9,15);
   $fsrc=sprintf("%10s",$scan_source[$ns]);
#  print ( $hms1,"->",$hms2,"  ", $fsrc," \n");
   $azel=&getazel($fmt_start_time[$ns],$xst,$yst,$zst,$lra[$ns],$ldec[$ns]);
   ($azz,$ell)=split(':',$azel);
   $fmazel=sprintf("%6.1f %6.1f",$azz,$ell);
#  print "SS2 $hms1 >$scan_source[$ns]< $coords{$scan_source[$ns]} $lra[$ns] $ldec[$ns] $fmazel\n";
   print (fil $hms1,"->",$hms2," ",$fsrc,"  ",$lra[$ns]," ",$ldec[$ns],"  ",$fmazel,"\n"); 
}
#SUBS=======================================================
#===========================================================
sub make_proc_pfb {
  local($psetup,$pfb_in,$format)=@_;
# print(fip "dbe0ini\n"); #not needed for PFB
  print(fip "define  ",$psetup,"       00000000000x\n");
  print(fip $pcalstate{$psetup},"\n");
#---missed this---??
#lo lines to start
  print(fip $loeqlo{$psetup});
  print(fip "rdbe_person=dbe0,pfb\n");  #ask for correct personality
#frequency list to setup
  print(fip $fqlist{$psetup});
#-------------------
  (@each_chan)=split('#',$pfb_in);
  $ne=$#each_chan+1;
  $ioch=""; #XXX
  for($ie=0;$ie<$ne;$ie++){
     ($cch,$cbn,$crf,$csb,$cbw,$cpol,$cif,$clo,$closb,$cpcal)=split(":",$each_chan[$ie]);
     if($cif eq 'A' ){$iif=0;} if($cif eq 'C' ){$iif=1;} #XXX
     if($cif eq 'B' ){$iif=0;} if($cif eq 'D' ){$iif=1;} #XXX
     $fq=$crf-$clo; $bbw=int($cbw);
     if($fq <0){$fq=-$fq;} #XXX
     for ($ch=0;$ch<16;$ch++){
       if($fq == $chet_fh[$ch]){
          $ich=$ch;
          $ioch=$ioch.$iif.":".$ich."::"; #XXX
       }
     }
#    print "$ch $fq $ich\n";
  }
  $ioch=substr($ioch,0,(length($ioch)-2)).";";
  print (fip "rdbe_cmd=0,5,dbe_ioch_assign=",$ioch,"\n");
  print (fip "mk5=fill_pattern=464374526;\n");
  print (fip "mk5=packet=36:0:5008:0:0;\n");
  print (fip "mk5=mode=mark5b:0xffffffff:1;\n");
  print (fip "mk5=play_rate=data:64;\n");
  print (fip "bank_check\n");
  print (fip "enddef\n");
}
#==================================================================================
sub make_proc_ddc {
  local($dsetup,$ddc_in,$format)=@_;
# print "#RACKFORMAT WANTED=$format\n";
# $fsetup0="cfgdb0".substr($dsetup,5,2); #name of fset procedure
# $fsetup1="cfgdb1".substr($dsetup,5,2);
# $fchkup0="chkup0".substr($dsetup,5,2); #name of fset procedure
# $fchkup1="chkup1".substr($dsetup,5,2); #name of fset procedure
  (@each_chan)=split('#',$ddc_in);
  $ne=$#each_chan+1;
# $ne1=$ne; $ne2=0; if($ne >4){$ne1=4; $ne2=$ne-4;} 
#ne1 channels in dbe0, ne2 in dbe1
  print(fip "define  ",$dsetup,"       00000000000x\n");
#print "DBGPCAL 575 $dsetup $pcalstate{$dsetup}\n";
  print(fip $pcalstate{$dsetup},"\n");
#lo lines to start
  print(fip $loeqlo{$dsetup});
  print(fip "rdbe_person=dbe0,ddc\n");  #ask for correct personality
#frequency list to setup
  print(fip $fqlist{$dsetup});
# if($ne2 >0){print(fip "$fsetup1\n");}
###  print(fip "$fsetup0\n"); #cfg statements in own procedure
###  print(fip "enddef\n");
#.........................
###  print(fip "define  ",$fsetup0,"       00000000000x\n");
# print(fip "dbeini\n");
# ====================dbe stream off to change=============
 print (fip "rdbe_send=dbe0,off\n");
 print (fip "rdbe_cmd=dbe0,3,dbe_dot?\n");
 if($firmware_type eq "DDC8"){
   print (fip "rdbe_send=dbe1,off\n");
   print (fip "rdbe_cmd=dbe1,3,dbe_dot?\n");
 }
# ==========================================================
  $xbb0=""; $xth0="";
  $xbb1=""; $xth1="";
  $ibn1=0; $ibn2=0;
  for($ii=0;$ii<$ne;$ii++){
     $dname="dbe".$whichdbe[$ii];
     if($whichdbe[$ii] == 0){
         $ibn1++;
	 $dbechan=$dbe0chan[$ii];
         $xbb0=$xbb0.$xbar[$ii].":";
	 $xth0=$xth0.$ii.":";
         if($ibn1 ==4){$xbb0=$xbb0."2:2:2:2;";}
     }
     if($whichdbe[$ii] == 1){
         $ibn2++;
	 $dbechan=$dbe1chan[$ii];
         $xbb1=$xbb1.$xbar[$ii].":";
	 $xth1=$xth1.$ii.":";
         if($ibn2 ==4){$xbb1=$xbb1."2:2:2:2;";}
     }
     print(fip "rdbe_dc_cfg=",$dname,",",$dbechan,$xcfg[$ii],"\n");
#    print "DBG540 $dname dbechan/ii/xcfg[ii]=$dbechan $ii $xcfg[$ii]\n";
  }
  print(fip "rdbe_cmd=dbe0,3,dbe_xbar=");
  $lx=length($xbb0); $xbb0=substr($xbb0,0,($lx-1));
  print(fip $xbb0,";\n");
  if($xbb1 ne ""){
    print(fip "rdbe_cmd=dbe1,3,dbe_xbar=");
    $lx=length($xbb1); $xbb1=substr($xbb1,0,($lx-1));
    print(fip $xbb1,";\n");
  }
  print(fip "rdbe_cmd=dbe0,3,dbe_vdif_threads_id=");
  $lx=length($xth0); $xth0=substr($xth0,0,($lx-1));
  print(fip $xth0,";\n");
  if($xth1 ne ""){
    print(fip "rdbe_cmd=dbe1,3,dbe_vdif_threads_id=");
    $lx=length($xth1); $xth1=substr($xth1,0,($lx-1));
    print(fip $xth1,";\n");
  }
# ====================dbe stream on again=============
  print (fip "\!+1s\n");
  print (fip "bank_check\n");
  print (fip "rdbe_send=dbe0,on\n");
  if($firmware_type eq "DDC8"){
    print (fip "rdbe_send=dbe1,on\n");
    if($bbw == 64)   {print (fip "mk5=mode=vdifl_40000-2048-8-2;\n");}
    elsif($bbw == 32){print (fip "mk5=mode=vdifl_40000-1024-8-2;\n");}
    elsif($bbw == 16){print (fip "mk5=mode=vdifl_40000-512-8-2;\n");}
    elsif($bbw == 8) {print (fip "mk5=mode=vdifl_40000-256-8-2;\n");}
    elsif($bbw == 4) {print (fip "mk5=mode=vdifl_40000-128-8-2;\n");}
    elsif($bbw == 2) {print (fip "mk5=mode=vdifl_40000-64-8-2;\n");}
    elsif($bbw == 1) {print (fip "mk5=mode=vdifl_40000-32-8-2;\n");}
    else {print (fip "mk5=mode=vdifl_40000-1024-8-2;\n");}
    print (fip "mk5=fill_pattern=464374526;\n");
    print (fip "mk5=packet=28:0:5032:0:0;\n");
  }
  else{
    if($bbw == 128)  {print (fip "mk5=mode=vdifl_20000-2048-4-2;\n");}
    elsif($bbw == 64){print (fip "mk5=mode=vdifl_20000-1024-4-2;\n");}
    elsif($bbw == 32){print (fip "mk5=mode=vdifl_20000-512-4-2;\n");}
    elsif($bbw == 16){print (fip "mk5=mode=vdifl_20000-256-4-2;\n");}
    elsif($bbw == 8) {print (fip "mk5=mode=vdifl_20000-128-4-2;\n");}
    elsif($bbw == 4) {print (fip "mk5=mode=vdifl_20000-64-4-2;\n");}
    elsif($bbw == 2) {print (fip "mk5=mode=vdifl_20000-32-4-2;\n");}
    elsif($bbw == 1) {print (fip "mk5=mode=vdifl_20000-16-4-2;\n");}
    else {print (fip "mk5=mode=vdifl_20000-512-4-2;\n");}
    print (fip "mk5=fill_pattern=464374526;\n");
    print (fip "mk5=packet=28:0:5032:0:0;\n");
  }
  print (fip "\!+1s\n");
# ==========================================================
  print(fip "enddef\n");
}
#==================================================================================
sub make_cfg_ddc {
  local($dsetup,$ddc_in)=@_;
#make cfg and xbar lists
  (@each_chan)=split('#',$ddc_in);
  $ne=$#each_chan+1;
  for($i=0;$i<8;$i++){$xbar[$i]="";$xcfg[$i]="";}
#====================================================
  for($ie=0;$ie<$ne;$ie++){
     ($cch,$cbn,$crf,$csb,$cbw,$cpol,$cif,$clo,$closb)=split(":",$each_chan[$ie]);
     $fq=$crf-$clo; $bbw=int($cbw);
#    print "DBG704 $dsetup $ie FQ,SB,POL,IF,WHICHDBE,DBECHAN=$fq $csb $cpol $cif $whichdbe[$ie] \n";
#check LO sideband
     $rsb=$csb;
     if($fq <0){
	 $fq=-$fq; 
	 if($csb eq "U"){$rsb = "L";}
	 if($csb eq "L"){$rsb = "U";}
	 $csb=$rsb;
     }
#    print "DBG684 $ie FQ,SB,POL,IF,WHICHDBE,DBECHAN=$fq $csb $cpol $cif $whichdbe[$ie] \n";
###temporary inelegance...
# New DG 27JUL14
     $deci=int(128/$bbw); 
# Old, misses 8 MHz and wrong for 4 MHz and below.
#     if($bbw == 128){$deci=1;} if($bbw == 64){$deci=2;} if($bbw == 32){$deci=4;} 
#     if($bbw == 16){$deci=8;} if($bbw == 4){$deci=16;} if($bbw == 2){$deci=32;} 
#     if($bbw == 1){$deci=64;}
     $ssb=1; if($csb eq "L"){$ssb=0;}
     if($ssb == 1){
        if($fq <=(640-$bbw))  {$fdds=$fq-512+$bbw/2;$bb_mode=1;$sub=2; goto ex1;}
        if($fq <=(768-$bbw))  {$fdds=768-$fq-$bbw/2;$bb_mode=0;$sub=1; goto ex1;}
        if($fq <=(768-$bbw/2)){$fdds=768-$fq-$bbw/2;$bb_mode=0;$sub=1; goto ex1;}
        if($fq < 768){$fdds=$fq+$bbw/2-768;$bb_mode = 1;$sub=1; goto ex1;}
        if($fq <=(896-$bbw)) {$fdds= $fq-768+$bbw/2;$bb_mode=1;$sub=1; goto ex1;}
        if($fq <=(1024-$bbw)){$fdds= 1024-$fq-$bbw/2;$bb_mode=0;$sub=0; goto ex1;}
      } else {
        if($fq >=(896+$bbw)) {$fdds=1024-$fq+$bbw/2;$bb_mode=1;$sub=0; goto ex1;}
        if($fq >=(768 + $bbw)){$fdds=$fq-768-$bbw/2;$bb_mode=0;$sub=3; goto ex1;}
        if($fq >=(768 + $bbw/2)){$fdds=$fq-768-$bbw/2;$bb_mode=0;$sub=3; goto ex1;}
        if($fq >=768){ $fdds = 768-$fq+$bbw/2;$bb_mode=1;$sub=3; goto ex1;}
        if($fq >=(640 + $bbw)){$fdds=768-$fq+$bbw/2;$bb_mode=1;$sub=3; goto ex1;}
        if($fq >=(512 + $bbw)){$fdds=$fq-512-$bbw/2;$bb_mode=0;$sub=2; goto ex1;}
     }
     ex1:
     $im1=$cbn-1;    #chan numbers start at 0
     $sfdds=sprintf("%4.6f",$fdds);
#    print "DBG707 $ie F,SB,BW,PL,IF,DBEN,DBECHN=$fq $csb $bbw $cpol $cif $whichdbe[$ie] >> $fq $sfdds $bb_mode $sub\n";
#    HELP is following IF order correct?  #    NONONONONONO !!??
#depends on 4x4 SWITCH!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     if($cif eq 'A' ){$iif=0;} if($cif eq 'C' ){$iif=1;}
     if($cif eq 'B' ){$iif=0;} if($cif eq 'D' ){$iif=1;}
     $ixf=$iif; if($ixf >1){$ixf=$iif-2;}
     $xbar[$im1]=$sub+$ixf*4;
     $xcfg[$ie]=$im1.":".$deci.":".$sfdds.":".$bb_mode;
#    print "DBG613 ie,im1=$ie $im1\n";
   }
#  for($i=0;$i < $ne;$i++){
#     print "DBG653:CFG $i XCFG,XBAR=$xcfg[$i] $xbar[$i] $whichdbe[$i] $dbe0chan[$i] $dbe1chan[$i]\n";
#  }
}
#===========================================================
#===========================================================
#frequency list and LO/IF setup: part of setup procedure
#rdbef statements are just a list to be used by later calibration software,
#FS looks up cals for these frequencies. The rdbe_if statements will also
#set the NRAO 4-way IF switch if it is present
#
sub make_fqlist {
  local($ssetup,$fdc_in)=@_;
# print "DBG703IN:$fdc_in,$ssetup,$firmware_type\n";
#standard FS lo lines to start
  print(fip $loeqlo{$ssetup});
  $fqlist{$ssetup}=""; #ass array collects statements
  (@fach_chan)=split('#',$fdc_in);
  $nf=$#fach_chan+1;
#========XXX====DDC==where to allocate chans dbe0/1 Apr14================
#===========try to put all chans from 1 pol in 1 dbe========================
  if($firmware_type eq "PFB"){
     for($ie=0;$ie<$nf;$ie++){
        $whichdbe[$ie] == 0;
     }
  } else {
     $nrcp=0; $nlcp=0;
     for($ie=0;$ie<$nf;$ie++){
        ($cch,$cbn,$crf,$csb,$cbw,$cpol,$cif,$clo,$closb,$cpcal)=split(":",$fach_chan[$ie]);
        if($cpol eq "R"){$nrcp++;}
        if($cpol eq "L"){$nlcp++;}
        if($ie < 4){
           $whichdbe[$ie]=0;
        } else {
           $whichdbe[$ie]=1;
        }
     }
# print "DBG607 NR,NL, Ntot=$nrcp $nlcp $ne\n";
#if dual pol 8 channel, send R to dbe0 and L to dbe1
     if(($nf >4) && ($nrcp == $nlcp)){
       for($ie=0;$ie<$nf;$ie++){
          ($cch,$cbn,$crf,$csb,$cbw,$cpol,$cif,$clo,$closb,$cpcal)=split(":",$fach_chan[$ie]);
          if($cpol eq "R"){$whichdbe[$ie]=0;}
          if($cpol eq "L"){$whichdbe[$ie]=1;}
       }
     }
  }
#enumerate channels inside each dbe
  $ich0=-1; $ich1=-1;
  for($ie=0;$ie<$nf;$ie++){
      if($whichdbe[$ie] == 0){$ich0++; $dbe0chan[$ie]=$ich0;$dbe1chan[$ie]=-1;}
      if($whichdbe[$ie] == 1){$ich1++; $dbe1chan[$ie]=$ich1;$dbe0chan[$ie]=-1;}
#     print "DBG790fqlist whichdbe $ie $whichdbe[$ie]\n";
  }
#====================================================
# print (fip "rdbefclr=dbe0\n");
  $fqlist{$ssetup}=$fqlist{$ssetup}."rdbefclr=dbe0\n";
# print "DBGFQQ $firmware_type $#fq_chans $ssetup \n";
  if($firmware_type eq "DDC8"){$fqlist{$ssetup}=$fqlist{$ssetup}."rdbefclr=dbe1\n";}
  $lo0_line=""; $lo1_line="";
  for($ie=0;$ie<$nf;$ie++){
     ($cch,$cbn,$crf,$csb,$cbw,$cpol,$cif,$clo,$closb,$cpcal)=split(":",$fach_chan[$ie]);
     $fq=$crf-$clo; $bbw=int($cbw);$ccpol=lc($cpol);$plosb=lc($closb)."sb";
     $im1=$cbn-1;    #chan numbers start at 0
# give edge frequency instead of centre: because this happens in rdbefxx anyway
     if($cif eq 'A' ){$iif=0;} if($cif eq 'C' ){$iif=1;}
     if($cif eq 'B' ){$iif=0;} if($cif eq 'D' ){$iif=1;}
     if($cif eq "A"){$lo0_line="loa,".$clo.",".lc($plosb).",".lc($cpol)."cp,dbe0,if0";}
     if($cif eq "C"){$lo1_line="loc,".$clo.",".lc($plosb).",".lc($cpol)."cp,dbe0,if1";}
     if($cif eq "B"){$lo0_line="lob,".$clo.",".lc($plosb).",".lc($cpol)."cp,dbe0,if0";}
     if($cif eq "D"){$lo1_line="lod,".$clo.",".lc($plosb).",".lc($cpol)."cp,dbe0,if1";}
     if(substr($firmware_type,0,3) eq "DDC"){
       $ich=0;
       $dben=$whichdbe[$im1];
     } else {
       $dben=0;
       for ($ch=0;$ch<16;$ch++){
         if($fq == $chet_fh[$ch]){
            $ich=$ch;
         }
       }
     }
     $ibn=sprintf("%02d",$ie+1);
     $psb=lc($csb)."sb";     $ppol=lc($cpol); $pbw=int($cbw);
     $fqlist{$ssetup}=$fqlist{$ssetup}."rdbef".$ibn."=dbe".$dben.",if".$iif.",".$crf.",";
     $fqlist{$ssetup}=$fqlist{$ssetup}.$pbw.",".$psb.",".$ppol.",".$ich."\n";
  }
  $loeqlo{$ssetup}="lo=\n";
# print "LOLINES=> $lo0_line <> $lo1_line \n";
  if($lo0_line ne ""){
     $fqlist{$ssetup}=$fqlist{$ssetup}."rdbe_if=".$lo0_line."\n";
     $loeqlo{$ssetup}=$loeqlo{$ssetup}."lo=".$lo0_line."\n";
  }
  if($lo1_line ne ""){
     $fqlist{$ssetup}=$fqlist{$ssetup}."rdbe_if=".$lo1_line."\n";
     $loeqlo{$ssetup}=$loeqlo{$ssetup}."lo=".$lo1_line."\n";
  }
#massage standard LO lines
#XXXXXXXX---------
  $loeqlo{$ssetup}=~ s/,u,/,usb,/g;
  $loeqlo{$ssetup}=~ s/,l,/,lsb,/g;
  $loeqlo{$ssetup}=~ s/,l,/,lcp,/g;
  $loeqlo{$ssetup}=~ s/,r,/,rcp,/g;
  $loeqlo{$ssetup}=~ s/,dbe0,/,/g;
  $loeqlo{$ssetup}=~ s/,dbe1,/,/g;
  $loeqlo{$ssetup}=~ s/,if0/,off/g; #NONONO Nasty... Must check if pcal on,,,,,,
  $loeqlo{$ssetup}=~ s/,if1/,off/g; #NONONO Nasty...
  $pcalstate{$ssetup}="pcaloff";
  if($cpcal ne ""){
     if(($cpcal eq "1") || ($cpcal eq "5")){
        $loeqlo{$ssetup}=~ s/off/$cpcal/g;
        $pcalstate{$ssetup}="pcalon";
     }
  }
##  print "DBG817PCAL=>$cpcal<>$ssetup<>$pcalstate{$ssetup}\n";
# print "DBG765 $ssetup $loeqlo{$ssetup}\n";
  if($firmware_type eq "DDC8"){
     $lo0_line=~ s/dbe0/dbe1/g;
     $lo1_line=~ s/dbe0/dbe1/g;
     if($lo0_line ne ""){$fqlist{$ssetup}=$fqlist{$ssetup}."rdbe_if=".$lo0_line."\n";}
     if($lo1_line ne ""){$fqlist{$ssetup}=$fqlist{$ssetup}."rdbe_if=".$lo1_line."\n";}
  }
}
#===========================================================
#========================time code/decode===================
sub vextime_2_secs {
  local($ftt)=@_;
  $ftt=~ /(\d+)y(\d+)d(\d+)h(\d+)m(\d+)s/;
  $ly=$1;
  $ssec=$2*86400+$3*3600+$4*60+$5+$dsec;
  $string=$ssec;
# print "$ftt $string dsec=$dsec\n";
  $string;
}
sub secs_2_fstime {
  local($ssec)=@_;
  $ld=int($ssec/86400.0); $ssec=$ssec-$ld*86400;
  $lh=int($ssec/3600.0); $ssec=$ssec-$lh*3600;
  $lm=int($ssec/60.0); $ls=$ssec-$lm*60;
  $string=sprintf("%04d.%03d.%02d:%02d:%02d",$ly,$ld,$lh,$lm,$ls);
  $string;
}
#==================================================================================
#=========azel calculation for listing=== made from ancient code....
sub getazel {
  local($time,$xx,$yy,$zz,$ara,$adec)=@_;
# print "$time $xx $yy $zz $ara $adec\n";
  $alona=-atan2($yy,$xx); #Mk3 convention
  $q1=$zz/(sqrt($xx*$xx+$yy*$yy+$zz*$zz));
  $gclat=atan2($q1,sqrt(1-$q1*$q1));
  $dlat=-3.35899e-3*sin(2.00*$gclat)+5.6398e-6*sin(4.0*$gclat)-0.01261e-6*sin(6.0*$gclat);
  $alata=$gclat-$dlat;
  $yrfl=substr($time,0,4); $day=substr($time,5,3); $hhmmss=substr($time,9,8);
  $ieyr=$yrfl-1900;
  $tfl=int(($ieyr)/4+$day);
  $tfl=$tfl+$ieyr*365.0-0.5;
  $gstm= 1.73993589 +$tfl * (0.0172027913 + $tfl * 5.064E-15);
  ($turns)=split(/\./,($gstm/6.283185306));
  $gstm=$gstm-$turns*6.283185306;
  $sfl=($tfl-36523.500)/36524.21988;  #(2000)
  $zeta0=$sfl * (0.011177209 + 0.000001464 *$sfl);
  $zfl  =$sfl * (0.011177209 + 0.000005299 *$sfl);
  $theta=$sfl * (0.009719079 - 0.000002065 *$sfl);
  $r1=substr($ara,0,2); $r2=substr($ara,3,2); $r3=substr($ara,6,4);
  $ra2000=($r1+$r2/60+$r3/3600)*3.14159/12.0;
  $d1=substr($adec,1,2); $d2=substr($adec,4,2); $d3=substr($adec,7,4);
  $dc2000=($d1+$d2/60+$d/3600)*3.14159/180.0;
# print "RAD $r1 $r2 $r3 $d1 $d2 $d3 $ra2000 $dc2000 \n";
  if(substr($adec,0,1) eq "-"){$dc2000=-$dc2000;}
  $crz= cos($ra2000 +$zeta0);
  $srz= sin($ra2000 +$zeta0);
  $tt2= sin($theta / 2.0)/cos($theta/2.0);
  $qfl= sin($theta) * (sin($dc2000)/cos($dc2000)+$tt2*$crz);
  $xfl= atan2($qfl * $srz , (1.0 - $qfl * $crz)); #correct ?
  $radat= $ra2000 + $zeta0 + $zfl + $xfl;
  $arg= $tt2 * ($crz - $srz * sin($xfl / 2.0)/cos($xfl/2.0));
  $decdat= $dc2000 + 2.0 * atan2($arg,1.0);
  $hh=substr($hhmmss,0,2); ($mm,$sc)=split(":",substr($hhmmss,3,5)); $imm=$mm;
  $hhr=$hh+$mm/60.0+$sc/3600.0;
  $utrad=$hhr*3.14159265/12.0;
  $hang=$gstm+$utrad*1.002738-$alona-$radat;
  $sel=sin($alata)*sin($decdat)+cos($alata)*cos($decdat)*cos($hang);
  $cel=sqrt(1.0-$sel*$sel); $elv=atan2($sel,$cel)*180./3.14159265;
  $cosaz=(sin($decdat)-sin($alata)*$sel)/(cos($alata)*$cel);
  $sinaz=-cos($decdat)*sin($hang)/$cel;
  $azim=atan2($sinaz,$cosaz)*180./3.14159265;
  if($azim < 0.0){$azim=$azim+360;}
  $string=$azim.":".$elv;
  $string;
}
#===========================================================
