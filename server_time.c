#include "server_time.h"

#define DIFFERENCE 2208988800UL // RFC
#define BACKLOG 10	 // how many pending connections queue will hold
#define BUFSIZE 4096
#define DEBUG 1

int sockfd, new_fd;

//Rutina de atención a la señal SIGPIPE.
void broken_pipe( void ){
    printf("Server: client closed connection.\n");
	//Cierra el socket asignado al hijo (cliente).
	close(new_fd);
	exit(0);
}

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int server_time ( int debug ){

	if ( debug == DEBUG )
			printf("Server TIME:\n");
	
	// listen on sock_fd, new connection on new_fd.
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information.
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	int rv;
	char s[INET6_ADDRSTRLEN];
	time_t local_time;
	char* port = "6008"; //6000+#grupo

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	//Familia.
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction\n");
		exit(1);
	}

	printf("TIME server running in port %s\n", port );

	while(1) {  // main accept() loop
		sin_size = sizeof (their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		if ( debug == DEBUG )
			printf("dbg: connection accepted.\n");

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s); //Convierte una estructura de direccion a String.
		
        if ( debug == DEBUG )
            printf("dbg: connected with %s\n", s);
		
		if (!fork()) { // this is the child process.
			signal(SIGPIPE, (void*) broken_pipe );	//Captura la señal SIGPIPE, que indica que no puede escribir en el socket.
			ssize_t n = 7;
			close(sockfd); // child doesn't need the listener
			//int buf[BUFSIZE];
			while( n != -1 ){
				local_time = time(NULL);//Obtenemos la hora del sistema
				local_time = htonl(local_time + DIFFERENCE);//Le damos la vuelta y le sumamos la diferencia de tiempo.
				n = sendto(new_fd, (void*) &local_time, sizeof(local_time), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
				
				if ( debug == DEBUG )
					printf("dbg: time send to %s.\n", s);
				
				sleep(1);

			}
            if ( debug == DEBUG )
                printf("dbg: closing connection with %s\n", s);
			close(new_fd);	//Cierro el socket.
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
