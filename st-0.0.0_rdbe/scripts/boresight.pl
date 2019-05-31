#!/usr/bin/perl 
# This programs injects snap commands into PCFS to enable continuous
# all sky boresighting. The program is called from a snap file by:
#	sy=/usr2/st/scripts/boresight.pl &
# it reads a file called /usr2/sched/<schedule>.cal_sequence
# which contains:
#"Antenna pointing Az=189.4 El=62.4 at 11:51:09 UT DOY 354 1996 LST 3.68
#source=PKSJ0303-6211,030248.1,-622304,1950.0
#flux1=gaussian,2.30,10s 
#flux2=gaussian,2.30,10s 
#boresight=1998:354:12:06:09
#boresight=1998:354:12:10:39
#"
#Antenna pointing Az=264.6 El=70.8 at 12:05:31 UT DOY 354 1996 LST 3.92
#source=VLAJ0220-349,022049.6,-345505,1950.0
#flux1=gaussian,0.69,10s 
#flux2=gaussian,0.69,10s 
#boresight=1998:354:12:20:31
#boresight=1998:354:12:25:01
#
# where the "345122501" is the expiry time in the format "DOYHHMMSS". The
# boresight will not be performed if this time has already passed. This is
# needed as "aquir" does not have an expiry time, so it is easy for the
# schedule to fall behind, and hence the source will not be at the AZ/EL that
# was required. By using the expiry time boresights will be missed to keep
# the schedule ontime. We prefer to miss out boresights than to have all the 
# AZ/EL's of the source change.
#
# the program inserts SNAP commands into PCFS in the format:
#source=PSR1259,130247.68,-635008.6,2000.0
#flux1=gaussian,03.6,10s
#flux2=gaussian,03.6,10s
#sy=/usr2/st/scripts/wait_on_source.pl 980331 &
#halt
#fivept
#sy=/usr2/st/scripts/wait_finish_bore.pl 980331 &
#halt
#!+10s
#
# Paul Harbison Canberra Deep Space Communications Complex March 1998.
#
#======================================================================
# first get log file name from PCFS shared memory using Chuck Naudet's
# getlogfile.pl script.

    # $|=1;
    # unshift (@INC,"/usr/lib/perl4/sys");
    # print "@INC\n";
    # require "ipc.ph";
    # require "shm.ph";
     #require "fs/params.ph";
     #require "fs/fs_types.ph";
    # require "fs/fscom.ph";
    # require "fs/shm_addr.ph";
#
     #$ipckey=1;
     #$shmid = shmget($ipckey,4096,0);
    # shmread($shmid,$string,264,8) || die "shmread: $! \n";
   #  print "Data from shared memory =$string \n";
   #  $logfile = $string . "\0";

#======================================================================
# get the schedule name
$schedule=$ARGV[0];
#======================================================================
# open the file containing the list of sources/fluxes/expiry times
#
open (SEQ_FILE,"</usr2/sched/$schedule.cal_sequence")
                || die "can't open /usr2/$schedule.cal_sequence\n" ;


$source_found=0;
$flux1_found=0;
$flux2_found=0;

while (<SEQ_FILE>) {
 #print "xx:$_\n";
	if (/source/) {
		$source=$_;
		$source_found=1;
	}
	if (/flux1/) {
		$flux1=$_;
		$flux1_found=1;
	}
	if (/flux2/) {
		$flux2=$_;
		$flux2_found=1;
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
		# pad the hour with a zero if less than 10
		$sec=sprintf("%.2d",$sec);
		$current_time = "$year$yday$hour$min$sec";

		if (($current_time < $boretime ) ){ #&&
		   #$source_found && $flux1_found && $flux2_found) {
			system("/usr2/st/bin/inject_snap $source");
			system("/usr2/st/bin/inject_snap $flux1");
			system("/usr2/st/bin/inject_snap $flux2");
			system("/usr2/st/scripts/wait_on_source.pl $schedule");
			system("/usr2/st/bin/inject_snap log_stuff");
			sleep 5; # wait for log_stuff to finish
			system("/usr2/st/bin/inject_snap fivept");
			system("/usr2/st/scripts/wait_finish_bore.pl $schedule");
			# wait 10 seconds tO See the antenna go back onsource
			sleep 10;
			$source_found=0;
			$flux1_found=0;
			$flux2_found=0;
		} else {
			system("/usr2/st/bin/inject_snap \\\"skipping $source");
		}

	}
	if (/\"/) {
		#do nothing as this line will be a comment
	}
}
