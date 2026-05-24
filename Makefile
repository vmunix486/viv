CC=gcc
# CFLAGS=-O3 -std=c89 -Wall -Wextra 
CFLAGS=-O3 -w
X11=/usr/X11
X11LIB=$(X11)/lib64
X11INC=$(X11)/include
W32FLAGS=-lgdi32 -mwindows
INCLUDES=-Isrc/include
RM=rm
RMFLAGS=-f -v

all:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o bin/image.o src/core/image.c

clean:
	$(RM) $(RMFLAGS) bin/*.o 	
