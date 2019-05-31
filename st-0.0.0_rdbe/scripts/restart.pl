#!/usr/bin/perl 
# This program finds the correct line number in the SNAP file to
# restart the schedule. It will also position the markIV tape in the
# correct place.
#
# Firstly the program halts the schedule with an "inject_snap halt" 
# The program is called from a snap file by:
#	sy=/usr2/st/scripts/restart.pl schedule &
# Paul Harbison Canberra Deep Space Communications Complex April 1998.
#
# The programs first halts the schedule then scans through the SNAP file 
# until it finds a time that is in the future. It calculates the tape footage 
# by looking for "st=for", "st=rev" and "et" lines and using the tape speed and 
# the time of the start and stop to get the correct footage. It the compares 
# the current tape footage against the required calculated footage to see how 
# long the "tapepos=" is going to take to get into correct position. If the 
# required position cannot be reached in time, then that time is skipped and 
# the next time read and the whole process done again. The program then waits 
# until the correct footage is reached and then issues a 
# "schedule=<sched_name>,#<line_number>" where line number is the source 
# command. The source command is chosen so that if the "!DOYHHMMSS" command is
# immediately after an "st=" then the source= after the "et" is chosen. If 
# the "!DOYHHMMSS" command is before the "st=" and after a "source=" then the 
# previous "source+" is chosen.  
#
#======================================================================
# Here are the configurable variables
#======================================================================

$absolute_start=110; # starting position of tape after "label=" command.
$tape_pos_speed=270; # tape speed that PCFS uses in tapepos in feet/sec

#======================================================================
# first get log file name from PCFS shared memory using Chuck Naudet's
# getlogfile.pl script.

     $|=1;
     unshift (@INC,"/usr/lib/perl4/sys");
     print "@INC\n";
     require "ipc.ph";
     require "shm.ph";
     require "fs/params.ph";
     require "fs/fs_types.ph";
     require "fs/fscom.ph";
     require "fs/shm_addr.ph";

     $ipckey=1;
     $shmid = shmget($ipckey,4096,0);
     shmread($shmid,$string,264,8) || die "shmread: $! \n";
     print "Data from shared memory =$string \n";
     $logfile = $string;
     print "logfile=$logfile\n";
#======================================================================
# get the schedule name
$schedule=$ARGV[0];
#======================================================================
# open the file containing the list of sources/fluxes/expiry times
#
open (SNAP_FILE,"</usr2/sched/$schedule.snp")
                || die "can't open /usr2/$schedule.snp\n" ;

$source_found=0;
$flux1_found=0;
$flux2_found=0;
$current_tape_found=0;
$first_time=1;
$starting_footage=$absolute_start;
$temp_tape_feet=$absolute_start;
$tape_feet=$absolute_start;

#=======================================================================
# get the current time
#=======================================================================
($sec,$min,$hour,$mday,$month,$year,$wday,$yday,$isdst) = gmtime(time);
# the following entries for testing only
$yday++;
$yday=347;
$hour=19;
$min=7;
$sec=50;

$current_time_string="$yday:$hour:$min:$sec";
$current_time = 24*3600*$yday +3600*$hour +60*$min +$sec;

open(LOG,"tail -1 -f /usr2/log/$logfile.log |") 
			|| die "Can't open /usr2/log/$logfile.log tail pipe \n";

#first halt the schedule
print"/usr2/st/bin/inject_snap halt\n";
#system("/usr2/st/bin/inject_snap halt");

