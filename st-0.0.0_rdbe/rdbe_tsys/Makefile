#
CFLAGS= 
LIBES = ../stlib/stlib.a ../../fs/poclb/poclb.a ../../fs/clib/clib.a\
../../fs/rtelb/rtelb.a -lm
#
all: ../bin/rdbe_tsys_receive_0 ../bin/rdbe_tsys_receive_1

OBJECTS0 = rdbe_tsys_receive_0.o  
#
../bin/rdbe_tsys_receive_0:  $(OBJECTS0)
	cc  -o ../bin/rdbe_tsys_receive_0 $(OBJECTS0) $(LIBES)
	strip ../bin/rdbe_tsys_receive_0
#
OBJECTS1 = rdbe_tsys_receive_1.o
#
../bin/rdbe_tsys_receive_1: $(OBJECTS1)
	cc  -o ../bin/rdbe_tsys_receive_1 $(OBJECTS1) $(LIBES)
	strip ../bin/rdbe_tsys_receive_1
#
