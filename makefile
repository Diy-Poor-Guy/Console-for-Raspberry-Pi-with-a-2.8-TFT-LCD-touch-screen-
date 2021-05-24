# The name of your C compiler:
CC= gcc

# You may need to adjust these cc options:
CFLAGS= -Wall -c -o

all: console 
 
TftLcd.o: TftLcd.c  TftLcd.h font_8X8.h
	$(CC) $(CFLAGS) TftLcd.o TftLcd.c

.c.o: $*.h
	$(CC) $(CFLAGS) $*.o $*.c

clean:
	rm *.o
	rm console
 
console: console.o touch.o TftLcd.o keyboard.o font_8X8.h
	gcc -o console console.o touch.o TftLcd.o keyboard.o -lpthread

