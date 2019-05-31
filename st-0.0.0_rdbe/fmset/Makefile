OBJECTS = fmset.o asktime.o get4time.o getfmtime.o getvtime.o\
initvstr.o set4time.o setfmtime.o setvtime.o gets2time.o sets2time.o asksure.o\
getrdbetime.o get5btime.o set5btime.o get_rdbetime.o\
get_rdbe_datetime.o get_rdbe_issync.o
#
CFLAGS =  -I/usr/include/ncurses
LIBES	= /usr2/fs/poclb/poclb.a /usr2/fs/clib/clib.a /usr2/fs/rtelb/rtelb.a
#
../bin/fmset: $(OBJECTS)
	cc  -o ../bin/fmset $(OBJECTS) $(LIBES) -lm -lncurses
	strip ../bin/fmset
#
fmset:
	rm -f ../bin/fmset
	make
