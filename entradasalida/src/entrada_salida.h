#ifndef entrada_salida_h
#define entrada_salida_h
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
#define MODULO "entrada_salida"



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
} t_config_entrada_salida;

bool iniciar_entrada_salida(char*);
void config_entrada_salida_destroy();
void loguear_config_entrada_salida();
void finalizar_entrada_salida();
t_config_entrada_salida* iniciar_config_entrada_salida(char* config_path);
extern t_log* logger;
extern t_config_entrada_salida* config_entrada_salida;
extern int memoria_escucha, conexion_memoria, conexion_kernel;


#endif