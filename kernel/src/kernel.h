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

typedef struct{
	t_alg_planificador id;
	void (*planificar)(void);
}t_planificador;
typedef struct
{
	int PUERTO_ESCUCHA;
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	char* IP_CPU;
	int PUERTO_CPU_DISPATCH;
	int PUERTO_CPU_INTERRUPT;
	t_planificador ALGORITMO_PLANIFICACION;
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
extern int grado_multiprogamacion_actual;
extern int conexion_memoria, cpu_dispatch,cpu_interrupt,kernel_escucha, conexion_io;
extern int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
extern t_queue* estado_new, *estado_ready, *estado_blocked, *estado_exit, *estado_ready_plus,*estado_exec, *estado_temp;
extern t_pcb* pcb_exec;
/*
extern t_queue* io_stdin;
extern t_queue* io_stdout;
extern t_queue* io_generica;
extern t_queue* io_dialfs;*/

bool iniciar_interrupt();
bool iniciar_dispatch();
bool iniciar_kernel(char*);
bool iniciar_logger_config();
bool inicializar_comandos();
char* leer_texto_consola();
bool iniciar_proceso(char** parametros);
bool finalizar_proceso(char**);
bool iniciar_planificacion(char**);
bool multiprogramacion(char**);
bool detener_planificacion(char**);
bool proceso_estado();
bool modificacion_estado();
void ready_a_exec();
void push_proceso_a_estado(t_pcb*, t_queue* ,pthread_mutex_t*);
t_pcb* pop_estado_get_pcb(t_queue* ,pthread_mutex_t*);
bool transicion_valida(t_queue*,t_queue*);
bool cambio_de_estado(t_queue*, t_queue*,pthread_mutex_t*,pthread_mutex_t*);
bool iniciar_semaforos();
bool finalizar_consola(char**);
void listar_comandos();
t_list* get_comandos();
void loguear_config();
void finalizar_kernel();
int ejecutar_comando_consola(char*params);
bool ejecutar_scripts_de_archivo(char** parametros);
void agregar_comando(op_code_kernel code,char* nombre,char* params, bool(*funcion)(char**));
bool iniciar_estados_planificacion();
bool iniciar_colas_entrada_salida();
void iniciar_consola();
bool iniciar_planificadores();
bool iniciar_sem_multiprogramacion();
void planificador_corto();
void plp_procesos_nuevos();
void plp_procesos_finalizados();
bool es_vrr();
void planificacion_FIFO();
void planificacion_RR();
void planificacion_VRR();
void ejecutar_planificacion();
void ejecutar_proceso();
bool crear_proceso_en_memoria(t_pcb*);
bool eliminar_proceso_en_memoria(t_pcb*);
void proceso_a_estado(t_pcb* pcb, t_queue* estado,pthread_mutex_t* mx_estado);

void recibir_pcb_de_cpu();

void liberar_pcb_exec();
t_comando_consola* comando_consola_create(op_code_kernel code,char* nombre,char* params,bool(*funcion)(char**));
// bool iniciar_conexion_io();
// void io_handler();
// bool iniciar_servidor_kernel();
#endif /* kernel.h*/



/*
Semaforos necesarios:

semáforo de ejecucion -> 1 valor
*/