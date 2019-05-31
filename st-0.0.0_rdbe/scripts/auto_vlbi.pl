#!/usr/bin/perl 
# This programs reads a file called /usr2/sched/*dsn_config on the PCFS
# machines to automate the startup of a VLBI track. The setup looks at the
# DSS number and the LO1 to determine the setup.
#
#  Usage: auto_vlbi.pl <Xant window display_name1> <PCFS display_name2>
#	  e.g.  auto_vlbi.pl eac40:0.0 eac40:0.1 will put xant,xplot,oci and
#	      	the scrolling log on eac40:0.0, anmd the three PCFS windows
#		on eac40:0.1
#
# Paul Harbison Canberra Deep Space Communications Complex Feb 1998.
#
# In the "initi" procedure of /usr2/proc/station.prc put in:
# 	proc=dsnstart
#	dsnstart
#
# Other script files are in /usr2/control/dsn_band_setup/ . They include:
#-rw-rw-r--   1 ops      rtx           318 Feb 11 01:53 43.fixed_subreflector
#-rw-rw-r--   1 ops      rtx           223 Feb 11 01:53 43.k-band.pcg.svlbi
#-rw-rw-r--   1 ops      rtx           417 Feb 11 01:53 43.k-band.svlbi
#-rw-r--r--   1 ops      rtx            52 Feb 11 03:40 43.l-band.pcg.svlbi
#-rw-rw-r--   1 ops      rtx           848 Feb 11 01:53 43.l-band.svlbi
#-rw-rw-r--   1 ops      rtx           222 Feb 11 01:53 43.s-band.vlbi
#-rw-rw-r--   1 ops      rtx           224 Feb 11 01:53 43.x-band.vlbi
#-rw-rw-r--   1 ops      rtx           405 Feb 11 01:53 45.fixed_subreflector
#-rw-rw-r--   1 ops      rtx           193 Feb 11 01:53 45.s-band.vlbi
#-rw-rw-r--   1 ops      rtx           193 Feb 11 01:53 45.x-band.vlbi
#
#
# The MarkIV recorder (either rec1 or rec2) will be worked out by looking 
# at the file /usr2/control/prime.
# The programn will go through all available /usr2/sched/*.dsn_config files
# to find the schedule whose start and stop time are withing two hours of the 
# current time.
#
#=============================================================================
# INITIALISATION
#=============================================================================
#remove all the old (if they exist) files (if they exist) from the last 
#time auto_vlib.pl was started.
(-e "/tmp/if1_postob") && system ("rm /tmp/if1_postob");
(-e "/tmp/if2_postob") && system ("rm /tmp/if2_postob");
(-e "/tmp/pcg_startup") && system ("rm /tmp/pcg_startup");
(-e "/tmp/70m.setup") && system ("rm /tmp/70m.setup");
$X_display_name=$ARGV[0];
$X_display_name1=$ARGV[1];

# Get the local machine PCFS machine name so that we know which complex we
# are at.
open(UNAME, "/bin/uname -n|");
$PCFS_NAME=<UNAME>;
close (UNAME);
chop($PCFS_NAME);

if ($PCFS_NAME eq "pcfs40") {
	$EAC_NAME=eac40;
	$RAC_NAME=rac40;
	$LMC_NUMBER=6;
} elsif ($PCFS_NAME eq "pcfs10") {
	$EAC_NAME=eac10;
	$RAC_NAME=rac14;
	$LMC_NUMBER=7;
} elsif ($PCFS_NAME eq "pcfs60") {
	$EAC_NAME=eac60;
	$RAC_NAME=rac60;
	$LMC_NUMBER=8;
}
#-----------------------------------------------------------------------------
#=============================================================================
# FIND THE RIGHT SCHEDULE AND PARSE THE INFORMATION
#=============================================================================
($sec,$min,$hour,$mday,$month,$year,$wday,$yday,$isdst) = gmtime(time); 
if ($year <=50) { # This code won't work after the year 2050
	$year="20$year";
} else {
	$year="19$year";
}
$yday++;
$month++; #in perl months start at zero, so add one
if ( (($year/4)== int($year/4)) && !(($year/400)== int($year/400)) ) {
	$days_in_year=366;
} else {
	$days_in_year=365;
}
#convert time into seconds
#print "curr=$year,$yday,$hour,$min,$sec\n";
$current_time = $days_in_year*24*3600*$year +24*3600*$yday +3600*$hour +60*$min +$sec;

