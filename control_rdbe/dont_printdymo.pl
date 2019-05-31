#!/usr/bin/perl
open(fi,"/tmp/DRlab.tmp");
$printer="dymo"; #name by which cups knows it
$fil=0; $ignore_misplaced=0;
while(<fi>){
  chomp;
  $inline=$_;
  (@ss)=split();  #so we can dissect lines
#ignore statements which are present twice and confuse gs
  if(($ss[1] eq "setgray") || ($ss[0] eq "showpage")){$ignore_misplaced = 1; $done_prolog=0;}
  if($ss[1] eq "setlinewidth"){$lw=$ss[0];}
  if($ss[2] eq "moveto"){
     $ignore_misplaced = 0;
     if($done_prolog == 0){
	  print (fo "0 setgray\n", $lw," setlinewidth\n 90 rotate \n");
	  $done_prolog=1;
     }
  }
  if($ignore_misplaced != 1){
    if(substr($_,5,5) eq "Adobe"){
# if not first label, send out previous
       if($fil != 0){
          &dymout;
       }
       # open temporary ps file for each label
       open(fo,">/tmp/dymo.ps");
       $fil++;
    }
    print(fo $inline,"\n");
  }
}
&dymout; # send out final label
sub dymout {
# close temporary ps and send to printer
print(fo "showpage\n\%\%Trailer\n");
close(fo);
`ps2epsi /tmp/dymo.ps /tmp/dymo1.ps`;
`lpr -P$printer /tmp/dymo1.ps`;
}
