
CC=gcc
CFLAGS=-lpaho-mqtt3c
DEPS=MiS.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


MiS_LCD: MiS_LCD.o
	$(CC) -o MiS_LCD MiS_LCD.o $(CFLAGS)

MiS_MSG: MiS_MSG.o
	$(CC) -o MiS_MSG MiS_MSG.o $(CFLAGS)

MiS_RST: MiS_RST.o
	$(CC) -o MiS_RST MiS_RST.o $(CFLAGS)

MiS_PSW: MiS_PSW.o
	$(CC) -o MiS_PSW MiS_PSW.o

.PHONY:	clean

clean:
	rm -f *.o 
