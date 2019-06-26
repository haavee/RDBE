#
CFLAGS= 
LIBES = ../stlib/stlib.a ../../fs/poclb/poclb.a ../../fs/clib/clib.a\
../../fs/rtelb/rtelb.a -lm
#
all: ../bin/rdbe_tsys_receive

#
../bin/rdbe_tsys_receive: rdbe_tsys_receive.o
	cc  -o ../bin/rdbe_tsys_receive rdbe_tsys_receive.o $(LIBES)
	strip ../bin/rdbe_tsys_receive
	-rm -f ../bin/rdbe_tsys_receive_log
	/bin/ln -s ../bin/rdbe_tsys_receive ../bin/rdbe_tsys_receive_log
