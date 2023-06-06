CFLAGS?=-g -Wall -Wno-unused-value

all: metrorec

metrorec: metrorec-runner.c metrorec.c
	$(CC) $(CFLAGS) -o metrorec metrorec-runner.c -lpthread 

clean:
	$(RM) metrorec reaction
	$(RM) -r *.dSYM
