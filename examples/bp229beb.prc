define  proc_library  19118230123x
" BP229B      EB_RDBE2  Eb
" vex2snap version 2014_nov_06 by DG and UB, MPIfR
"< RDBE  rack >< Mark5C   recorder 1>
enddef
define  exper_initi   19118230122x  
proc_library
sched_initi
enddef
define  sched_initi   19118230123x
"setpers
!+1s
rdbe_cmd=0,3,dbe_data_send=on:2019118233000:2019119073027:0;
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
define  setup01       19119012310x
pcalon
"lo=
lo=loa,16072.00,lsb,rcp,1
lo=loc,16072.00,lsb,lcp,1
rdbe_person=dbe0,ddc
rdbefclr=dbe0
rdbefclr=dbe1
rdbef01=dbe0,if0,15240.00,64,usb,r,0
rdbef02=dbe1,if1,15240.00,64,usb,l,0
rdbef03=dbe0,if0,15304.00,64,usb,r,0
rdbef04=dbe1,if1,15304.00,64,usb,l,0
rdbef05=dbe0,if0,15368.00,64,usb,r,0
rdbef06=dbe1,if1,15368.00,64,usb,l,0
rdbef07=dbe0,if0,15432.00,64,usb,r,0
rdbef08=dbe1,if1,15432.00,64,usb,l,0
rdbe_if=loa,16072.00,lsb,rcp,dbe0,if0
rdbe_if=loc,16072.00,lsb,lcp,dbe0,if1
rdbe_if=loa,16072.00,lsb,rcp,dbe1,if0
rdbe_if=loc,16072.00,lsb,lcp,dbe1,if1
rdbe_send=dbe0,off
rdbe_cmd=dbe0,3,dbe_dot?
rdbe_send=dbe1,off
rdbe_cmd=dbe1,3,dbe_dot?
rdbe_dc_cfg=dbe0,0:2:32.000000:0
rdbe_dc_cfg=dbe1,0:2:32.000000:0
rdbe_dc_cfg=dbe0,1:2:32.000000:1
rdbe_dc_cfg=dbe1,1:2:32.000000:1
rdbe_dc_cfg=dbe0,2:2:96.000000:1
rdbe_dc_cfg=dbe1,2:2:96.000000:1
rdbe_dc_cfg=dbe0,3:2:96.000000:0
rdbe_dc_cfg=dbe1,3:2:96.000000:0
rdbe_cmd=dbe0,3,dbe_xbar=3:3:3:2:2:2:2:2;
rdbe_cmd=dbe1,3,dbe_xbar=7:7:7:6:2:2:2:2;
rdbe_cmd=dbe0,3,dbe_vdif_threads_id=0:2:4:6;
rdbe_cmd=dbe1,3,dbe_vdif_threads_id=1:3:5:7;
!+1s
bank_check
rdbe_send=dbe0,on
rdbe_send=dbe1,on
mk5=mode=vdifl_40000-2048-8-2;
mk5=fill_pattern=464374526;
mk5=packet=28:0:5032:0:0;
!+1s
enddef
define  setpers       00000000000xx
rdbe_cmd=dbe0,50,dbe_personality=ddc:ddc_1601583.bin
rdbe_cmd=dbe1,50,dbe_personality=ddc:ddc_1601583.bin
rdbe_cmd=dbe0,3,dbe_execute=init
!+2s
rdbe_cmd=dbe1,3,dbe_execute=init
!+2s
"essr=run,off
"essr=mode,3
"essr=route,2,4
"essr=route,3,4
"essr=pace,4,3
"essr=run,on
mk5=mode=vdifl_40000-1024-8-2;
mk5=fill_pattern=464374526;
mk5=packet=28:0:5032:0:0;
enddef
define  preob         19118232950x
ifread
bbcread
ifread2
bbcread2
!+1s
op_stream=start
enddef
define  start         19118232953
collect@!,10s
enddef
define  collect       19118232953x
rdbe_tsys=dbe0,if0
rdbe_tsys=dbe0,if1
rdbe_tsys=dbe1,if0
rdbe_tsys=dbe1,if1
enddef
define  setdbe1       19118230129x
rdbe_cmd=1,3,dbe_data_send=on:2019118233000:2019119073027:0;
!+1s
rdbe_cmd=1,3,dbe_1pps_mon=enable:239.0.2.35:20020;
!+1s
rdbe_cmd=1,3,dbe_tsys_mon=enable:239.0.2.35:20021:10;
!+1s
rdbe_cmd=1,3,dbe_dot?;
!+1s
rdbe_time=dbe1,3
enddef
define  checkmk5      19119013013x
mk5=rtime?;
!+1s
mk5=dir_info?
"mk5=scan_set=next
!+1s
mk5=scan_check?
mk5=vsn?
mk5=bank_set?
mk5=mode?
mk5=play_rate?
" write out some bytes from beginning of the last scan
!+1s
mk5=scan_set=::+40000000
!+1s
mk5=disk2file=systest.m5b:::w
!+3s
" run checkdata
sy=exec /home/oper/bin/checkdata_ddc8-2gbps.py `lognm` systest.m5b &
"
enddef
