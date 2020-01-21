#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>

int client_tcp ( char* host, char* port, int debug );
int client_udp ( char* host, char* port, int debug );
