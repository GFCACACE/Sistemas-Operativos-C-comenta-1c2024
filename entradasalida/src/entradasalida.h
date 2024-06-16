#ifndef entradasalida_h
#define entradasalida_h
#include<commons/config.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/queue.h>
#include <utils/utils-client.h>
#include <utils/utils-server.h>
#include <utils/utils-serializacion.h>
#include <utils/utils-commons.h>
#include <utils/utils-config.h>
#include <readline/readline.h>
#include "generica.h"
#include "stdin.h"
#include "stdout.h"
#define MODULO "entradasalida"

typedef struct
{
	char* TIPO_INTERFAZ;
    int TIEMPO_UNIDAD_TRABAJO;
    char* IP_KERNEL;
    int PUERTO_KERNEL;
    char* IP_MEMORIA;
    int PUERTO_MEMORIA;
    char* PATH_BASE_DIALFS;
    int BLOCK_SIZE;
    int BLOCK_COUNT;
    char* NOMBRE;
	
	t_config* config;
} t_config_io;

typedef struct{
    int cod_op;
    char* peticion;
}t_peticion_io;


typedef enum
{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} t_interfaz;

t_config_io* iniciar_config_io(char*, char*);
void config_io_destroy(t_config_io*);
bool iniciar_log_config(char*, char*);
extern t_log* logger;
extern t_config_io* config;
extern t_queue* cola_peticiones_io;
extern int conexion_memoria, conexion_kernel;
extern int cod_op_kernel,cod_op_memoria;
extern sem_t sem_bin_cola_peticiones; 


bool iniciar_io(char*,char*);
bool iniciar_semaforo_y_cola();
void config_io_destroy(t_config_io*);
void loguear_config();
void loguear_config_generica();
void loguear_config_stdin();
void loguear_config_stdout();
void loguear_config_dialfs();
void recibir_io();
int ejecutar_op_io();
void finalizar_io();
#endif