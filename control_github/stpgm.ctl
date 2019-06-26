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
*rdbe_tsys_receive_0 n rdbe_tsys_receive_0 &
*rdbe_tsys_receive_1 n rdbe_tsys_receive_1 &
* HV 24Jun2019 - Rewrite of TSYS receive, now takes
*                command line arguments.
*      Combine: /usr2/control/rdbead.ctl [RDBE0, RDBE1]
*      with /usr2/oper/vlbish/examples/rdbe_10g_init.scr
*      to find out which of the four rdbes is RDBE0,RDBE1
*      (match IP address) for the FS and from the configuration
*      you can tell which TSYS broadcast they do
tsys0 n rdbe_tsys_receive 0 239.0.2.34 20021 &
tsys1 n rdbe_tsys_receive 1 239.0.2.35 20021 &
