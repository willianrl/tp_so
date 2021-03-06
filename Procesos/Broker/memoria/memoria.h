#ifndef MEMORIA_MEMORIA_H_
#define MEMORIA_MEMORIA_H_

#include "../variables_globales.h"


//-------------  ESTRUCTURAS  ----------------------------


typedef struct{

	void* inicio_particion;
	void* fin_particion;
	bool libre;
	char* flag;
	int fifo;

	int id_mensaje;
	int cola_pertenece;
	int size_mensaje;
	clock_t ultimo_acceso;

} t_particion;

pthread_mutex_t MUTEX_PARTICIONES;


//------------  VARIABLES  --------------------------------

void* inicio_memoria; /* la memoria */
t_list* particiones;  /* para la manipulacion de memoria*/
int flag_memoria;  /*para la consolidacion de buddy_system*/
int fifo;


int TAMANO_MEMORIA;
int TAMANO_MINIMO_PARTICION;
char* ALGORITMO_MEMORIA;
char* ALGORITMO_REEMPLAZO;
char* ALGORITMO_PARTICION_LIBRE;
int FRECUENCIA_COMPACTACION;


//-----------------  FUNCIONES -----------------------------
/*
name: iniciar_memoria
desc: inicia todas las variables de la memoria. Necesaria para que la memoria funcione
*/
void iniciar_memoria();
/*
name: pedir_memoria
desc: dado un tamnio te devuelve una particion de memoria en donde puedes guardar ese datos. semejante al malloc
*/
//void* pedir_memoria(int size);
void* pedir_memoria(int size, int id_mensaje, int cod_op);
/*
name: eliminar_particion
desc: elimina una particion por fifo o lru
*/
void eliminar_particion();
/*
name: dump_memoria
desc: describe toda la informacion de la memoria
*/
void dump_memoria();
/*
name: compactar
desc: comprime la memoria
*/
void compactar();

//buscar una particion
void* buscar_particion(int id_mensaje);

void finalizar_memoria();


#endif /* MEMORIA_MEMORIA_H_ */

