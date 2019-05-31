#!/usr/bin/perl
#---------------------------------------------------------------------------
#  A perl script  used to adjust the PCFS ifd and ifdtp attenuators! 
#  This uses  PH's inject_snap routine
#
# Usage:      >adjust_ifd.pl vc# vc# ...
# Example:    >adjust_ifd.pl  7
# Example:    >adjust_ifd.pl  9 11
# From oprin: >sy=/usr2/st/scripts/adjust_ifd.pl  &
#
#
# vc#'s are the VC's which the user would like to skip in the ifd/ifdtp
# optiminization procedure.
#
# Code can be found in pcfs10:/usr2/st/scripts/  . Also be sure to
# get all the *.ph files; see pcfs10:/usr/lib/perl4
#
# This code currently is set to be run when the PCFS is running
# and the antenna is pointing to the sky at zenith!.
#
# Additional debugging output is obtained by running the script
# (when the PCFS is running) directly from a xterm window.
#
#
# If you recieve a "warning or error message" you might simply
# try to reissue the procedure again.
#
#  Version 1.0  1/19/98   cjn  :  Change Comments Go Here
#       Mods    3/10/98   cjn  :  Added option of skipping over some VCs
#				  in the ifd and iftp optiminization, and
#                                 clean up the comments a bit. A few bug fixes.
#               4/1/98    cjn  :  Add the reading of the log file name from
#                                 shared memory!
#               4/4/98    cjn  :  Estimate the best change in the ifdtp values. 
#                                 Much quicker to converge.
#               6/22/98   cjn  :  Determine bad vc's automatically and skip-them.
#
# Future work:
#      1) Include the possiblity of the automated VC's 10 dB attenuator. 
#      2) Tidy code up and improve the convergence.
#
#  Users Take Note:
#       Feel free to modify and improve. Please let me know of any bugs
#       or improvements you make! Thanks
#                                  Charles J. Naudet           JPL    4/4/98
#
#---------------------------------------------------------------------------
#
  require "/usr2/st/scripts/getlogname.pl";
#
#------------------------------ Switch settings ----------------------------
  $debug =  0;
  $snapit = 1;
# The max number of adjustment iterations
  $int_limit = 50;
# The iteration limit after which a vc outside the upper/lower limits
# is called bad and removed from the adjustment list
  $vc_limit = 35;
# The upper and lower VC and IF limits
  $vc_low = 2800;
  $vc_hi  = 8000;
  $ifd_low = 3000;
  $ifd_hi  = 15000;
#---------------------------------------------------------------------------
  print "    *****   Automated IFD and IFDTP level adjuster STARTED ! ****** \n" ;
  print "							\n";
  print "The VC  upper and lower limits = [$vc_low,$vc_hi] \n";
  print "The ifd upper and lower limits = [$ifd_low,$ifd_hi]\n";
  print "                                                   \n";
  print " Check that you are looking at the SKY and CAL is OFF \n";
  print " To stop adjust_ifd type \"stop_adjust\" in the oprin input window \n";
  print "  -----  a 10 second wait -------\n";
  print "   \n";
  sleep 10;
#
  $logname = &getlogname;
  $vss = @ARGV ;
# Number of variables =  vcs
  if ($vss >= 1) {
    print "The following VCs will NOT be used in ifd optiminization: \n";
    foreach $skip (@ARGV) {
      ($skip >= 1) && (@vcskip[$skip] = 1) ;
      print " $skip ";
    }
  }
  print " \n";
  if ( ($logname EQ "") || ($logname EQ "-") ) {
     $logname = "null";
  }
  $logname = "/usr2/log/" . $logname . ".log" ;
#
# Check that the file exists!
  (-r $logname) || die " $logname does not exist or is not readable! \n"; 

  open(PROC,"tail -10 -f $logname |") || die "Can't open tail pipe, $logname \n";
# Should be at Zenith! 
#
  print "$logname opened \n";
  ($debug == 1) && (print " logfile opened \n"); 
