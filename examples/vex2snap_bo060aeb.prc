define  proc_library  00000000000x
" BO060A      EB_RDBE2  Eb
" vex2snap version 2014_nov_06 by DG and UB, MPIfR
"< RDBE  rack >< Mark5C   recorder 1>
enddef
define  exper_initi   00000000000x  
proc_library
sched_initi
enddef
define  sched_initi   00000000000x
"setpers
!+1s
rdbe_cmd=0,3,dbe_data_send=on:2019111100000:2019111124850:0;
!+1s
rdbe_cmd=0,3,dbe_1pps_mon=enable:239.0.2.34:20020;
!+1s
rdbe_cmd=0,3,dbe_tsys_mon=enable:239.0.2.34:20021:10;
!+1s
rdbe_cmd=0,3,dbe_dot?;
!+1s
rdbe_time=dbe0,3
setdbe1
getgps
!+1s
ready_disk
!+1s
enddef
define  setup01       00000000000x
pcalon
"lo=
lo=loa,4100.00,usb,rcp,1
lo=loc,4100.00,usb,lcp,1
rdbe_person=dbe0,pfb
rdbefclr=dbe0
rdbef01=dbe0,if0,4884.00,32,lsb,r,8
rdbef02=dbe0,if1,4884.00,32,lsb,l,8
rdbef03=dbe0,if0,4916.00,32,lsb,r,7
rdbef04=dbe0,if1,4916.00,32,lsb,l,7
rdbef05=dbe0,if0,4948.00,32,lsb,r,6
rdbef06=dbe0,if1,4948.00,32,lsb,l,6
rdbef07=dbe0,if0,4980.00,32,lsb,r,5
rdbef08=dbe0,if1,4980.00,32,lsb,l,5
rdbef09=dbe0,if0,5012.00,32,lsb,r,4
rdbef10=dbe0,if1,5012.00,32,lsb,l,4
rdbef11=dbe0,if0,5044.00,32,lsb,r,3
rdbef12=dbe0,if1,5044.00,32,lsb,l,3
rdbef13=dbe0,if0,5076.00,32,lsb,r,2
rdbef14=dbe0,if1,5076.00,32,lsb,l,2
rdbef15=dbe0,if0,5108.00,32,lsb,r,1
rdbef16=dbe0,if1,5108.00,32,lsb,l,1
rdbe_if=loa,4100.00,usb,rcp,dbe0,if0
rdbe_if=loc,4100.00,usb,lcp,dbe0,if1
rdbe_cmd=0,5,dbe_ioch_assign=0:8::1:8::0:7::1:7::0:6::1:6::0:5::1:5::0:4::1:4::0:3::1:3::0:2::1:2::0:1::1:1;
mk5=fill_pattern=464374526;
mk5=packet=36:0:5008:0:0;
mk5=mode=mark5b:0xffffffff:1;
mk5=play_rate=data:64;
bank_check
enddef
define  setpers        00000000000x
rdbe_cmd=dbe0,50,dbe_personality=pfbg:pfbg_1_4.bin
rdbe_cmd=dbe0,3,dbe_execute=init
!+2s
"essr=run,off
"essr=mode,4
"essr=route,2,4
"essr=pace,4,3
"essr=run,on
mk5=fill_pattern=464374526;
mk5=mode=mark5b:0xffffffff:1;
mk5=packet=36:0:5008:0:0;
enddef
define  preob         00000000000x
ifread
rdbe_cmd=0,3,dbe_quantize=hold_set;
!+1s
op_stream=start
enddef
define  start         00000000000
collect@!,10s
enddef
define  collect       00000000000x
rdbe_tsys=dbe0,if0
rdbe_tsys=dbe0,if1
enddef
define  setdbe1       00000000000x
enddef
