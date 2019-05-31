#!/usr/bin/python

# this program takes in a carrier frequency, sideband, and bandwidth.
# it returns a sub-band selection and DDS tuning
# command line syntax:
# ./ft.py <fc> <sb> <bw>
# for example, to choose a carrier frequency of 512MHz, upper sideband,
# and 16MHz bandwidth, the command line would be:
# ./ft.py 512 1 16

# imports
import time
import sys

# make a function that uses the easy hex conversion
def dec2hex(n):
    return "%X" % n

fc = float(sys.argv[1])
sb = int(sys.argv[2])
bw = float(sys.argv[3])

if sb == 1:
    print "USB selected"
    if fc <= 640-bw:
        # region 2 selected, start frequencies from 512 to 640-BW
        fdds = fc - 512 + bw/2
        bb_mode = 1
        sub = 2
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 + bw
        print "%s-%sMHz selected" % (f1,f2)
    elif fc <= 704 and bw == 128:
        # special case to handle 128MHz BW in region 1 where up-mixing is needed
        # region 1 selected, 640 > fc > 704
        fdds = 768 - fc - bw/2
        bb_mode = 0
        sub = 1        
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 + bw
        print "%s-%sMHz selected" % (f1,f2)
    elif fc <= 768-bw/2 and bw < 128:
        # region 1 selected, start frequencies from 640 to 768-BW
        fdds = 768 - fc - bw/2
        bb_mode = 0
        sub = 1        
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 + bw
        print "%s-%sMHz selected" % (f1,f2)
    elif fc <= 768 and bw == 128:
        # special case to handle 128MHz BW in region 1 where down-mixing is needed
        # region 1 selected, 640 > fc > 768
        fdds = fc - 768 + bw/2
        bb_mode = 1
        sub = 1        
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 + bw
        print "%s-%sMHz selected" % (f1,f2)        
    elif fc <= 896-bw and bw < 128:
        # region 1 selected, start frequencies from 768 to 896-BW
        fdds = fc - 768 + bw/2
        bb_mode = 1
        sub = 1
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 + bw
        print "%s-%sMHz selected" % (f1,f2)        
    elif fc <= 1024-bw:
        # region 0 selected, start frequencies from 896 to 1024-BW
        fdds = 1024 - fc - bw/2
        bb_mode = 0
        sub = 0
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 + bw
        print "%s-%sMHz selected" % (f1,f2)        
    else:
        print "invalid carrier frequency selection, please try again."
else:
    print "LSB selected"
    if fc >= 896+bw:
        # region 0 selected, start frequencies from 1024 to 896+BW
        fdds = 1024 - fc + bw/2
        bb_mode = 1
        sub = 0
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 - bw        
        print "%s-%sMHz selected" % (f1,f2)
    elif fc >= 768 + bw and bw < 128:
        # region 3 selected, start frequencies from 896 to 768+BW
        fdds = fc - 768 - bw/2
        bb_mode = 0
        sub = 3
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 - bw        
        print "%s-%sMHz selected" % (f1,f2)
    elif fc >= 832 and bw == 128:
        # special case to handle 128MHz BW in region 3 where up-mixing is needed
        # region 3 selected, start frequencies from 896 to 768+BW
        fdds = fc - 768 - bw/2
        bb_mode = 0
        sub = 3
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 - bw
        print "%s-%sMHz selected" % (f1,f2)
    elif fc >= 768 and bw == 128:
        # special case to handle 128MHz BW in region 3 where down-mixing is needed
        # region 3 selected, start frequencies from 896 to 768+BW
        fdds = 768 - fc + bw/2
        bb_mode = 1
        sub = 3
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 - bw
        print "%s-%sMHz selected" % (f1,f2)        
    elif fc >= 640 + bw and bw < 128:
        # region 3 selected, start frequencies from 768 to 640+BW
        fdds = 768 - fc + bw/2
        bb_mode = 1
        sub = 3
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 - bw        
        print "%s-%sMHz selected" % (f1,f2)
    elif fc >= 512 + bw:
        # region 2 selected, start frequencies from 640 to 576+BW
        fdds = fc - 512 - bw/2
        bb_mode = 0
        sub = 2
        print "dds frequency: %s, baseband mixer mode: %s, sub-band: %s" % (fdds,bb_mode,sub)
        f1 = fc
        f2 = f1 - bw        
        print "%s-%sMHz selected" % (f1,f2)
    else:        
        print "invalid carrier frequency selection, please try again."