# Be sure your at sky and with cal off!
  if ($snapit == 1) {
    ($debug == 2) && (print ">adjust_ifd:    Sending out inject_snaps! \n");
    system("/usr2/st/bin/inject_snap \"vread\" "); 
    &update_log;
    &update_log;
  }
  $ok_done = 0;
  $count   = 0;
  while (<PROC>) {
        s/\n//g;
        $line=$_;
  	$change_ifd = 0 ;
        $change_ifdtp = 0;
        $_ =~ s/\$\$\$\$\$/65535/g;
  	@test =  split(/[\$:&#;,\/? ]/);
        ($debug == 1) && (print ">adjust   $line \n"); 
# 
  	($test[1] EQ 'tpi') &&  &check_tpiall;
  	($test[1] EQ 'ifd') && &check_ifd;
  	($test[2] EQ 'IFTP') && &check_ifdtp;
        ($test[1] EQ 'patch') && &check_patch;
#
  	($change_ifd != 0) && &change_ifd;
  	($change_ifdtp != 0) && &change_ifdtp;
#
        if ( (($change_ifd != 0) || ($change_ifdtp != 0)) && ($ok_done ==1) ) {
         print ("                NO changes to IFD or IFDTP needed! \n");
         print ("   **********  IFD Adjust Script Stopped *********** \n");
         system("/usr2/st/minical/done adjust_ifd");   
	 system("/usr2/st/bin/inject_snap \"cont\" ");
         exit;
        } 
#
        if(($count >= $int_limit) && ($ok_done == 2)) {
           $count   = 0;
           $ok_done = 3;
           $change_ifdtp = 1;
           print("*************************************************************\n");
           print("WARNING: Unable to optimize all VCs within the limits chosen!\n");
           print("WARNING: Possible BAD VC, Incorrect Patching or Bad RF path! \n");
           print("---------------------- 10 second wait -----------------------\n");
           print("*************************************************************\n");
           system("/usr2/st/bin/inject_snap \"wakeup\" ");
           sleep 10;
        }
           
#
        ($line =~ /stop_adjust/) && die "   *** IFD Adjust Stopping Script (software stop)***  \n" ;
        ($line =~ /terminate/) && (die) ;
# 
        if ($count > $int_limit) {
         system("/usr2/st/bin/inject_snap \"wakeup\" ");
         system("/usr2/st/bin/inject_snap \"vread\" "); 
         print("*************************************************************\n");
         print("WARNING!! Possible BAD VC, Incorrect Patching or BAD RF path!\n");
         print("WARNING!! Check RF Signal Path and VCs                       \n"); 
         print("WARNING!! Be Sure All VC's used are > 2000 counts            \n");
         print("WARNING!! Try adjust_ifd.pl again or Adjust the IFds manually\n"); 
         print("---------------------- 10 second wait -----------------------\n");
         print("*************************************************************\n");
         sleep 10;
         print("*** IFD Adjust Script Stopped *** \n");
         system("/usr2/st/bin/inject_snap \"cont\" ");
         close PROC;
         exit
        }
#  
        if ($ok_done == 4) {
          print " -----------------------------------\n";
          system("/usr2/st/bin/inject_snap \"vread\" ");          
          print " Final IFD   = $ifda_atten $ifdb_atten \n";
          print " Final IFDTP = $ifdtpa_atten $ifdtpb_atten \n";
          print "   *** IFD Adjust Script Completed *** \n" ; 
          system("/usr2/st/minical/done adjust_ifd");
          system("/usr2/st/bin/inject_snap \"cont\" ");

#          close PROC;
          exit;
        }
  }
  ($debug == 1) && (print ">adjust Script Finished ! \n");
  system("/usr2/st/bin/inject_snap \"cont\" ");
  close PROC; 
#------------------------------------------------------------------------------------ 
  sub update_log {
#   So we do it
   ($debug == 1) && (print ">adjust in subroutine update_log \n");
   if ($snapit == 1) {
    system("/usr2/st/bin/inject_snap \"ifdtp\" ");
# Must wait 1 seconds here! This really takes a long time!
    sleep(1.0);
#
    system("/usr2/st/bin/inject_snap \"patch\" ");
#
    system("/usr2/st/bin/inject_snap \"ifd\" ");
#
    system("/usr2/st/bin/inject_snap \"tpiall\" ");
#    sleep(1.0);
   }
  }
  sub check_tpiall {
    if ($debug == 1) {
      print  ">adjust in sub routine  check_tpiall \n  @test \n";
    }
    $change_ifd  = 0;
    $change_ifda = 0;
    $change_ifdb = 0;
    $size = @test;
#
# A check that all the necessary information has been aquired!
    if ( (@if1_vc <=0) && (@if2_vc <= 0 ) ) {
      $update_log;
      return;
    }
#
#    Don't include if1 or if2 here
    @vc = @test[2..$size-2];
    print "The current vc values = @vc \n";
#   
    $count++ ;
    print ("\n \n");
    print ("IFD Adjust Interation #$count \n");
    ($ok_done == 3) && (return);
#
    for ( $i=0; $i <= 13; $i++) {
     $j = $i + 1;
     if($vcskip[$i+1] == 0) { 
       if($if1_vc[$i+1] == 1) {
          if ($vc[$i] <= $vc_low) {
             $change_ifda = -1;
             print "	if1  vc #$j lower than the limit, $vc[$i]   Adjusting \n";
             if ($count > $vc_limit) {
                 ($vcskip[$i+1]=1);
             }
          } elsif ( $vc[$i] >= $vc_hi  ) {
             $change_ifda = +1;
             print "	if1  vc #$j higher than the limit, $vc[$i]  Adjusting \n";
             if ($count > $vc_limit) {
                 ($vcskip[$i+1]=1);
             }
          } else {
#            the vc is AOK
          }
       } elsif ($if2_vc[$i+1] == 1) { 
          if ($vc[$i] <= $vc_low) {
             $change_ifdb = -1;
             print "	if2  vc #$j lower than the limit, $vc[$i]   Adjusting \n";
             if ($count > $vc_limit) {
                 ($vcskip[$i+1]=1);
             }
          } elsif ($vc[$i] >= $vc_hi) {
             $change_ifdb = +1;
             print "	if2  vc #$j higher than the limit,$vc[$i]  Adjusting \n";
             if ($count > $vc_limit) {
                 ($vcskip[$i+1]=1);
             }
          } else {
#            the vc is AOK
          }
       } else {
          ($debug == 1) &&  print " VC number $i is not patched!  \n";
       }
     } else {
       print "**--> VC #$j is NOT being used in IF adjustment procedure!\n";
     }
    }  
#
#
   if (($change_ifda != 0) || ($change_ifdb != 0)) {
    $change_ifd = 1 ;
    $ok_done    = 2 ;
    }
#
   if($change_ifd == 0) {
      ($ok_done = 3) ;  
   }	
  }
  sub change_ifd {
    ($debug == 1) && (print ">adjust in subroutine change_ifd \n");
     $ifda_new = $ifda_atten + $change_ifda;
     $ifdb_new = $ifdb_atten + $change_ifdb;
    ($change_ifda < 0) && (print " Decreasing the if1 atten \n");
    ($change_ifdb > 0) && (print " Increasing the if2 atten \n");
    ($ifda_new == 0) && (die "WARNING1! No Apparent Signal if1 ! \n");
    ($ifdb_new == 0) && (die "WARNING1! No Apparent Signal if2 ! \n");
    ($ifda_new > 63) && (die "WARNING! If1 attenuator at limit ! \n");
    ($ifdb_new > 63) && (die "WARNING! If2 attenuator at limit ! \n");
 
    ($snapit == 1) && system("/usr2/st/bin/inject_snap \"ifd=$ifda_new,$ifdb_new,*,* \" ");
    (print "Changing IFD -->ifd=$ifda_new,$ifdb_new,*,* \n"); 
     &update_log;
  }
  sub change_ifdtp {
     $ifdtpa_new = $ifdtpa_atten + $change_ifdtpa;
     $ifdtpb_new = $ifdtpb_atten + $change_ifdtpb;
    ($ifdtpa_new == 0) && (die "WARNING2! No Apparent Signal if1! \n");
    ($ifdtpb_new == 0) && (die "WARNING2! No Apparent Signal if2! \n");
#    ($ifdtpa_new > 31) && (die "WARNING! Ifdtp1 attenuator at limit! \n");
#    ($ifdtpb_new > 31) && (die "WARNING! Ifdtp2 attenuator at limit! \n");
    if ($ifdtpa_new >= 31) {
       $ifdtpa_new   = 31;
       $change_ifdtpa = 31-$ifdtpa_atten;
       print "Over max! dropping IFDTP to $ifdtpa_new \n";
    }
    if ($ifdtpb_new >= 31) {
       $ifdtpb_new   = 31;
       $change_ifdpb = 31 - $ifdtpb_atten;
       print "Over max! dropping IFDTP to $ifdtpb_new \n";
    }
#    print "$change_ifdtpa, $change_ifdtpb \n"; 
    if (($change_ifdtpa != 0) || ($change_ifdtpb != 0 )) {
     ($snapit == 1) && system("/usr2/st/bin/inject_snap \"ifdtp=$ifdtpa_new,$ifdtpb_new \" ");
     (print "Changing IFDTP ->ifdtp=$ifdtpa_new,$ifdtpb_new  \n");
     sleep(1.0);
    }
#
    if ( ($change_ifdtpa == 0) && ($change_ifdtpb == 0) ) {
       $change_ifdtp = 0;
       $ok_done=4;
    }
    &update_log;
 }
  sub check_ifd {
    if ($debug == 1) {
      print  ">adjust in sub routine  check_ifd  @test \n";
    }
    $size = @test;
    ( $size <=2 ) && (return);
    $ifda_atten = $test[2];
    $ifdb_atten = $test[3];
    $ifda = $test[7];
    $ifdb = $test[8];
    print ("Current IFD Settings $ifda_atten $ifdb_atten   Readings: $ifda $ifdb \n");   
    ($debug == 1) && (print "adjust $ifda_atten $ifdb_atten $ifda $ifdb \n");
    $change_ifdtp  = 0;
    $change_ifdtpa = 0;
    $change_ifdtpb = 0;
#
# First check if we should even adjust the ifdtp (ok_done==3_then check to
# be sure thats its patched (if1_if==1 and/or if2_if==1)
#


    if ($ok_done == 3) {
     if ($if1_if == 1) {
        $change_ifdtpa = 10.*0.434294482*log($ifda/$ifd_low);
        $change_ifdtpa = int($change_ifdtpa);

      if ($ifda <= $ifd_low) {
#         $change_ifdtpa = -2 ;
          if ($change_ifdtpa > -1) { $change_ifdtpa = -1 };
          print ("IFDA is low, decreading ifdtpa attenutor by $change_ifdtpa db; $ifda \n");
      } elsif ($ifda >= $ifd_hi) {
#         $change_ifdtpa = +2 ;
          if ($change_ifdtpa < 1)  { $change_ifdtpa =  1 };
          print ("IFDA is hi , increasing ifdtpa attenutor by $change_ifdtpa db; $ifda \n");
      }
     } 
     
     if ($if2_if == 1) { 
        $change_ifdtpb = 10.*0.434294482*log($ifdb/$ifd_low);
        $change_ifdtpb = int($change_ifdtpb);

      if ($ifdb <= $ifd_low) {
#         $change_ifdtpb = -2;
          if ($change_ifdtpb > -1) { $change_ifdtpb = -1 };
          print ("IFDB is low, decreasing ifdtpb attenutor by $change_ifdtpb db; $ifdb \n");
      } elsif ($ifdb >= $ifd_hi) {
#         $change_ifdtpb = +2;
          if ($change_ifdtpb < 1)  { $change_ifdtpb =  1 };
          print ("IFDB is hi , increasing ifdtpb attenutor by $change_ifdtpb db; $ifdb \n");
      }
     }
#
        ( ($change_ifdtpa != 0) || ($change_ifdtpb != 0) ) && ($change_ifdtp=1);
        ($change_ifdtp == 0) && ($ok_done = 4) ; 
    }
   }	
  sub check_ifdtp{
    if ($debug == 1) {
      print  ">adjust in sub routine  check_ifdtp @test \n";
    }
    $size = @test;
    ( $size <= 2 ) && (return);
    $ifdtpa_atten = $test[5]/2.;
    $ifdtpb_atten = $test[6]/2.;
    print ("Current IFDTP Settings  $ifdtpa_atten $ifdtpb_atten \n");
    ($debug == 1) && (print ">adjust ifdtpa= $ifdtpa_atten  ifdtpb= $ifdtpb_atten  \n");
   }
  sub check_patch {
    if ($debug ==1) {
       print ">adjust in sub routine check_patch \n @test \n";
    }
    $size = @test;
    if($size > 3) {
        @temp = @test[3..$size];
    } else {
        return;
    }
    ($debug == 1) && (print "patch; @temp \n");
# zero the patching arrays
    $if1_vc[0..14] = 0 ;
    $if2_vc[0..14] = 0 ;
    $if1_if = 0;
    $if2_if = 0;
#
    if ($test[2] EQ 'lo1') {
      foreach $num (@temp) {
        $num =~ s/h//g;
        $num =~ s/l//g;
        if (($num > 0) || ($num <= 14)) {
           $if1_vc[$num] = 1;
           $if1_if       = 1;
        }
      } 
      ($debug == 1) && (print "Adjust patch $test[2] : @if1_vc \n");
    } elsif ($test[2] EQ 'lo2') {
      foreach $num (@temp) {
        $num =~ s/h//g;
        $num =~ s/l//g;
        if (($num > 0) || ($num <= 14)) {
           $if2_vc[$num] = 1;
           $if2_if       = 1;
        }
      }
      ($debug == 1) && (print "Adjust patch $test[2] : @if2_vc \n");
    }
#  Indicate that we read all the necesaary data at least once
    ($ok_done <= 1) && ($ok_done = 1); 
  }	
