#!/usr/bin/perl 
# 
# This routine opens a tail pipe of the log file. 
# By searching/splitting the input lines a great deal 
# of log-file checks can be made. Presently, this routine 
# looks for completion of a PCFS fivept and then writes 
# a "lbore completed" message to the PCGS log file. 
#
# usage:    chk_bore.pl logfile_name
# example:  chk_bore.pl mrt003
# 
# This routine can be called by a PCFS procedure. Currently 
# its in initpl at pcfs10. 
# 
# 
# Jan 14 1998 Chuck Naudet 
#--------------------------------------------------------------------------- 
  $debug = 0;
  $filename =  $ARGV[0]  ;
  if ($filename EQ "" ) {
    $filename = "station" ; 
  }
  $filename = "/usr2/log/" . $filename . ".log" ;
#
  open(PROC,"tail -1 -f $filename |") || die "Can't open tail pipe ";
  while ($line=<PROC>) {
   if($debug == 1) {
     ($line =~ /#fivpt/) && (print "$line "); 
   }
   ($line =~ /terminate/) && (exit) ;
   if ($line =~ /#fivpt#offset/) {
#  Ok Tell the PCFS that fivept is finished
    system("/usr2/st/minical/done lbore");
#  OK now be sure and make the default to be ZERO OFFSETS!!
    system("/usr2/st/bin/inject_span \"azeloff=000000,000000\" ");
#    exit;
   }
  }
  close PROC
