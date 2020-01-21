#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_time.h"
#include "server_time.h"

#define MIN_OPTION 2
#define MAX_OPTION 7

int main (int argc, char *argv[]){

	if ( argc < MIN_OPTION + 1 || argc > MAX_OPTION + 1 ){
		printf("Uso: %s [-h serverhost] [-p port] [-m cu | ct | s ] [-d]\n", argv[0]);
		exit(0);
	}
	
	char* option[] = {"-h", "-p", "-m", "-d"};	//Opciones soportadas.
	char* modes[] = {"cu", "ct", "s"};	//Modos soportados: udp,tcp y servidor.
	char* mode_exec; 
	int mode_dbg = 0;
	char* port = NULL;
	char* host = NULL;
	
	int i = 1;
	int j;
	
	//Recoge los argumentos.
	do{
		for (j = 0; j < 4; j++){
			if ( strcmp(argv[i], option[j]) == 0 )
				break;
		}
        if( j < 4 ){
			switch (j){
				case 0:
					host = argv[i+1];
					break;
				case 1:
					port = argv[i+1];
					break;
				case 2:
					mode_exec = argv[i+1];
					break;
				case 3:
					mode_dbg = 1;
					break;
			}
		}
		i++;

	}
	while( i < argc );	
	
	//Comprobamos el modo de ejecución.
	for(j = 0; j < 3; j++){
		if ( strcmp ( mode_exec, modes[j]) == 0 ){		
			break;
		}
	}
	if ( j < 3 ){
		switch( j ){
			case 0:			
				return client_udp ( host, port, mode_dbg );
			case 1:	
				return client_tcp ( host, port, mode_dbg );
			case 2:
				return server_time( mode_dbg );
		}
	}
	
	printf("Introduzca correctamente las opciones.\n");
	printf("[-h serverhost] [-p port] [-m cu | ct | s ] [-d]\n");
	
	return -1;
}
