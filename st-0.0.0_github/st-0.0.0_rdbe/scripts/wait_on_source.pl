#!/usr/bin/perl
#
#  A perl script used to halt the schedule until the antenna is on point
#  This uses  PH's inject_snap routine
#
# Usage:   	   $/usr2/st/scripts/wait_on_source.pl logfile_name 
# from oprin:   >sy=/usr2/st/scripts/wait_on_source.pl logfile_name &
# Example: 	>sy=/usr2/st/scripts/wait_on_source.pl mrt003 &
#
# Code can be found in pcfs40:/usr2/st/script/wait_on_source.pl
#
# This code currently is set to be run when the PCFS is running
# and the antenna is pointing to the sky at zenith!.
#
#  Version 1.0  13th Feb 98   Paul Harbison  :  
#       Mods    -------       ---  :
#---------------------------------------------------------------------------
#
# Switch settings
$debug =  0;
$snapit = 1;
$int_limit = 20000;
 
#first halt the schedule so that nothing is done until the antenna is on point 
system("/usr2/st/bin/inject_snap \"halt\" ");

$logname = $ARGV[0];
if ($logname EQ "") {
	$logname = "station";
}
$logname = "/usr2/log/" . $logname . ".log" ;

open(PROC,"tail -1 -f $logname |") || die "Can't open tail pipe \n";

($debug == 1) && (print " logfile opened \n"); 

# can't inject_snap the "onpoint" comment as it goes into the logfile
# and then this program assumes the operator typed it!
print " *****   Waiting for antenna to be on point ****** \n" ;
print " *****   Type 'onpoint' to force the status to be onpoint\n";
system("/usr2/st/bin/inject_snap \"onsource\" ");

$count   = 0;
while (<PROC>) {
        chop($_);
        ($debug == 1) && (print ">adjust   $_ \n"); 
 
  	if (/\/onsource\//) { # if we find "/ousource/"
  		if (/SLEWING/) {
			# send another "onsource" command
			$count++;
			sleep (5);
			print " *****   Waiting for antenna to be on point ****** \n" ;
			print " *****   Type 'onpoint' to force the status to be onpoint\n";
			system("/usr2/st/bin/inject_snap \"onsource\" ");
		} elsif (/TRACKING/) {
			print " *****   On point schedule is continuing\n";
			system("/usr2/st/bin/inject_snap \"cont\" ");
			exit;
		}
	}

        if ($count >= $int_limit) {
           $count   = 0;
           print("************************************************************\n");
           print "WARNING: antenna off point for too long!\n";
           print("************************************************************\n");
        }
           

        if (/onpoint/) {
		system("/usr2/st/bin/inject_snap '\"On point is forced, schedule is continuing'");
		system("/usr2/st/bin/inject_snap \"cont\" ");
		exit;
	}
        if (/terminate/) {
         	print("*** antenna on source Script Stopped *** \n");
        	print("*** now assuming we are on point *** \n");
         	close PROC;
         	die;
	}
}
($debug == 1) && (print "ANTENNA IS NOW ON POINT ! \n");
#now continue the schedule now that the antenna is on point 
system("/usr2/st/bin/inject_snap \"cont\" ");
close PROC;
#----------------------------------------------------------------------------- 
