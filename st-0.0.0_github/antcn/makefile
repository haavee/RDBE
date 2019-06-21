#
CFLAGS= -g -DNDEBUG -I../../fs/include
LIBES = ../../fs/poclb/poclb.a ../../fs/clib/clib.a ../../fs/rtelb/rtelb.a ../socklib/socklib.a -lm
#
OBJECTS = antcn.o antcn_ar.o 
#
../bin/antcn: $(OBJECTS)
	cc  -o ../bin/antcn $(OBJECTS) $(LIBES)
#
antcn:
	rm -f ../bin/antcn
	make
