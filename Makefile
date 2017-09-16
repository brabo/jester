CC=gcc

CFLAGS=-I. --std=gnu99 -I./include

JT_OBJ=src/jester.o

LDFLAGS=
LDLIBS=

jester: $(JT_OBJ)
	$(CC) -o $@ $(JT_OBJ) $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	rm -f $(JT_OBJ)

all: jester

clean:
	rm -f src/*.o jester
