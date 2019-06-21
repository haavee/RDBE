# Makefile for library routines
#
OBJECTS = socklib.o
#
socklib.a: $(OBJECTS)
	touch socklib.a
	rm socklib.a
	ar -qc socklib.a $(OBJECTS)
	ar -s socklib.a

clean:
	-rm -f $(OBJECTS) socklib.a
