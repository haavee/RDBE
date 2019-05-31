#!/usr/bin/perl 
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
