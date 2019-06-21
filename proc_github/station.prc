"--------------------------------------
define  initi         19172205326x
"welcome to the rdbe field system #1 at AR
xlog=on
xdisp=on
"rdbe_initi
"mk5c_initi
enddef
define  ifd2          00000000000x
rdbe_cmd=dbe0,5,dbe_alc=1:$:off
enddef
define  ifread        00000000000x
rdbe_cmd=dbe0,5,dbe_alc?
rdbe_adc=dbe0,5,0
rdbe_adc=dbe0,5,1
enddef
define  set_quant     00000000000x
rdbe_cmd=dbe0,5,dbe_ddc_quantize=0:$
rdbe_cmd=dbe0,5,dbe_ddc_quantize=1:$
rdbe_cmd=dbe0,5,dbe_ddc_quantize=2:$
rdbe_cmd=dbe0,5,dbe_ddc_quantize=3:$
enddef
"--------------------------------------
define  rdbe_initi    00000000000x
rdbe_cmd=dbe0,3,dbe_data_send=off
"rdbe_cmd=dbe0,3,dbe_dot_set=
"rdbe_sync=dbe0
rdbe_cmd=dbe0,3,dbe_dot?
rdbe_cmd=dbe0,3,dbe_ioch_assign=0:4::1:4::0:5::1:5::0:6::1:6::0:7::1:7::0:8::1:8::0:9::1:9::0:10::1:10::0:11::1:11
rdbe_cmd=dbe0,3,dbe_quantize=reset
!+1s
rdbe_cmd=dbe0,3,dbe_quantize=hold_set
!+1s
rdbe_cmd=dbe0,3,dbe_quantize?
rdbe_cmd=dbe0,10,dbe_status?
!+2s
enddef
define  rdbe2_initi   00000000000xx
rdbe_cmd=dbe1,30,dbe_personality=:pfbg_1_4.bin
rdbe_cmd=dbe1,20,dbe_execute=init
rdbe_cmd=dbe1,3,dbe_data_send=off
"rdbe_cmd=dbe1,3,dbe_dot_set=
rdbe_sync=dbe1
rdbe_cmd=dbe1,3,dbe_dot?
rdbe_cmd=dbe1,3,dbe_ioch_assign=0:4::1:4::0:5::1:5::0:6::1:6::0:7::1:7::0:8::1:8::0:9::1:9::0:10::1:10::0:11::1:11
rdbe_cmd=dbe1,3,dbe_quantize=reset
!+1s
rdbe_cmd=dbe1,3,dbe_quantize=hold_set
!+1s
rdbe_cmd=dbe1,3,dbe_quantize?
rdbe_cmd=dbe1,10,dbe_status?
!+2s
enddef
define  mk5c_initi    00000000000x
"mk5relink
"!+2s
mk5=personality=mark5c:bank;
!+4s
mk5=fill_pattern=464374526;
!+1s
mk5=mode=mark5b:0xffff:1;
!+1s
mk5=packet=36:0:5008:0:0;
enddef
define  sched_initi   00000000000x
"mk5_status
!+2s
"- - - - starting schedule - - - -
enddef
define  sched_end   01000000000
"mk5_status
"- - - - end of schedule - - - -
enddef
define  midob         00000000000x
onsource
wx
rdbe_cmd=dbe0,2,dbe_dot?
rdbe_cmd=dbe0,2,dbe_data_send?
rdbe_cmd=dbe0,2,dbe_status?
mk5_status;
mk5=pointers?;
op_stream=start
enddef
define  postob        00000000000x
!+1s
mk5_status;
collect@
rdbe_cmd=0,5,dbe_alc?
!+1s
rdbe_time=dbe0,3
!+1s
rdbe_adc=dbe0,3,0
!+1s
rdbe_adc=dbe0,3,1
enddef
define  midtp         00000000000
enddef
define  checkmk5      00000000000
mk5=rtime?;
!+1s
"mk5=scan_check?;
mk5=dir_info?;
enddef
define  caltsys       00000000000x
caltemp=formvc,formif
onsource
tpi=formvc,formif
ifd=max,max,*,*
!+2s
tpzero=formvc,formif
ifd=old,old,*,*
calon
!+2s
onsource
tpical=formvc,formif
tpdiff=formvc,formif
caloff
tsys=formvc,formif
enddef
define  dualpol       00000000000x
rdbe_cmd=dbe0,3,dbe_ioch_assign=0:1::1:1::0:3::1:3::0:5::1:5::0:7::1:7::0:9::1:9::0:11::1:11::0:13::1:13::0:15::1:15
enddef
define  singpol       00000000000x
rdbe_cmd=dbe0,3,dbe_ioch_assign=0:0-15
enddef
define  getver       000000000000
rdbe_cmd=dbe0,2,dbe_hw_version?
rdbe_cmd=dbe0,2,dbe_alc_fpgaver?
rdbe_cmd=dbe0,2,dbe_sw_version?
rdbe_cmd=dbe0,2,dbe_fs?
rdbe_cmd=dbe0,2,dbe_dot?
rdbe_cmd=dbe0,2,dbe_personality?
rdbe_cmd=dbe0,2,dbe_status?
rdbe_cmd=dbe1,2,dbe_hw_version?
rdbe_cmd=dbe1,2,dbe_alc_fpgaver?
rdbe_cmd=dbe1,2,dbe_sw_version?
rdbe_cmd=dbe1,2,dbe_fs?
rdbe_cmd=dbe1,2,dbe_dot?
rdbe_cmd=dbe1,2,dbe_personality?
rdbe_cmd=dbe1,2,dbe_status?
enddef
define  testsetup01   00000000000x
rdbefclr=dbe0
rdbef01=dbe0,if0,8512,32,lsb,r,4
rdbef03=dbe0,if0,8480,32,lsb,r,5
rdbef05=dbe0,if0,8448,32,lsb,r,6
rdbef07=dbe0,if0,8416,32,lsb,r,7
rdbef02=dbe0,if1,8512,32,lsb,l,4
rdbef04=dbe0,if1,8480,32,lsb,l,5
rdbef06=dbe0,if1,8448,32,lsb,l,6
rdbef08=dbe0,if1,8416,32,lsb,l,7
rdbef09=dbe0,if0,8384,32,lsb,r,8
rdbef11=dbe0,if0,8352,32,lsb,r,9
rdbef13=dbe0,if0,8320,32,lsb,r,10
rdbef15=dbe0,if0,8288,32,lsb,r,11
rdbef10=dbe0,if1,8384,32,lsb,l,08
rdbef12=dbe0,if1,8352,32,lsb,l,09
rdbef14=dbe0,if1,8320,32,lsb,l,10
rdbef16=dbe0,if1,8288,32,lsb,l,11
rdbe_if=loc,7600,usb,rcp,dbe0,if0
rdbe_if=loa,7600,usb,lcp,dbe0,if1
rdbe_if=lob,2100,usb,rcp,dbe1,if0
rdbe_if=lod,2100,usb,lcp,dbe1,if1
rdbe_cmd=dbe0,5,dbe_ioch_assign=0:4::1:4::0:5::1:5::0:6::1:6::0:7::1:7::0:8::1:8::0:9::1:9::0:10::1:10::0:11::1:11
enddef
define  testsetup02   00000000000x
rdbefclr=dbe0
rdbef01=dbe0,if0,4912,32,lsb,r,4
rdbef03=dbe0,if0,4880,32,lsb,r,5
rdbef05=dbe0,if0,4848,32,lsb,r,6
rdbef07=dbe0,if0,4816,32,lsb,r,7
rdbef02=dbe0,if1,4912,32,lsb,l,4
rdbef04=dbe0,if1,4880,32,lsb,l,5
rdbef06=dbe0,if1,4848,32,lsb,l,6
rdbef08=dbe0,if1,4816,32,lsb,l,7
rdbef09=dbe0,if0,4784,32,lsb,r,8
rdbef11=dbe0,if0,4752,32,lsb,r,9
rdbef13=dbe0,if0,4720,32,lsb,r,10
rdbef15=dbe0,if0,4688,32,lsb,r,11
rdbef10=dbe0,if1,4784,32,lsb,l,08
rdbef12=dbe0,if1,4752,32,lsb,l,09
rdbef14=dbe0,if1,4720,32,lsb,l,10
rdbef16=dbe0,if1,4688,32,lsb,l,11
rdbe_if=loc,4100,usb,rcp,dbe0,if0
rdbe_if=loa,4100,usb,lcp,dbe0,if1
rdbe_if=lob,4100,usb,rcp,dbe1,if0
rdbe_if=lod,4100,usb,lcp,dbe1,if1
rdbe_cmd=dbe0,5,dbe_ioch_assign=0:4::1:4::0:5::1:5::0:6::1:6::0:7::1:7::0:8::1:8::0:9::1:9::0:10::1:10::0:11::1:11
enddef
define  rdbe_status   00000000000x
rdbe_cmd=0,5,dbe_alc?
!+1s
rdbe_time=dbe0,3
!+1s
rdbe_adc=dbe0,3,0
!+1s
rdbe_adc=dbe0,3,1
!+1s
rdbe_cmd=0,5,dbe_status?
!+1s
rdbe_cmd=0,5,dbe_data_send?
enddef
define  ifd1          00000000000
rdbe_cmd=dbe0,5,dbe_alc=0:$:off
enddef
