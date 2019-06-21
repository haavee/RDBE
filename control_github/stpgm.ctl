* Put site-specific programs here that should
* be started by the Field System. 
* antcn should not be here
*erchk n xterm -geom 99x16+0+518 -title ERRORS -e erchk &
&stqkr n stqkr &
moni2 n xterm -fn 8x16 -geom 81x6+1+1 -tit "System Status" -e monit2 &
erchk n xterm -fn 8x16 -sb -geom 96x10+0+900 -tit ERRORS -e erchk &
* moni2 n xterm -fn 8x16 -geom 81x6+20+200 -tit "System Status" -e monit2 &
* erchk n xterm -fn 8x16 -sb -geom 96x6+0+732 -tit ERRORS -e erchk &
* dbbccn n dbbccn &
* sdhr n /usr2/st/bin/askvax &
* sdhr n /usr2/st/bin/antrcv_multi &
rdbe_tsys_receive_0 n rdbe_tsys_receive_0 &
rdbe_tsys_receive_1 n rdbe_tsys_receive_1 &
