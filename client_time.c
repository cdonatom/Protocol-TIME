#include "client_time.h"

#define DIFFERENCE 2208988800UL // RFC
#define DEBUG 1


int sockfd;

//Rutina de atención a la señal de sigint.
void kill_client_tcp (){
	close(sockfd);	//Cierra el socket.
	printf("\nSIGINT received, closing program\n");
	exit(0);
}

int client_tcp ( char* host, char* port, int debug ){
	
	if ( debug == DEBUG )
		printf("Client TCP:\n");
	
	if (port == NULL){
		port = "37";//Puerto 37 por defecto
		if ( debug == DEBUG )
			printf("dbg: port 37 selected by default\n");
	}
	  
    time_t buf;
	struct tm* time;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int n; 

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_INET;//Familia
	hints.ai_socktype = SOCK_STREAM; //Datagramas para TCP
	
	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("client: socket");
			continue;
		}
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if (p == NULL){
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	
	freeaddrinfo(servinfo);	// all done with this structure
	char output[128];
	
	signal(SIGINT, (void*) kill_client_tcp );	//Captura la señal de sigint.
	                
	while(1){
	
		n = recv(sockfd, &buf, sizeof(time_t), 0);	//Queda a la espera de recibir del servidor.
		if (n < 0)
			fprintf(stderr, "socket read failed: %s\n",
                              		strerror(errno));	
        if ( n > 0 ){
			
			if ( debug == DEBUG )
				printf("dbg: received packet.\n");

			buf = ntohl(buf) - DIFFERENCE;//Protocolo TIME cuenta desde 1900, Unix desde 1970
			
			if ( debug == DEBUG )
				printf("dbg: converting time..\n");
				
			time = localtime (&buf);
   			strftime(output,128,"%a %b %d %T %Z %Y", time);	//Presentación correcta de fecha y hora.
			printf ("%s\n", output);
		}
		if ( n == 0 ){
			printf("Server doesn't respond\n");
			close(sockfd);	//El servidor no responde. Cerramos la conexión.
			return -1;
		}
		if ( debug == DEBUG )
			printf("dbg: padding our variables.\n");

		bzero(output, strlen(output));
		bzero(&buf, sizeof(time_t)); 
	}
	return 0;           

}


int client_udp ( char* host, char* port, int debug ){
 	if ( debug == DEBUG )
		printf("Client UDP:\n");
 	
	if (port == NULL){
		port = "37";//Puerto 37 por defecto
		if ( debug == DEBUG )
			printf("dbg: port 37 selected by default\n");
	}
    time_t buf;

	struct addrinfo hints, *servinfo, *p;
	int rv;
	int outchars, n; 

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_INET;//Familia UDP
	hints.ai_socktype = SOCK_DGRAM; //Datagramas para UDP
	
	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("client: socket");
			continue;
		}
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if (p == NULL){
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	freeaddrinfo(servinfo);
	outchars = 0;

	if ( debug == DEBUG )
		printf("dbg: sending empty packet.\n");

	send( sockfd, NULL, outchars, 0);//Enviamos un datagrama vacío.

	n = recv(sockfd, &buf, sizeof(time_t), 0);	//Queda a la espera de recibir del servidor.
	if (n < 0) {
		fprintf(stderr, "socket read failed: %s\n",
                              		strerror(errno));
        return 3;
    }
	if ( debug == DEBUG )
		printf("dbg: received packet.\n");

	buf = ntohl(buf) - DIFFERENCE;	//Protocolo TIME cuenta desde 1900, Unix desde 1970
	
	if ( debug == DEBUG )
				printf("dbg: converting time..\n");
				
	struct tm* time = localtime (&buf);
	char output[128];
    strftime(output,128,"%a %b %d %T %Z %Y", time);	//Presentación correcta de fecha y hora.

	printf ("%s\n", output);
	close( sockfd );	//Cerramos la conexión.
	return 0;

}
