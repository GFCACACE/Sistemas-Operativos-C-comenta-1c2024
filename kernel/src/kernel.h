#ifndef kernel_h
#define kernel_h
#include<commons/config.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/queue.h>
#include <utils/utils-client.h>
#include <utils/utils-server.h>
#include <utils/utils-commons.h>
#include <utils/utils-config.h>
#include <readline/readline.h>
#define MODULO "kernel"

typedef enum
{
	FIFO,
	RR,
	VRR
}t_alg_planificador;
typedef struct
{
	int PUERTO_ESCUCHA;
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	//char* IP_FILESYSTEM;
	char* IP_CPU;
	int PUERTO_CPU_DISPATCH;
	int PUERTO_CPU_INTERRUPT;
	t_alg_planificador ALGORITMO_PLANIFICACION;
	int QUANTUM;
	char** RECURSOS;
	char** INSTANCIAS_RECURSOS;
	int GRADO_MULTIPROGRAMACION;
	char* PATH_SCRIPTS;
	
	t_config* config;
} t_config_kernel;


typedef enum
{
	EJECUTAR_SCRIPT,
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	MULTIPROGRAMACION,
	PROCESO_ESTADO,
	EXIT
}op_code_kernel;

typedef struct t_comando_consola {
	op_code_kernel comando;
	char* parametros;
	char* nombre;
	bool (*funcion)(char**);
}t_comando_consola;

t_config_kernel* iniciar_config_kernel(char*);
void config_kernel_destroy(t_config_kernel*);
extern t_log* logger;
extern t_config_kernel* config;
extern int conexion_memoria, cpu_dispatch,cpu_interrupt;
extern int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
extern t_queue* estado_new, *estado_ready, *estado_blocked, *estado_exit, *estado_ready_plus,*estado_exec;
/*extern t_queue* estado_ready;
extern t_queue* estado_ready_plus;
extern t_queue* estado_blocked;
extern t_queue* estado_exit;
extern t_queue* io_stdin;
extern t_queue* io_stdout;
extern t_queue* io_generica;
extern t_queue* io_dialfs;*/


bool iniciar_kernel(char*);
bool iniciar_logger_config();
bool inicializar_comandos();
void consola();
char* leer_texto_consola();
bool iniciar_proceso(char** parametros);
bool finalizar_proceso(char**);
bool iniciar_planificacion(char**);
bool multiprogramacion(char**);
bool detener_planificacion(char**);
bool proceso_estado();
bool finalizar_consola(char**);
void listar_comandos();
t_list* get_comandos();
void loguear_config();
void finalizar_kernel();
bool ejecutar_comando_consola(char*params);
bool ejecutar_scripts_de_archivo(char** parametros);
void agregar_comando(op_code_kernel code,char* nombre,char* params, bool(*funcion)(char**));
bool iniciar_estados_planificacion();
bool iniciar_colas_entrada_salida();
bool iniciar_consola();
bool es_vrr();

#endif /* kernel.h*/