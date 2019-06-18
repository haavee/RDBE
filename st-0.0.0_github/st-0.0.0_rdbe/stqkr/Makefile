#
all: ../bin/stqkr ../bin/prog_rdbe_alc_adj
CFLAGS= -g
LIBES = ../stlib/stlib.a  \
  ../../fs/port/port.a ../../fs/poclb/poclb.a \
  ../../fs/clib/clib.a ../../fs/rtelb/rtelb.a /usr/lib/i386-linux-gnu/libm.a
#
OBJECTS1 =\
stqkr.o getgps.o effcal.o effpcal.o effrx.o effwx.o efflo.o \
  p_rdbe_alc.o rdbe_cmd.o rdbe_adc.o rdbe_time.o \
  rdbe_fq.o rdbe_tsys.o rdbefclr.o rdbe_if.o rdbe_sync.o rdbe_talk.o \
  rdbe_person.o rdbe_dc_cfg.o rdbe_send.o scmds.o \
  rdbe_fchk.o rdbe_init.o essr.o
#
../bin/stqkr: $(OBJECTS1)
	cc  -o ../bin/stqkr $(OBJECTS1) $(LIBES) -lm
	strip ../bin/stqkr
#
OBJECTS2 =\
prog_rdbe_alc_adj.o
#
../bin/prog_rdbe_alc_adj: $(OBJECTS2)
	cc  -o ../bin/prog_rdbe_alc_adj $(OBJECTS2) $(LIBES) -lm
	strip ../bin/prog_rdbe_alc_adj
#
#all:
#	rm -f ../bin/stqkr ../bin/prog_rdbe_alc_adj
#	make