$if1_pcg_div=0;
$if2_pcg_div=0;
$schedule_found=0;
while ($filename = </usr2/sched/*.dsn_config>) {
	open(LIST,$filename);
	while ($line = <LIST>)  {
		#print "$line\n";
    		if ( $line =~ /^exp_stop_time=/ ) { 
			($dummy,$stop_time) = split (/\=/,$line);	
		}
    		if ( $line =~ /^start_time=/ ) { 
			($dummy,$start_time) = split (/\=/,$line);	
        	}
    		if ( $line =~ /^schedule=/ ) { 
			($dummy,$schedule) = split (/\=/,$line);
		}
    		if ( $line =~ /^project=/ ) { 
			($dummy,$project) = split (/\=/,$line);
		}
    		if ( $line =~ /^profile=/ ) { 
			($dummy,$profile) = split (/\=/,$line);
		}
    		if ( $line =~ /^2nd_lo_1=/ ) { 
			($dummy,$nd_lo_1) = split (/\=/,$line);
		}
    		if ( $line =~ /^2nd_lo_2=/ ) { 
			($dummy,$nd_lo_2) = split (/\=/,$line);
		}
    		if ( $line =~ /^recorder=/ ) { 
			($dummy,$recorder) = split (/\=/,$line);
		}
    		if ( $line =~ /^dss=/ ) { 
			($dummy,$dss) = split (/\=/,$line);
		}
    		if ( $line =~ /^if1=/ ) { 
			($dummy,$if1) = split (/\=/,$line);
		}
    		if ( $line =~ /^if2=/ ) { 
			($dummy,$if2) = split (/\=/,$line);
		}
    		if ( $line =~ /^sub_reflector_fixed=/ ) { 
			($dummy,$sub_reflector_fixed) = split (/\=/,$line);
		}
    		if ( $line =~ /^if1_pcg_div=/ ) { 
			($dummy,$if1_pcg_div) = split (/\=/,$line);
		}
    		if ( $line =~ /^if2_pcg_div=/ ) { 
			($dummy,$if2_pcg_div) = split (/\=/,$line);
		}
    		if ( $line =~ /^LO=/ ) { 
			($dummy,$lo) = split (/\=/,$line); # get rid of the "LO="
			($lo1,$lo2) = split (/,/,$lo);     # split the "lo1,lo2"
		}
                if ( $line =~ /^zen_cal=/ ) {
                        ($dummy,$zen_cal) = split (/\=/,$line);
                }
                if ( $line =~ /^src_cal=/ ) {
                        ($dummy,$src_cal) = split (/\=/,$line);
                }

	}
	chop ($stop_time); # use "chop" to get rid of the CR at end of line
	chop ($start_time);
	chop ($schedule);
	chop ($project);
	chop ($nd_lo_1);
	chop ($nd_lo_2);
	chop ($recorder);
	chop ($dss);
	chop ($if1);
	chop ($if2);
	chop ($sub_reflector_fixed);
	chop ($if1_pcg_div);
	chop ($if2_pcg_div);
	chop ($lo2); # don't need to chop lo1 as lo1 came from middle of the line
	chop ($zen_cal);
	chop ($src_cal);

#convert the start and stop time into seconds

($year,$yday,$hour,$min,$sec) = split (/:/, $stop_time);
#print "stop=$year,$yday,$hour,$min,$sec\n";
if ( (($year/4)== int($year/4)) && !(($year/400)== int($year/400)) ) {
	$days_in_year=366;
} else {
	$days_in_year=365;
}
$stop_time = $days_in_year*24*3600*$year +24*3600*$yday +3600*$hour +60*$min +$sec;

($year,$yday,$hour,$min,$sec) = split (/:/, $start_time);
#print "star=$year,$yday,$hour,$min,$sec\n";
if ( (($year/4)== int($year/4)) && !(($year/400)== int($year/400)) ) {
	$days_in_year=366;
} else {
	$days_in_year=365;
}
$start_time = $days_in_year*24*3600*$year +24*3600*$yday +3600*$hour +60*$min +$sec;
#print "start  =$start_time\n";
#print "stop   =$stop_time\n";
#print "current=$current_time\n";
	# if the current_time is within two hours of start_time
        if ( (($start_time-7200)<$current_time) && 
           ($stop_time>$current_time)){
                print "Found schedule with start time:$year:$yday:$hour:$min:$sec\n";
                print "Schedule name is: $schedule\n";
                print "Project name is: $project\n";
		print "dss=$dss\n";
		print "lo1=$lo1\n";
		print "lo2=$lo2\n";
		print "recorder:$recorder\n";
		$schedule_found=1;
		last; # stop at the first schedule found
	}
}

if ($schedule_found==0) {#if the DSS has not been set then there is no schedule.
	print "Did not find a schedule to run\n";
	print "This window will disappear in 5 seconds\n";
	sleep 5;
	exit; 
} 

#if the antenna is a 70m then pop up window to ask CC/P1/P2 pointing mode and
# at lest for DSS-43, the hydraulic mode is: SP1=P1, SP2=P2, SP3=CC
if ( $dss==43 || $dss==14 || $dss==63) {
        #call an tcl/tk window and save the asc_mode and SP-mode
        #into /tmp/70m.setup. Read it later.
        system("/usr2/st/scripts/tk.70m -d $X_display_name1 -geom +100+100");
}
#=============================================================================
# GET THE UTC-UT1 "DELUT" from /usr2/control/dsn_band_setup/ut1-utc.out
#=============================================================================
open (DELUT_FILE,"</usr2/control/dsn_band_setup/ut1-utc.out")
                || die "can't open /usr2/control/dsn_band_setup/ut1-utc.out\n" ;
$delut_found=0;
while (<DELUT_FILE>) {
	($dum, $yr, $mo, $da, $MJD, $x, $error, $y, $error, $delut, $error)=
								   split(/\s+/);
	if ($yr <=50) { # This code won't work after the year 2050
        	$yr="20$yr";
	} else {
        	$yr="19$yr";
	}
	if (($yr==$year) && ($mo==$month) && ($da==$mday)){
		#we have found the right year/month/day delut value so
		#  exit the loop
		$delut=int($delut*1000);
		print "Delut (UTC-UT1)=$delut\n";
		$delut_found=1;
		last;
	}
}
if ($delut_found==0) {
        system("/usr2/st/scripts/tk.prompt -d $X_display_name1 \"Could not find UTC-UT1 DELUT value. Please enter manually: AP ACS DELUT=xx\"\n");

}
close (DELUT_FILE);

#=============================================================================
# Create the SNP and PRC if running an antenna calibration
#=============================================================================
if ($project eq "cal") { # if we are doing an antenna calibration
	if ($lo1 > $lo2 ){#always do the calibration at the highest LO frequency
		&find_band_name ($lo1); #get the band name in text
	} else {
		&find_band_name ($lo2); #get the band name in text
	}
	#get the first pointing source from the cal_sequence file
        #create the CAL_SEQ file
        open (CAL_SEQ,"</usr2/sched/$schedule.cal_sequence")
                || die "can't open /usr2/sched/$schedule.cal_sequence\n" ;
	# keep reading source commands until a time is found in the boresight 
	# command that is in the future, then escape the while loop.
	while (<CAL_SEQ>) {
		if (/source/) {
			$pointing_source=$_;
			chop($pointing_source);
		}
        	if (/boresight/) {
                	($dummy, $boretime )= split (/=/);
			$boretime =~ s/://g;
                	($sec,$min,$hour,$mday,$month,$year,$wday,$yday,$isdst) = gmtime(time);
                	if ($year <=50) { # This code won't work after the year 2050
                        	$year="20$year";
                	} else {
                        	$year="19$year";
                	}
                	# pad the yday with a zero if less than 100
                	$yday++;
                	$yday=sprintf("%.3d",$yday);
                	$month++; #in perl months start at zero, so add one
                	# pad the min with a zero if less than 10
                	$min=sprintf("%.2d",$min);
                	# pad the hour with a zero if less than 10
                	$hour=sprintf("%.2d",$hour);
                	#	 pad the hour with a zero if less than 10
                	$sec=sprintf("%.2d",$sec);
                	$current_time = "$year$yday$hour$min$sec";

                	if ( $current_time < $boretime ) {
				print "found time that in the future\n";
				last; # exit while loop
                	}
		}
	}
        #create the SNAP file
        open (SNP_FILE,">/usr2/sched/$schedule.snp.orig")
                || die "can't open /usr2/sched/$schedule.snp.orig\n" ;
        print SNP_FILE "$pointing_source\n";
        print SNP_FILE "setup01=0\n";
        print SNP_FILE "ready\n";
        print SNP_FILE "sy=/usr2/st/scripts/boresight.pl $schedule &\n";
        print SNP_FILE "halt\n";
        close (SNP_FILE);

        #create the PRC file from /usr2/control/band_setup/$dss.$band.prc.cal
        system("cp /usr2/control/dsn_band_setup/$dss.$band.prc.cal /usr2/proc/$schedule.prc.orig");
} 

#=============================================================================
# OPEN THE FILES
#=============================================================================
#put the schedule name into the default schedule file
open (DSN_STARTUP_FILE,">/usr2/proc/dsnstart.prc") 
			|| die "can't open /usr2/proc/dsnstart.prc\n" ;

print DSN_STARTUP_FILE "define  dsnstart      00000000000x\n"; 
# insert here all the common commands for all bands/antennas
print DSN_STARTUP_FILE "log=$schedule\n"; 
# Now insert the line to run the schedule!!!!
#-------------------------------------------
print DSN_STARTUP_FILE "schedule=$schedule,#1\n";
print DSN_STARTUP_FILE "enddef\n";
close DSN_STARTUP_FILE;

#first make a copy of the original file if it doesnt already exist
if (!(-e "/usr2/sched/$schedule.snp.orig")) {
        system ("mv /usr2/sched/$schedule.snp /usr2/sched/$schedule.snp.orig");
}
open (SNAP_FILE,">/usr2/sched/$schedule.snp")
                        || die "can't open /usr2/sched/$schedule.snp\n" ;

#first make a copy of the original PRCfile if it doesnt already exist then
# append to the original procedures.
if (!(-e "/usr2/proc/$schedule.prc.orig")) {
        system ("mv /usr2/proc/$schedule.prc /usr2/proc/$schedule.prc.orig");
}
# open the PRC file for writing
open (PRC_FILE,">/usr2/proc/$schedule.prc")
                        || die "can't open /usr2/proc/$schedule.prc\n" ;

#open the original PRC file for reading, so that lines can be commented out
open (PRC_ORIG_FILE,"</usr2/proc/$schedule.prc.orig")
                 || die "can't open /usr2/proc/$schedule.prc.orig\n" ;
while(<PRC_ORIG_FILE>) {
	if ($recorder =~/s2/ ){ 
		# if S2 then comment out enable,pass,stack in proc file
		if ((/^enable/) || (/^pass/) || (/^stack/) ) {
			print PRC_FILE "\"$_";
		} else {
			print PRC_FILE $_;
		}
	} else { # if MarkIV don't comment anything out
		print PRC_FILE $_;
	}
}
close (PRC_ORIG_FILE);

print PRC_FILE "define  dsn_ready     00000000000x\n";

#open up a file that will contain the PCG commands. This file will later be
# appended to the "schedule".prc file. This is because we want the PCG's to
# turn on AFTER the calibrations have been done.
open (PCG_STARTUP_FILE,">/tmp/pcg_startup") 
			|| die "can't open /tmp/pcg_startup\n" ;

#=============================================================================
# SUBROUTINES
#=============================================================================
# This subroutine moves the contents of the "band_file_name" into the 
# dsnstart.prc. Both "band_file_name" and "dsnstart.prc" are passed as
# parameters.
sub process_band_setup{
		$band_file_name=$_[1];
		$PRC_FILE=$_[0];
                open( BANDSETUP, "<$band_file_name") || 
				die "could not open $band_file_name";
                while (<BANDSETUP>) {
			if (!/#/) { # if line doesn not start with "#" 
                        	print PRC_FILE $_;
			}
                }
}

#----------------------------------------------------------------------------
# This subroutine moves the contents of the "pcg.band_file_name" into a file
# so that later it can be concatenated to dsnstart.prc. Both 
# "pcgband_file_name" and "dsnstart.prc" are passed as parameters.
sub process_pcg_setup{
                $PCG_STARTUP_FILE=$_[0];
                $band_file_name=$_[1];
                $if_num=$_[2];
                open( PCGSETUP, "<$band_file_name") ||
                                die "could not open $band_file_name";
                while (<PCGSETUP>) {
                        if (!/#/) { # if line does not start with "#"
				if ($if_num==1) {
					s/DIV_NUM/$if1_pcg_div/;
				} else { #must be IF2
					s/DIV_NUM/$if2_pcg_div/;
				}
                                print PCG_STARTUP_FILE $_;
                        }
                }
}
#----------------------------------------------------------------------------
# This subroutine converts an LO frequency into the corresponding text band name
sub find_band_name {
        $lo=$_[0]; # lo is the first argument passed to the subroutine
        if ($lo>1200 && $lo < 1700) {
                $band="l-band";
        } elsif ($lo>1800 && $lo < 3000) {
                $band="s-band";
        } elsif ($lo>3000 && $lo < 7000) {
                $band="c-band";
        } elsif ($lo>7000 && $lo < 9000) {
                $band="x-band";
        } elsif ($lo>9000 && $lo < 26000) {
                $band="k-band";
        } else {
                $band="unknown-band";
        }
}
#----------------------------------------------------------------------------
# This subroutine sets the IF matrix switch numbers based upon the DSS and
# LO1 and LO2 settings. It will add the PCG startup commands if the PCG divider
# is greater than 0. Also the RF/IF chain will be setup for the particular band
# and the pointing model set. We only need one pointing model, so when we are 
# doing dual band S/X only the higher frequency pointing model is used. It 
# could be S/X or X/S on the IF channels so some intelligence needs to be 
# built in.
sub process_if_band{
    #now process the IF's for the 70m antenna
    #------------------------------------------
    # work out the IF1 and IF2 and other band related procedures
    if ( $dss==43 || $dss==14 || $dss==63) { 
	if ($lo1>1200 && $lo1 < 1700) {
		$if1_num=1; # 70MS if input with L-band
		&process_band_setup(PRC_FILE,
			"/usr2/control/dsn_band_setup/$dss.l-band.$project");
		system("cat /usr2/control/dsn_band_setup/$dss.l-band.postob.$project > /tmp/if1_postob");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.l-band.pcg.$project",1);
		}
	} elsif ($lo1>1800 && $lo1 < 3000) {
		$if1_num=1; # this is 70MS RCP-VRD
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.s-band.$project");
		# if lo2 is X-band then do not put in pointing model setup
		# in other words use the pointing model of the higher freq lo
		if ($lo1 >= $lo2) { 
		   &process_band_setup(PRC_FILE,
                     "/usr2/control/dsn_band_setup/$dss.s-band.point.$project");
	
		}
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.s-band.pcg.$project",1);
		}
	} elsif ($lo1>3000 && $lo1 < 7000) {
		$if1_num=0; # future C-band
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.c-band.$project");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.c-band.pcg.$project",1);
		}
	} elsif ($lo1>7000 && $lo1 < 9000) {
		$if1_num=9; # 70MX if input LCP-VRD
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.x-band.$project");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.x-band.pcg.$project",1);
		}
	} elsif ($lo1>9000 && $lo1 < 26000) {
		$if1_num=8; # MMS1 if input
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.k-band.$project");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.k-band.pcg.$project",1);
		}
	} else {
		$if1_num=1 # make this the default
	}

	# Check out IF2 for 70m antenna

	if ($lo2>1200 && $lo2 < 1700) {
		$if2_num=0; # no L-band on IF2
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.l-band.pcg.$project",2);
		}
	} elsif ($lo2>1800 && $lo2< 3000) {
		$if2_num=0; # no S-band on IF2
                #if this is X/S VLBI then include S-band RF setup
                if ($lo1 > $lo2) {
                        &process_band_setup(PRC_FILE,
                           "/usr2/control/dsn_band_setup/$dss.s-band.$project");
                }
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.s-band.pcg.$project",2);
		}
	} elsif ($lo2>3000 && $lo2< 7000) {
		$if2_num=0; # no C-band on IF2
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.c-band.pcg.$project",2);
		}
	} elsif ($lo2>7000 && $lo2 < 9000) {
		$if2_num=1; # 70MX IF2 input
                # if lo2 is X-band then put in X-band pointing model/RF setup
                # in other words use the pointing model of the higher freq lo
                if ($lo1 < $lo2) {
                   &process_band_setup(PRC_FILE,
                     "/usr2/control/dsn_band_setup/$dss.x-band.$project");

                }
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.x-band.pcg.$project",2);
		}
	} elsif ($lo2>9000 && $lo2 < 26000) {
		$if2_num=8; # MMS2 if input
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.k-band.pcg.$project",2);
		}
	} else {
		$if2_num=1 # make this the default
	}
}

#now process the IF's for the HEF 34m antenna
#------------------------------------------
if ( $dss==15 || $dss==45 || $dss==65) {
	if ($lo1>1200 && $lo1 < 1700) {
		$if1_num=0; # No L-band on HEF antenna
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.l-band.$project");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.l-band.pcg.$project",1);
		}

	} elsif ($lo1>1800 && $lo1 < 3000) {
		$if1_num=1; # this is 34MS LCP
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.s-band.$project");
                # if lo2 is X-band then do not put in pointing model setup
                # in other words use the pointing model of the higher freq lo
                if ($lo1 >= $lo2) {
                   &process_band_setup(PRC_FILE,
                     "/usr2/control/dsn_band_setup/$dss.s-band.point.$project");

                }
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.s-band.pcg.$project",1);
		}

	} elsif ($lo1>3000 && $lo1 < 7000) {
		$if1_num=0; # No C-band on HEF antenna
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.c-band.$project");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.c-band.pcg.$project",1);
		}
	} elsif ($lo1>7000 && $lo1 < 9000) {
		$if1_num=2; # 34MX if input
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.x-band.$project");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.x-band.pcg.$project",1);
		}
	} elsif ($lo1>9000 && $lo1 < 26000) {
		$if1_num=0; # NO K-band on HEF
                &process_band_setup(PRC_FILE,
                        "/usr2/control/dsn_band_setup/$dss.k-band.$project");
		if ($if1_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.k-band.pcg.$project",1);
		}
	} else {
		$if1_num=1 # make this the default
	}

	# Check out IF2 for HEF antenna

	if ($lo2>1200 && $lo2 < 1700) {
		$if2_num=0; # no L-band on IF2 for HEF
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.l-band.pcg.$project",2);
		}
	} elsif ($lo2>1800 && $lo2< 3000) {
                $if2_num=1; # 34MS IF2 input
		#if this is an X/S VLBI then include S-band RF setup
		if ($lo1 > $lo2) {
                	&process_band_setup(PRC_FILE,
                           "/usr2/control/dsn_band_setup/$dss.s-band.$project");
		}
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.s-band.pcg.$project",2);
		}
        } elsif ($lo2>3000 && $lo2< 7000) {
                $if2_num=0; # no C-band on IF2 for HEF
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.c-band.pcg.$project",2);
		}
        } elsif ($lo2>7000 && $lo2 < 9000) {
                $if2_num=2; # 34MX IF2 input
                # if lo1 is S-band then put in X-band pointing model/RF setup
                # in other words use the pointing model of the higher freq LO
                if ($lo1 < $lo2) {
                   &process_band_setup(PRC_FILE,
                     "/usr2/control/dsn_band_setup/$dss.x-band.$project");

                }
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.x-band.pcg.$project",2);
		}
        } elsif ($lo2>9000 && $lo2 < 26000) {
                $if2_num=0; # NO K-band on HEF
		if ($if2_pcg_div>0) {
			&process_pcg_setup(PCG_STARTUP_FILE,
                          "/usr2/control/dsn_band_setup/$dss.k-band.pcg.$project",2);
		}
	} else {
		$if2_num=1 # make this the default
        }
}
close (PCG_STARTUP_FILE);
} #end of process_if_band procedure

#=============================================================================
# MAIN PROGRAM
#=============================================================================
# get the prime recorder, either "rec1" or "rec2" from the file 
# /usr2/control/prime which is editted by local station maintenance.
print "rec=$recorder\n";
if ($recorder eq "mk4") {
	open (PRIME,"</usr2/control/prime") || 
				die "can't open /usr2/control/prime";
	while (<PRIME>) {
		if ( !/#/) {	# if the line is not a comment
			$recorder=$_;
			chop ($recorder);
		}
	}
	print "rec=$recorder\n";
	system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'Has the tape been degaussed and prepassed? 
If not degauss and pre-pass now and press Continue after prepass has finished.
(it will take 20 minutes!!!)
Make sure recorder is in REMOTE. Note down tape number.
Affix experiment label onto tape (label supplied by M&I)'");
}
if (($recorder eq "s2") && !($project eq "cal")) {
	system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'Please insert rewound S2 video tapes and note tape label.'");
	system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'Please Start S2 sampler and use profile: $profile
Press <ENTER> at SERIAL SETUP screen
Press S to setup the sampler - should see Processing
Then run 2nd lo control from RAC pulldown menu and set:
Synth1 Freq:$nd_lo_1 MHz (+7dBm)
Synth1 Freq:$nd_lo_1 MHz (+7dBm)'");
}
system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'In block 0 rack: 
Please patch Power Meter 1 to either :
Splitter output for L-band,70MS,70MX,34MS or 34MX via the filter.
(use filter 325 for X-band or 295 for S-band/L-band)'");


# now let's edit the SNAP file to configure it for automatic startup:
$first_time=0;
open(SNAP_IN,"/usr2/sched/$schedule.snp.orig") 
		|| die "cannot open /usr2/sched/$schedule.snp.orig";
while (<SNAP_IN>) {

    if (/ready/ && $first_time==0) {  # find the first "ready" and insert this:

        # read the 70 meter antenna hydraulic and pointing mode from the TK
        #  window results.
        open(LIST,"/tmp/70m.setup") || die "can't open /tmp/70m.setup\n" ;
        $sp_mode = <LIST>;
        $acs_mode = <LIST>;
        chop($sp_mode);
        chop($acs_mode);
        close (LIST);

        # Now put all the antenna startup commands in the file
        open(LIST,"/usr2/control/dsn_band_setup/$dss.antenna.startup") ||
           die "can't open /usr2/control/dsn_band_setup/$dss.antenna.startup\n";
        while (<LIST>) {
                if (!/#/) { # if line doesn not start with "#"
                        if (/\*MODE/) {
                                s/\*MODE/$acs_mode/;
                        } elsif (/\*SPX/) {
                                s/\*SPX/$sp_mode/;  
                        }
                        print PRC_FILE $_;
                }
        }

        $first_time=1;  # make sure we run this only once.
	print PRC_FILE "antenna=AP ACS SIDE\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS SCANCL\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS HAPCL\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS ELPCL\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS AXPCL\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS AZPCL\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS DEPCL\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS HXPCL\n";
	print PRC_FILE "!+3s\n";
	print PRC_FILE "antenna=AP ACS DELUT=$delut\n";
	print PRC_FILE "!+3s\n";

	&process_if_band;

        #print PRC_FILE "sy=tk.prompt \"Press continue after VCO/IFD levels set\" 1 &\n";
	print PRC_FILE "sy=/usr2/st/scripts/adjust_ifd.pl $schedule &\n";
        print PRC_FILE "halt\n";


        if ($zen_cal==1) {
                print PRC_FILE "zen_cal\n";   
        } else {
                print PRC_FILE "\"\n";
                print PRC_FILE "\" There is no Zenith Calibration\n";
                print PRC_FILE "\"\n";
        }
        if ($src_cal==0) {
                print PRC_FILE $source_line;
		#print PRC_FILE "!+1s\n";
                #print PRC_FILE $source_line;
		#print PRC_FILE "!+1s\n";
                #print PRC_FILE $source_line;
		#print PRC_FILE "!+1s\n";
        } else {
                # The source command for the SRC_CAL is assumed to have been
		# inserted at the end of the zen_cal procedure.
        }

        print PRC_FILE "sy=/usr2/st/scripts/tk.prompt \"Sound safety warnings\" 1 &\n";
        print PRC_FILE "halt\n";
        print PRC_FILE "sy=/usr2/st/scripts/tk.prompt \"Select link x, do AP ACM ADA TRACK\" 1 &\n";
        print PRC_FILE "halt\n";
	if ($sub_reflector_fixed==1) {
		# this call a procedure which positions and shuts down the 
		# subreflector if required.
		print PRC_FILE "fix_sub\n";
	}
        if ($src_cal==1) { # put in the original source after the src_cal
               #print PRC_FILE "sy=/usr2/st/scripts/tk.prompt \"press cont when antenna is on point \" 1 &\n";

		print PRC_FILE "sy=/usr2/st/scripts/wait_on_source.pl $schedule &\n";
                print PRC_FILE "halt\n";
                print PRC_FILE "src_cal\n";   
                print PRC_FILE "sy=/usr2/st/scripts/wait_finish_bore.pl $schedule &\n"; 
                print PRC_FILE "halt\n";
		#wait 10 seconds to check source on xplot after boresight
                print PRC_FILE "!+10s\n";
                print PRC_FILE $source_line;

        } else {
                print PRC_FILE "\"\n";
                print PRC_FILE "\" There is no Source Calibration, going straight to the first source\n";
                print PRC_FILE "\"\n";
        }
        if (($if1_pcg_div>0) || ($if2_pcg_div>0)){
                # this calls the PCG trun on procedure 
                print PRC_FILE "turn_on_pcg\n";
        }

        # the next line print PRC_FILEed will be the "ready"
        print SNAP_FILE "dsn_ready\n";
    	#insert a preob in the S2 snap file as drudg is currently 
	#leaving them out. This is also done below.
    	if (($recorder eq "s2") && !($project eq "cal")){ 
		print SNAP_FILE "preob\n"; 
	}
    }
    if (!/source/) {print SNAP_FILE $_;}
    if (/source/ ) {
        if($first_source==0) {  # Find the first "source=" but 
                                # don't print PRC_FILE first the source out yet.
                $first_source=1;# Make sure we run this only once.
                $source_line=$_;
                print SNAP_FILE $_; # put the source= into the snap to preserve
				    # the line numbers of the snap file.
		$vco_setup=<SNAP_IN>; #save the "setup01=0" or "sx2a1=1" etc.
                print SNAP_FILE $vco_setup; # print out "setup01=0" or "sx2a1=1"
        } else {
                print SNAP_FILE $_;
        }
    }
    #insert a preob in the S2 snap file as drudg is currently leaving them out.
    # this is also specially done after "dsn_ready" - see above
    if (($recorder eq "s2") && !($project eq "cal") && (/source/) && 
							($first_time==1)) {
        print SNAP_FILE "preob\n";
    }

    if (/unlod/ && EOF) {
                #print PRC_FILE "post_cal"
    }
}
print SNAP_FILE "post_cal\n";
print SNAP_FILE "dsn_post_cal\n";
close (SNAP_FILE);

print PRC_FILE "$vco_setup";
print PRC_FILE "ready\n";
print PRC_FILE "enddef\n";

#now insert the PCG commands if required(to be turned on after Tsys calibration)
#---------------------------------------------------------------------------
if ($if1_pcg_div>0) {
        print PRC_FILE "define  turn_on_pcg   00000000000x\n";
        open(LIST,"/tmp/pcg_startup");
        while (<LIST>)  {
                (!/#/) && print PRC_FILE $_;
        }
        close (LIST);
        print PRC_FILE "sy=tk.prompt \"Set PCG 10dB above nf with RB=3KHz\" 1 &\n";
	print PRC_FILE "halt\n";
	
        print PRC_FILE "enddef\n";
}

#write proc to fix the subreflector if required (track command already sent)
#-----------------------------------------------------
if ($sub_reflector_fixed == 1) { # the subreflector is fixed at 45 degrees
        print PRC_FILE "define  fix_sub       00000000000x\n";
        open(LIST,"/usr2/control/dsn_band_setup/$dss.fixed_subreflector");
        while (<LIST>)  {
                (!/#/) && print PRC_FILE $_;
        }
        close (LIST);
        print PRC_FILE "enddef\n";
}

#write proc for both if1 and if2 band specific post_cal
#-----------------------------------------------------
print PRC_FILE "define  dsn_postcal   00000000000x\n";
open(LIST,"/tmp/if1.postob");
while (<LIST>)  {
        (!/#/) && print PRC_FILE $_;
}
close (LIST);
open(LIST,"/tmp/if2.postob");
while (<LIST>)  { 
        (!/#/) && print PRC_FILE $_;
}
close (LIST);
print PRC_FILE "sy=tk.prompt 'VLBI has Finished. Press STOP LMC if no antenna using EAC' 1 &\n";
print PRC_FILE "halt\n";
print PRC_FILE "sy=tk.prompt \'send antenna to stow\' 1 &\n";
print PRC_FILE "halt\n";
print PRC_FILE "sy=tk.prompt 'tear down link on CMC. Thank you. Goodbye.' 1 &\n";
print PRC_FILE "halt\n";
print PRC_FILE "enddef\n";

#creat null procedure so that ERROR is not given if operator forces onpoint by
#typing "onpoint"
print PRC_FILE "define  onpoint       00000000000x\n";
print PRC_FILE "enddef\n";
#creat null procedure so that ERROR is not given if operator forces onpoint by
#typing "borefinish"
print PRC_FILE "define  borefinish    00000000000x\n";
print PRC_FILE "enddef\n";
print PRC_FILE "define  restart       00000000000x\n";
print PRC_FILE "sy=/usr2/st/scripts/restart.pl $schedule\n";
print PRC_FILE "enddef\n";
close PRC_FILE;

# prompt the operator to put the antenna in the link
#system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'Run antenna startup macro until just before issuing TRACK and then press continue'");

sleep 1; # leave some space between when the windows pop up

# Now that we know the Project name we can start the EAC software:
print "starting LMC on $EAC_NAME on display=$X_display_name project=$project\n";
system ("rsh $EAC_NAME '/usr/local/bin/start_eac $X_display_name $project' &");
#we could check here that mds/stfmain/oci/xant are all running beore continuing

print "waiting 40 seconds for EAC programs to finish running\n";
sleep 40;

# prompt the operator to put the antenna in the link1
if ( $dss==43 || $dss==14 || $dss==63) {
	system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'On CMC dissolve current link: 
REMx ALL
CNFx    (where x is old LMC link number)
ADDx UPLNK=$dss"."L PASS1=0 SC1=50 AP1 L$LMC_NUMBER RCCy 
CNFx NOW
(where x is the link number, and y is the BVR RCC number)'");
system("/usr2/st/scripts/tk.prompt  -d $X_display_name1 -geom +100+100 'Put ANT70 display on CMC: AP1 D ANT70 xx'");
} elsif ( $dss==45 || $dss==15 || $dss==65) {
	system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'On CMC do: 
ADDx UPLNK=$dss"."E PASS1=0 SC1=50 AP1 L$LMC_NUMBER RCCy 
CNFx NOW
(where x is the link number, and y is the BVR RCC number)'");
system("/usr2/st/scripts/tk.prompt  -d $X_display_name1 -geom +100+100 'Put ANTHE dsiplay on CMC: AP1 D ANTHE xx'");
} else {
	system("/usr2/st/scripts/tk.prompt -d $X_display_name1 -geom +100+100 'The antenna is not known'"); 
	exit;
}

system("/usr2/st/scripts/tk.prompt  -d $X_display_name1 -geom +100+100 'Press Continue when the Antenna Control window displays DSS-$dss'");

# run the PCFS field system
system("/usr2/st/scripts/startFS $X_display_name1 $recorder $if1_num $if2_num ");
sleep 5; 	#this sleep is useful to pause before the window goes away.
# end