#=======================================================================
# read through snap file to find time in the future 
#=======================================================================
$line_number=0; # reset line number counter
while (<SNAP_FILE>) {
	#print "$line_number:$_\n";
	$line_number++;
	if (/^source/) {
		$source=$_;
		$source_line=$line_number;
		#print "$line_number:found source\n";

	}
	if (/^st=for,/ && ( /slp/ || /lp/) ) {
		$recorder="s2";
		print "rec=$recorder\n";
	}
	if (/^\!/ && !/^\!\+/ ) { #if a time field and not a "!+3s" field
		$snap_time= $_;
		$snap_time_string= $_;
		chop($snap_time_string);
		$snap_time=~ s/\!//; # get rid of leading "!"
		chop ($snap_time);
		$yday=substr($snap_time,0,3);
		$hour=substr($snap_time,3,2);
		$min=substr($snap_time,5,2);
		$sec=substr($snap_time,7,2);
		$snap_time = 24*3600*$yday +3600*$hour +60*$min +$sec;
		# if markIV then find out if we have enough time to get to the
		# tapepos.
		if ($recorder ne "s2") {
			#get current tape position 
			if (!$current_tape_found) {
                    	    print"/usr2/st/bin/inject_snap tape\n";
                    	    system("/usr2/st/bin/inject_snap tape");
                    	    while (<LOG>) {
                        	chop($_);
                        	@test =  split(/[\$:&#;,\/? ]/);
                        	if (/\/tape\//) {
                                	print "input=$_\n";
                                	#print "1=$test[1],3=$test[3]\n";
                                	$current_footage=$test[3];
					$current_tape_found=1;
					last;
				} else {
					print"/usr2/st/bin/inject_snap tape\n";
                        		system("/usr2/st/bin/inject_snap tape");
					sleep 5;
				}
			    }
			}
			#$current_footage=7100;
			#print "current_footage is $current_footage\n";
                    # wait till tape gets to required footage
                    if (($direction eq "forward") && ($tape_speed>0)){
                        $temp_tape_feet= $starting_footage + int(($tape_speed * 
                                        ($snap_time- $tape_start_time))/12);
                        print" ! for adjust=",int(($tape_speed *
                                        ($snap_time- $tape_start_time))/12),"temp_tape_feet= $temp_tape_feet\n";
                    } elsif (($direction eq "reverse") && ($tape_speed>0)) {
                        $temp_tape_feet= $starting_footage - int(($tape_speed * 
                                        ($snap_time- $tape_start_time))/12);
                        print" ! rev adjust=",int(($tape_speed *
                                        ($snap_time- $tape_start_time))/12),"temp_tape_feet= $temp_tape_feet\n";
                    }

			$time_gap=int(12*($current_footage - $temp_tape_feet)/
							$tape_pos_speed);
			#take absolute value of time_gap. (no abs in perl 4) 
			if ($time_gap<0) {$time_gap=$time_gap*(-1); }
			print "snap line=$line_number: time gap=$time_gap seconds: current_footage is $current_footage: target footage is $temp_tape_feet \n";
		} else {
			$time_gap=30;
		}
		if (( $snap_time-($time_gap+10)) > $current_time) { 
		  print "$line_number:found future :snap time=$snap_time_string , current time=$current_time_string\n";
		  #send the correct source
		  print "/usr2/st/bin/inject_snap $source";
		  #system("/usr2/st/bin/inject_snap $source");
		  if ($recorder ne "s2") { # only position tape for MarkIV
 
		    print"/usr2/st/bin/inject_snap tape\n";
		    system("/usr2/st/bin/inject_snap tape");

		    print"seconds since beginning of tape=".($snap_time-$tape_start_time)."\n";
		    print "/usr2/st/bin/inject_snap tapepos=$temp_tape_feet\n";
		    #system("/usr2/st/bin/inject_snap tapepos=$temp_tape_feet");

		    # if "et" was the last tape command then goto the next 
		    # source= command.
		    if (!($last_tape_command =~ /^et/) ) {
			$source_line=$line_number+1;
			print "st= is last command skipping to next source=\n";
			while (<SNAP_FILE>) { 
				(/source=/) && last;
				#print "$_\n";
				$source_line++;
			}
		  	#send the correct source
		  	print "/usr2/st/bin/inject_snap $_";
		  	#system("/usr2/st/bin/inject_snap $_");
		    }
		    print "last_tape_command is $last_tape_command \n";
		    print "/usr2/st/bin/inject_snap schedule=$schedule,#$source_line\n";
		    while (<LOG>) {
			chop($_);
			@test =  split(/[\$:&#;,\/? ]/);	
			if (/\/tape\//) {
				#print "input=$_\n";
				#print "1=$test[1],3=$test[3]\n";
				$tape_footage=$test[3];
				#print "tape_footage=$tape_footage\n";
				#print "target tape feet=$temp_tape_feet\n";
				$feet_diff=$tape_footage - $temp_tape_feet;
				#perl 4doesn't like the "abs()"
				if ($feet_diff<0) {$feet_diff=$feet_diff * -1; }
				if (($feet_diff) < 100 ){
					print "breaking out\n";
					last; #break out of while loop
				}
				print "snap line=$source_line: target feet=$temp_tape_feet: $feet_diff feet to go.\n";
				#print"/usr2/st/bin/inject_snap tape\n";
				system("/usr2/st/bin/inject_snap tape");
				sleep 5;
			}
		    }
		  } else {
			print "will not make the tapepos in time. skipping!\n"
		  }
		# some SNAP files do not have st=for all the way through them
		print("/usr2/st/bin/inject_snap $last_tape_command");
		#system("/usr2/st/bin/inject_snap $last_tape_command");
		print "/usr2/st/bin/inject_snap schedule=$schedule,#$source_line\n";
		#system("/usr2/st/bin/inject_snap schedule=$schedule,#$source_line");
		close (LOG);
		exit;
		}
	}
	if (/^st=for,/ && !/slp/ ) { # if forward tape movement is started. 
	    if (!/slp/) { # Tape position is not needed for S2.
		($dummy, $st_tape )= split (/=/);
		($dummy, $tape_speed )= split (/,/,$st_tape);
		print "speed=$tape_speed\n";
		print "tape_start_time=$snap_time\n";
		$tape_start_time=$snap_time;
		$tape_start_footage=$tape_feet;
		$starting_footage=$tape_feet;
		if (($direction eq "reverse") || ($first_time==1)){ 
			# if this is the first "st=for"
			# assume xx feet of tape from the start
			$starting_footage=$absolute_start; 
			$first_time=0;
			print "$first_time $direction ONCE Going in FORWARD starting at $starting_footage\n";
		}
		$direction="forward";
		print "Going in FORWARD starting at $starting_footage\n";
	   }
	   $last_tape_command=$_;
	}
        if (/^st=rev,/ && !/slp/ ) { # if reverse tape movement is started.
                                     # Tape position is not needed for S2.
                ($dummy, $st_tape )= split (/=/);
                ($dummy, $tape_speed )= split (/,/,$st_tape);
                print "speed=$tape_speed\n";
                print "tape_start_time=$snap_time\n";
                $tape_start_time=$snap_time;
		$tape_start_footage=$tape_feet;
		$starting_footage=$tape_feet;
		if ($direction eq "forward") { # if this is the first "st=rev"
			$starting_footage=$tape_feet;
		}
		$direction="reverse";
		print "Going in REVERSE starting at $starting_footage\n";
		$last_tape_command=$_;
        }
	if (/^et/ ) { # if we find an "end tape" 
		print "tape_speed=$tape_speed\n";
		print "End of tape\n";
		$tape_stop_time=$snap_time;
		$starting_footage=$tape_start_footage;
                if (($direction eq "forward") && ($tape_speed>0)){
                        $tape_feet= $starting_footage + int(($tape_speed *
                                        ($snap_time- $tape_start_time))/12);
                        print"for adjust=",int(($tape_speed *
                                        ($snap_time- $tape_start_time))/12),"tape_feet= $tape_feet\n";
                    } elsif (($direction eq "reverse") && ($tape_speed>0)) {
                        $tape_feet= $starting_footage - int(($tape_speed *
                                        ($snap_time- $tape_start_time))/12);
                        print"rev adjust=",int(($tape_speed *
                                        ($snap_time-$tape_start_time))/12),"tape_feet= $tape_feet\n";
                }
		$tape_speed=0;
                $last_tape_command=$_;
	}
	if (/^\"/) {
		#do nothing as this line will be a comment
	}
}
