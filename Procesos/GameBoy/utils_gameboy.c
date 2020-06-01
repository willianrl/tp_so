#include "utils_gameboy.h"


void inicializar_archivos(){

	CONFIG = leer_config("/home/utnso/workspace/tp-2020-1c-Bomberman-2.0/Procesos/GameBoy/gameboy.config");
	LOGGER = iniciar_logger("/home/utnso/workspace/tp-2020-1c-Bomberman-2.0/Procesos/GameBoy/gameboy.log", "gameboy", 1, LOG_LEVEL_INFO);
}


static char* comprobar_proceso(char *proceso){

	if(string_equals_ignore_case(proceso, "SUSCRIPTOR") == 1)
		return "broker";
	return proceso;
}


void obtener_direcciones_envio(char* proceso){

	proceso = comprobar_proceso(proceso);

	char* ip_key = obtener_key("ip", proceso);
	char* puerto_key = obtener_key("puerto", proceso);

	IP_SERVER = config_get_string_value(CONFIG, ip_key );
	PUERTO_SERVER = config_get_string_value(CONFIG, puerto_key );

	free(ip_key);
	free(puerto_key);
}


void enviar_mensaje(t_paquete* paquete, int socket_cliente){

	int bytes_enviar;

	void* mensaje = serializar_paquete(paquete, &bytes_enviar);

	if(send(socket_cliente, mensaje, bytes_enviar, 0) < 0)
		perror("[utils_gameboy.c] FALLO EL SEND");

	log_info(LOGGER, "Se creo la conexion con el proceso IP = %s, PUERTO = %s\n", IP_SERVER, PUERTO_SERVER);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(mensaje);
}


static void modo_suscriptor(int conexion){

	int cod_op, size, id;
	void*datos;

	while(1){
		if(recv(conexion, &cod_op, sizeof(uint32_t), 0 ) < 0){
			perror("[gameboy.c : 28]FALLO RECV");
			continue;
		}
		switch(cod_op){

			case CONFIRMACION:
				recv(conexion, &id, sizeof(uint32_t), 0);

				printf("[CONFIRMACION DE SUSCRIPCION]cod_op = %d, mi id de suscriptor= %d \n", cod_op, id);
				break;

			case NEW_POKEMON...LOCALIZED_POKEMON:
				recv(conexion, &id, sizeof(uint32_t), 0);
				recv(conexion, &size, sizeof(uint32_t), 0);
				datos = malloc(size);
				recv(conexion, datos, size, 0);

				printf("[MENSAJE DE UNA COLA DEL BROKER]cod_op = %d, id correlativo = %d, size mensaje = %d \n", cod_op, id, size);
				break;
			}
	}
}


static void modo_emisor(int conexion){

	int cod_op, id;

	recv(conexion, &cod_op, sizeof(uint32_t), 0);
	recv(conexion, &id, sizeof(uint32_t), 0);

	printf("[CONFIRMACION DEL MENSAJE] cod_op = %d, id mensaje en el broker = %d \n", cod_op, id);
}


void esperando_respuestas(int socket, char* modo){

	if(string_equals_ignore_case(modo, "SUSCRIPTOR") == 1)
			modo_suscriptor(socket);
		modo_emisor(socket);
}





