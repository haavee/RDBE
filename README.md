# RDBE
Bolt support for RDBE onto FieldSystem

This repo contains station code (under the 'arecibo' branch) to enrich the Field System with rdbe specific commands and configuration. 

The project also contains a Python based "vex2snap" program which knows about Mark5C, Mark6, FlexBuff and e-vlbi recorders and RDBE backends and can be used in stead of DRUDG to generate the .prc/.snp files for your station. It allows you to override the recorder type from that found in the VEX file it is "snapping".

