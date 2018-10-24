## these are set in ../Makefile
# CC = gcc
# CFLAGS = -g #-O2

libcheetah.a:	libcheetah.o saclru.o util.o sacopt.o ppopt.o faclru.o \
		facopt.o pixie.o din.o dmvl.o
	rm -f libcheetah.a
	ar qcv libcheetah.a libcheetah.o util.o saclru.o sacopt.o faclru.o \
		ppopt.o facopt.o pixie.o din.o dmvl.o
	$(RANLIB) libcheetah.a

clean:
	rm -f libcheetah.a *.o *.exe core *~ Makefile.bak

libcheetah.o:	libcheetah.c
	$(CC) $(CFLAGS) -c libcheetah.c

util.o:	util.c
	$(CC) $(CFLAGS) -c util.c

saclru.o:	saclru.c
	$(CC) $(CFLAGS) -c saclru.c

sacopt.o:	sacopt.c
	$(CC) $(CFLAGS) -c sacopt.c

faclru.o:	faclru.c
	$(CC) $(CFLAGS) -c faclru.c

facopt.o:	facopt.c
	$(CC) $(CFLAGS) -c facopt.c

pixie.o:	pixie.c
	$(CC) $(CFLAGS) -c pixie.c

din.o:	din.c
	$(CC) $(CFLAGS) -c din.c

ppopt.o:	ppopt.c
	$(CC) $(CFLAGS) -c ppopt.c

dmvl.o:	dmvl.c
	$(CC) $(CFLAGS) -c dmvl.c

ascbin.o:	ascbin.c
	$(CC) $(CFLAGS) -c ascbin.c

ascbin:	ascbin.o
	$(CC) $(CFLAGS) -o ascbin ascbin.o

/tmp/trace_file:        trace.ccom.Z
	zcat trace.ccom | ascbin > /tmp/trace_file

test:
	$(MAKE) cheetah ascbin
	$(MAKE) /tmp/trace_file
	cheetah -Csa -Fbasic -Rlru -a5 -b14 -l4 -n2 -t1000000 < /tmp/trace_file > /tmp/test.results
	cheetah -Csa -Fbasic -Ropt -a5 -b14 -l4 -n2 -t1000000 < /tmp/trace_file >> /tmp/test.results
	cheetah -Cfa -Fbasic -Rlru -M65536 -i64 -t1000000 < /tmp/trace_file >> /tmp/test.results
	cheetah -Cfa -Fbasic -Ropt -M65536 -i64 -t1000000 < /tmp/trace_file >> /tmp/test.results
	cheetah -Cdm -Fbasic -c13 -a3 -b8 -t1000000 < /tmp/trace_file >> /tmp/test.results
	echo "The following diff should be empty"
	diff orig_results /tmp/test.results
