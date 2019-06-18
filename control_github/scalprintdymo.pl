#!/usr/bin/perl
open(fi,"/tmp/DRlab.tmp");
# scale and numbers to take off x and y before scale, fontsize
$scale=1.5; $xoff=22; $yoff=-40; $fontsize=10;
$fil=0; $ignore_misplaced=0;
while(<fi>){
  chomp;
  $inline=$_;
  (@ss)=split();  #so we can dissect lines
#ignore statements which are present twice and confuse gs
  if(($ss[1] eq "setgray") || ($ss[0] eq "showpage")){$ignore_misplaced = 1; $done_prolog=0;}
  if($ss[2] eq "moveto"){
     $ignore_misplaced = 0;
     if($done_prolog == 0){
          $linewidth=0.5*$scale;
	  $lw=sprintf("%1.2f ",$linewidth);
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
# presently label is a wee bit small, so scale the x-coords
    if($ss[1] eq "scalefont"){$inline=sprintf("%d scalefont ", $fontsize);}
    if(($ss[2] eq "moveto") || ($ss[2] eq "lineto")){
        $x1=($ss[0]-$xoff)*$scale; 
        $y1=($ss[1]-$yoff)*$scale; 
	$inline=sprintf("%d %d %s ", $x1,$y1,$ss[2]);
	print "NEW $_ $inline \n";
    }
    print(fo $inline,"\n");
  }
}
&dymout; # send out final label
sub dymout {
# close temporary ps and send to printer
print(fo "showpage\n\%\%Trailer\n");
close(fo);
`gs -q -dNOPAUSE -dSAFER -g400x900 -r300x300  -sDEVICE=pbm -sOutputFile=- /tmp/dymo.ps -c quit |  pnmnoraw   >/tmp/x$fil.pbm`;
}
