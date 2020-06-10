#include <stdio.h>
#include <sys/mount.h>
#include "servidor.h"

void* mensaje_suscripcion(int cod_op, char* datos[], int *size);
void* stream_suscripcion(char* datos[], int* size);

int TIEMPO_DE_REINTENTO_CONEXION;
int TIEMPO_DE_REINTENTO_OPERACION;
int TIEMPO_RETARDO_OPERACION;
char* PUNTO_MONTAJE_TALLGRASS;
char* IP_BROKER;
int PUERTO_BROKER;

void* mensaje_suscripcion(int cod_op, char* datos[], int *size){

	void* mensaje = stream_suscripcion(datos, size);

	void* stream = malloc( 2 * sizeof(uint32_t) + *size);

	int offset = 0;

	memcpy(stream + offset, &cod_op, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, size, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, mensaje, *size);
	offset += *size;

	*size = offset;
	free(mensaje);
	return stream;
}

//MENSAJE DE SUSCRIPCION
void* stream_suscripcion(char* datos[], int* size){

	uint32_t t_mensaje = codigo_operacion(datos[0]),
			 tiempo_suscripcion = atoi(datos[1]);

	*size = 2 * sizeof(uint32_t);

	void* stream = malloc(*size);

	int offset = 0;

	memcpy(stream + offset, &t_mensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, &tiempo_suscripcion, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	return stream;
}


void suscribirse(char* cola){
	int socket = crear_conexion(IP_BROKER, PUERTO_BROKER);

	char *datos[] = {cola, "-1"};

	int size;
	void* mensajeAEnviar = mensaje_suscripcion(SUSCRIPTOR, datos, &size);

	send(socket, mensajeAEnviar, size, 0);

	int cod_op, id_correlativo;
	void* buffer;

	while(1){
		if(recv(socket, &cod_op, sizeof(uint32_t), 0 ) < 0){
			perror("FALLO RECV");
			continue;
		}
	recv(socket, &id_correlativo, sizeof(uint32_t), 0);

		switch(cod_op){

			case CONFIRMACION:
				printf("[CONFIRMACION DE SUSCRIPCION]cod_op = %d, mi id de suscriptor= %d \n", cod_op, id_correlativo);
				break;

			case NEW_POKEMON:
				recv(socket, &size, sizeof(int), 0);
				recv(socket, buffer, size, 0);
				printf("Se recibio un new_pokemon del broker\n");
				break;

			case CATCH_POKEMON:
				recv(socket, &size, sizeof(int), 0);
				recv(socket, buffer, size, 0);
				printf("Se recibio un catch_pokemon del broker\n");
				break;

			case GET_POKEMON:
				recv(socket, &size, sizeof(int), 0);
				recv(socket, buffer, size, 0);
				printf("Se recibio un get_pokemon del broker\n");
				break;
		}
	}
}

void leer_archivo_configuracion(){

	t_config* config = leer_config("/home/utnso/workspace/tp-2020-1c-Bomberman-2.0/Procesos/GameCard/gamecard.config");

	TIEMPO_DE_REINTENTO_CONEXION = config_get_int_value(config,"TIEMPO_DE_REINTENTO_CONEXION");
	TIEMPO_DE_REINTENTO_OPERACION = config_get_int_value(config,"TIEMPO_DE_REINTENTO_OPERACION");
	TIEMPO_RETARDO_OPERACION = config_get_int_value(config,"TIEMPO_RETARDO_OPERACION");
	PUNTO_MONTAJE_TALLGRASS = config_get_string_value(config,"PUNTO_MONTAJE_TALLGRASS");
	IP_BROKER = config_get_string_value(config,"IP_BROKER");
	PUERTO_BROKER = config_get_int_value(config,"PUERTO_BROKER");

	config_destroy(config);
}


int main(){
	
	pthread_t tid;

	pthread_create(&tid, NULL, (void*)iniciar_servidor, NULL);

	pthread_create(&tid, NULL, (void*)suscribirse, "new_pokemon");
	pthread_create(&tid, NULL, (void*)suscribirse, "catch_pokemon");
	pthread_create(&tid, NULL, (void*)suscribirse, "get_pokemon");

	leer_archivo_config();


	pthread_join(tid, NULL);

	return 0;
}




