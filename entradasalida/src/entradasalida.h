#ifndef entradasalida_h
#define entradasalida_h
#include <utils/hello.h>
#include<commons/config.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/queue.h>
#include <utils/utils-client.h>
#include <utils/utils-server.h>
#include <utils/utils-commons.h>
#include <utils/utils-config.h>
#include <readline/readline.h>
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
	
	t_config* config;
} t_config_io;


t_config_io* iniciar_config_io(char*);
void config_io_destroy(t_config_io*);
extern t_log* logger;
extern t_config_io* config;
extern int conexion_memoria, conexion_kernel;
extern int cod_op_kernel,cod_op_memoria;


bool iniciar_io(char*);
void config_io_destroy(t_config_io*);
void loguear_config();
void finalizar_io();



#endif