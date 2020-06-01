
#include <cosas_comunes.h>
#include <pthread.h>
#include "team.h"
#include <commons/collections/list.h>
#include <commons/string.h>

void element_destroyer(void* elemento){
	t_entrenador* ent = (t_entrenador*) elemento;
	free(ent->objetivo);
	free(ent->pokemones);
	free(ent->posicion);
	free(ent->semaforo);
}

/*void algortimoCercano(void* elemento, int posicionPokemonx, int posicionPokemony){
	t_entrenador* ent = (t_entrenador*) elemento;
	ent -> cercania = ((ent -> posicion -> posx) - posicionPokemonx) + ((ent -> posicion -> posy) - posicionPokemony);
}

t_entrenador elegirEntrenadorXCercania(int posx, int posy){
	void _algoritmoCercano(void* elemento){
		algoritmoCercano(elemento, posx, posy);
	}

	t_list* listaFiltrada = list_map(listaBlocked, _algoritmoCercano);
}
*/

void* stream_get_pokemon(char* datos, int* bytes){

	char* nombre_pokemon = datos;
	uint32_t size_nombre = strlen(nombre_pokemon) + 1;

	*bytes = sizeof(uint32_t) + size_nombre;

	void* stream = malloc(*bytes);

	int offset = 0;

	memcpy(stream + offset, &size_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, nombre_pokemon, size_nombre);
	offset += size_nombre;

	return stream;
}

void enviar_mensaje(t_paquete* paquete, int socket_cliente){

	int bytes_enviar;

	void* mensaje = serializar_paquete(paquete, &bytes_enviar);

	if(send(socket_cliente, mensaje, bytes_enviar, 0) < 0)
		perror("[utils_gameboy.c] FALLO EL SEND");


	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(mensaje);
}


void enviarMensajeAlBroker(int codigo_operacion, char* mensaje){
	int socket = crear_conexion("127.0.0.1", "4444");
	int tamanioString;

	void* streamMensaje = stream_get_pokemon(mensaje, &tamanioString);

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete ->  codigo_operacion = codigo_operacion;
	paquete -> buffer = malloc(sizeof(t_buffer));
	paquete -> buffer -> size = tamanioString;
	paquete -> buffer -> stream = malloc(sizeof(tamanioString));
	paquete -> buffer -> stream = streamMensaje;

	enviar_mensaje(paquete, socket);

	int cod_confirmacion;
	int tamanioMensajeBroker;
	int idBroker;

	recv(socket, &cod_confirmacion, sizeof(int), 0);
	recv(socket, &tamanioMensajeBroker, sizeof(int), 0);
	recv(socket, &idBroker, sizeof(int), 0);

	printf("ID DEL MENSAJE BROKER= %d", idBroker);
	fflush(stdout);
}

int main(){

	//LEO ARCHIVO DE CONFIGURACION
	leer_archivo_configuracion();

	pthread_mutex_init(&semPlanificador,NULL);

	int cantEntrenadores = cant_elementos(POSICIONES_ENTRENADORES);
	t_entrenador* entrenadores[cantEntrenadores];
	pthread_t* hilos[cantEntrenadores];

	//TEMPORAL hasta poder mandar mensajes entre procesos
	//char* mensajeConsola = string_new();
	//string_append(&mensajeConsola, argv[1]);

	//creo el diagrama de estados
	listaReady = list_create();
	listaExecute = list_create();
	listaBlocked = list_create(); //sin NEW, inicializamos los entrenadores en BLOCKED
	listaExit = list_create();

	//setteo entrenadores y asigno hilo a c/entrenador
	for(i=0;i<cantEntrenadores;i++){
		setteoEntrenador(entrenadores[i], hilos[i], i);
	}

	printf("/////////////////////////////////////////////////////////\n");
	fflush(stdout);
	pthread_mutex_lock(&semPlanificador);

	enviarMensajeAlBroker(GET_POKEMON, "Pikachu");

	pthread_t servidor;

	pthread_create(&servidor, NULL, (void*) iniciar_servidor, NULL);
	pthread_detach(servidor);

	for(i=0;i<cantEntrenadores;i++){
		pthread_join(*hilos[i],NULL);
	}

	//DEFINIR como destruir elementos
	list_destroy_and_destroy_elements(listaReady, free);
	list_destroy_and_destroy_elements(listaExecute, free);
	list_destroy_and_destroy_elements(listaBlocked, free);
	list_destroy_and_destroy_elements(listaExit, free);


	for(i=0;i<cantEntrenadores;i++){
		free(entrenadores[i]-> posicion);
		free(entrenadores[i]-> objetivo);
		free(entrenadores[i]-> pokemones);
		free(entrenadores[i]);
	}
	return EXIT_SUCCESS;
}

void leer_archivo_configuracion(){
	t_config* config = leer_config("/home/utnso/workspace/tp-2020-1c-Bomberman-2.0/Procesos/Team/team1.config");
		//PASO TODOS LOS PARAMETROS
		POSICIONES_ENTRENADORES = config_get_array_value(config,"POSICIONES_ENTRENADORES");
		POKEMON_ENTRENADORES = config_get_array_value(config,"POKEMON_ENTRENADORES");
		OBJETIVOS_ENTRENADORES = config_get_array_value(config,"OBJETIVOS_ENTRENADORES");
		TIEMPO_RECONEXION = config_get_int_value(config,"TIEMPO_RECONEXION");
		RETARDO_CICLO_CPU = config_get_int_value(config,"RETARDO_CICLO_CPU");
		ALGORITMO_PLANIFICACION = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
		if(strcmp(ALGORITMO_PLANIFICACION,"RR")){
				QUANTUM = config_get_int_value(config,"QUANTUM");
		}
		if(strcmp(ALGORITMO_PLANIFICACION,"SJF")){
				ESTIMACION_INICIAL = config_get_int_value(config,"ESTIMACION_INICIAL");
		}
		IP_BROKER = config_get_string_value(config,"IP_BROKER");
		PUERTO_BROKER= config_get_int_value(config,"PUERTO_BROKER");
		LOG_FILE= config_get_string_value(config,"LOG_FILE");


		//MUESTRO CONFIG EN CONSOLA
		int contador = 0;
		while(POKEMON_ENTRENADORES[contador] != NULL){
			printf("Entrenador %d POKEMONOS: %s\n", contador + 1, POKEMON_ENTRENADORES[contador]);
			printf("Entrenador %d OBJETIVOS: %s\n\n", contador + 1, OBJETIVOS_ENTRENADORES[contador]);
			contador++;
		}
		printf("\nTIEMPO_RECONEXION: %i\n", TIEMPO_RECONEXION);
		printf("RETARDO_CICLO_CPU: %i\n", RETARDO_CICLO_CPU);
		printf("ALGORITMO_PLANIFICACION: %s\n", ALGORITMO_PLANIFICACION);
		printf("IP_BROKER: %s\n", IP_BROKER);
		printf("PUERTO_BROKER: %d\n", PUERTO_BROKER);
		printf("LOG_FILE: %s\n", LOG_FILE);

		config_destroy(config);
}

