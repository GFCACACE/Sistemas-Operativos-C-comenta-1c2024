#ifndef cpu_h
#define cpu_h

#include<commons/config.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/dictionary.h>
#include "utils/utils-server.h"
#include "utils/utils-client.h"
#include "utils/utils-config.h"
#include "utils/utils-commons.h"
#define MODULO "cpu"
#define EXIT_PROGRAM "EXIT"
typedef struct
{
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	int PUERTO_ESCUCHA_DISPATCH;
	int PUERTO_ESCUCHA_INTERRUPT;
	t_config* config;
} t_config_cpu;

typedef struct {
	int dispatch;
	int interrupt;
	int memoria;
} t_connections_cpu;




bool iniciar_log_config();
bool iniciar_registros_cpu();
bool iniciar_dispatch();
bool iniciar_conexion_memoria();
bool iniciar_conexion_kernel();
bool iniciar_semaforos();


bool iniciar_cpu(char*);

t_config_cpu* iniciar_config_cpu(char*);
t_dictionary* iniciar_diccionario_cpu();
void config_destroy_cpu(t_config_cpu*);
void finalizar_cpu();
void finalizar_estructuras_cpu();
void loguear_config();
int ejecutar_proceso_cpu();
int intentar_conexion(char*,int,char*);
char* pedir_proxima_instruccion(t_pcb* pcb);

//Ciclo de Instrucción//////////////////////
void ciclo_de_instruccion(t_pcb* pcb);
void* interpretar_valor_instruccion(char*);
bool fetch(t_pcb*);
bool decode();
bool execute(t_pcb*);
bool check_interrupt();
bool devolver_contexto(t_pcb*);
bool actualizar_contexto(t_pcb*);
bool actualizar_registros(t_pcb*);
bool exe_set(void*,void*);
bool exe_mov_in(void*,void*);
bool exe_mov_out(void*,void*);
bool exe_sum(void*,void*);
bool exe_sub(void*,void*);
bool exe_jnz(void*,void*);
bool exe_resize(void*);
bool exe_copy_string(void*);
bool exe_wait(/*recurso*/);
bool exe_signal(/*recurso*/);
bool exe_exit();
bool exe_io_gen_sleep(void*,void*);
/*Faltan las instrucciones de IO*/
///////////////////////////////////

extern t_log* logger;
extern t_config_cpu * config;
extern t_registros_cpu* registros_cpu;
extern int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
extern int cod_op_kernel_dispatch;
extern t_dictionary* diccionario_registros_cpu;
// Cambiar funciones en base a esto!!!!
extern pthread_t* mutex_interrupt;
extern char* IR;
extern char* INSTID;
extern void* PARAM1;
extern void* PARAM2;
extern void* PARAM3;
extern int cod_op_interrupt;
#endif //cpu_h
