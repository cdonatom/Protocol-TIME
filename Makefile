# Local configuration. Change according to your system.
CC=gcc

all: atdate

atdate: atdate.c client_time.c server_time.c
	$(CC) -Wall -g atdate.c client_time.c server_time.c -o atdate
clean: 
	rm -f *.o atdate


.PHONY: all clean