CFLAGS   = -Wall -g -I ../include -fPIC
LD       = gcc
LDFLAGS  = -Wall -g -L../lib64
AS       = gcc
ASFLAGS  = -Wall -g -I ../include -c

PROGS    = snakes nums hungry
SNAKEOBJS  = randomsnakes.o util.o
SNAKELIBS  = -lPLN -lsnakes -lncurses -lrt
HUNGRYOBJS = hungrysnakes.o util.o
NUMOBJS    = numbersmain.o
OBJS       = $(SNAKEOBJS) $(HUNGRYOBJS) $(NUMOBJS)
EXTRACLEAN = core $(PROGS)

# Add the new library target
LIBLWP_OBJS = lwp.o scheduler.o ../src/magic64.o
LIBLWP      = liblwp.so

.PHONY: all allclean clean rs hs ns

# Default target to build all programs and liblwp.so
all: $(LIBLWP) $(PROGS)

# Rule to build liblwp.so
$(LIBLWP): $(LIBLWP_OBJS)
	$(LD) -shared -o $@ $(LIBLWP_OBJS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:
	rm -f $(OBJS) $(LIBLWP_OBJS) $(LIBLWP) *~ TAGS ../src/*.o

snakes: $(SNAKEOBJS) ../lib64/libPLN.so ../lib64/libsnakes.so
	$(LD) $(LDFLAGS) -o snakes $(SNAKEOBJS) $(SNAKELIBS)

hungry: $(HUNGRYOBJS) ../lib64/libPLN.so ../lib64/libsnakes.so
	$(LD) $(LDFLAGS) -o hungry $(HUNGRYOBJS) $(SNAKELIBS)

nums: $(NUMOBJS) ../lib64/libPLN.so
	$(LD) $(LDFLAGS) -o nums $(NUMOBJS) -lPLN

hungrysnakes.o: hungrysnakes.c ../include/lwp.h ../include/snakes.h
	$(CC) $(CFLAGS) -c hungrysnakes.c

randomsnakes.o: randomsnakes.c ../include/lwp.h ../include/snakes.h
	$(CC) $(CFLAGS) -c randomsnakes.c

numbersmain.o: numbersmain.c ../include/lwp.h
	$(CC) $(CFLAGS) -c numbersmain.c

util.o: util.c ../include/lwp.h ../include/util.h ../include/snakes.h
	$(CC) $(CFLAGS) -c util.c

lwp.o: lwp.c scheduler.h
	$(CC) $(CFLAGS) -c lwp.c

scheduler.o: scheduler.c scheduler.h
	$(CC) $(CFLAGS) -c scheduler.c

# Rule to build the magic64.o object file from magic64.S
../src/magic64.o: ../src/magic64.S
	$(AS) $(ASFLAGS) -o $@ $<

rs: snakes
	(export LD_LIBRARY_PATH=../lib64; ./snakes)

hs: hungry
	(export LD_LIBRARY_PATH=../lib64; ./hungry)

ns: nums
	(export LD_LIBRARY_PATH=../lib64; ./nums)
