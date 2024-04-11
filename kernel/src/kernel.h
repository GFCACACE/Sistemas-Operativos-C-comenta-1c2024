#ifndef kernel_h
#define kernel_h
#include<commons/config.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/queue.h>
#include <utils/utils-client.h>
#include <utils/utils-server.h>
#include <utils/utils-config.h>
#include <readline/readline.h>
#define MODULO "kernel"

typedef struct
{
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	char* IP_FILESYSTEM;
	int PUERTO_FILESYSTEM;
	char* IP_CPU;
	int PUERTO_CPU_DISPATCH;
	int PUERTO_CPU_INTERRUPT;
	char*  ALGORITMO_PLANIFICACION;
	int QUANTUM;
	char** RECURSOS;
	char** INSTANCIAS_RECURSOS;
	int GRADO_MULTIPROGRAMACION;
	
	t_config* config;
} t_config_kernel;


typedef enum
{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO,
	EXIT
}op_code_kernel;

t_config_kernel* iniciar_config_kernel(char*);
void config_kernel_destroy(t_config_kernel*);
extern t_log* logger;
extern t_config_kernel* config;
extern int conexion_memoria, cpu_dispatch,cpu_interrupt;
extern int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;


bool iniciar_kernel(char*);
void consola();
char* leer_texto_consola();
void iniciar_proceso(char** parametros);
void finalizar_proceso(char**);
void iniciar_planificacion(char**);
void multiprogramacion(char**);
void detener_planificacion(char**);
void proceso_estado(char**);
void listar_comandos();
op_code_kernel codigo_comando(char*);
void loguear_config();
void finalizar_kernel();

#endif /* kernel.h*/