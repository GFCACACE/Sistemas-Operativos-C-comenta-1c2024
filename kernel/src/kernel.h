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
#include <readline/history.h>
#include <time.h>
#define MODULO "kernel"

typedef enum t_alg_planificador
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

typedef enum
{
	NEW,
	READY,
	BLOCKED,
	EXEC,
	EXIT_STATE
}t_codigo_estado;

typedef struct t_comando_consola {
	op_code_kernel comando;
	char* parametros;
	char* nombre;
	bool (*funcion)(char**);
}t_comando_consola;

typedef struct t_pcb_query {
	t_pcb* pcb;
	t_queue* estado;
}t_pcb_query;

typedef struct{
	t_queue* estado_blocked;
	pthread_mutex_t mx_blocked;
}t_blocked_interfaz;

t_config_kernel* iniciar_config_kernel(char*);
void config_kernel_destroy(t_config_kernel*);
extern t_log* logger;
extern t_config_kernel* config;
extern t_dictionary* diccionario_conexiones_io, *diccionario_struct_io;
extern int grado_multiprogamacion_actual;
extern int conexion_memoria, cpu_dispatch,cpu_interrupt,kernel_escucha, conexion_io;
extern int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
extern t_queue* estado_new, *estado_ready, *estado_exit, *estado_ready_plus,*estado_exec, *estado_temp;
extern t_pcb* pcb_exec;
extern t_list* lista_interfaces_blocked;
extern bool planificacion_detenida;
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
char *recibir_nombre(int);
bool iniciar_proceso(char** parametros);
bool finalizar_proceso(char**);
bool iniciar_planificacion(char**);
bool detener_planificacion(char**);
bool multiprogramacion(char**);
bool detener_plani(char**);
bool proceso_estado();
bool modificacion_estado();
t_pcb* ready_a_exec();
t_pcb* ready_plus_a_exec();
void a_ready(t_pcb* pcb);
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
//bool iniciar_colas_entrada_salida();
void iniciar_consola();
bool iniciar_planificadores();
bool iniciar_sem_multiprogramacion();
bool inicializar_dictionario_mutex_colas();
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
bool eliminar_proceso_en_blocked(uint32_t pid_buscado);
bool eliminar_proceso_en_exec(uint32_t pid);
void proceso_a_estado(t_pcb* pcb, t_queue* estado,pthread_mutex_t* mx_estado);


void recibir_pcb_de_cpu();
void pasar_a_exit(t_pcb*);

void liberar_pcb_exec();
t_comando_consola* comando_consola_create(op_code_kernel code,char* nombre,char* params,bool(*funcion)(char**));
bool iniciar_threads_io();
void iniciar_conexion_io();

void io_handler(int* conexion);
void io_gen_sleep(int pid,char** splitter);
//void io_stdin(int pid,char** splitter);
bool le_queda_quantum(t_pcb* pcb);
bool iniciar_servidor_kernel();
bool existe_interfaz(char*);
bool eliminar_proceso(uint32_t);
bool eliminar_proceso_en_lista(uint32_t pid_buscado,t_queue* estado_buscado ,pthread_mutex_t* mutex_estado_buscado);
t_pcb* encontrar_en_lista(uint32_t pid_buscado,t_queue* estado_buscado ,pthread_mutex_t* mutex_estado_buscado);


t_queue* buscar_cola_de_pcb(uint32_t pid);
t_pcb* buscar_pcb_en_cola(t_queue* cola,uint32_t pid);
t_pcb_query* buscar_pcb(uint32_t pid);
t_list* get_estados();

void bloquear_mutex_colas();
void desbloquear_mutex_colas();

void modificar_quantum_restante(t_pcb* pcb);

#endif /* kernel.h*/



/*
Semaforos necesarios:

semáforo de ejecucion -> 1 valor
*/