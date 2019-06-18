#!/usr/bin/perl
#
#  A perl script used to halt the schedule until the fivept is finished
#  This uses  PH's inject_snap routine
#
# Usage:   	   $/usr2/st/scripts/wait_finish_bore.pl logfile_name 
# from oprin:   >sy=/usr2/st/scripts/wait_finish_bore.pl logfile_name &
# Example: 	>sy=/usr2/st/scripts/wait_finish_bore.pl mrt003 &
#
# Code can be found in pcfs40:/usr2/st/script/wait_finish_bore.pl
#
# This code currently is set to be run when the PCFS is running
# and the antenna is pointing to the sky at zenith!.
#
#  Version 1.0  23th Mar 98   Paul Harbison  :  
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

$count   = 0;
while (<PROC>) {
        chop($_);
 
  	if (/#fivpt#offset/) { # if we find "#fivpt#offset"
		print " ***************************************************\n";
		print "\n";
		print " *****   Boresight Finished schedule is continuing\n";
		print "\n";
		print " ***************************************************\n";
		system("/usr2/st/bin/inject_snap \"cont\" ");
		exit;
	}
	if ($count/20 == int($count/20)) {#print the message only every 20 lines
        	print " *****   Waiting for boresight to finish ******\n" ;
        	print " *****   Type 'borefinish' to force the status to be finished boresight\n";
	}
	$count++;
        if ($count >= $int_limit) {
           $count   = 0;
           print("************************************************************\n");
           print "WARNING: antenna boresighting for too long!\n";
           print("************************************************************\n");
        }
           

        if (/borefinish/) {
		system("/usr2/st/bin/inject_snap '\"Boresight finish is forced, sched is continuing'");
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
($debug == 1) && (print "BORESIGHT FINISHED ! \n");
#now continue the schedule now that the antenna is on point 
close PROC;
#----------------------------------------------------------------------------- 
