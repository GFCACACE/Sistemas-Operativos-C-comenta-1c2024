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
#include <stdint.h>
#define MODULO "cpu"
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

typedef struct
{
	void* puntero;
	int size;
	char* string_valor;
}t_param;



bool iniciar_log_config();
bool iniciar_registros_cpu();
bool iniciar_dispatch();
bool iniciar_conexion_memoria();
bool iniciar_conexion_kernel();
bool iniciar_variables();//semáforos y variables globales


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
t_param interpretar_valor_instruccion(char*);
bool fetch(t_pcb*);
bool decode();
bool execute(t_pcb*);
bool check_interrupt(t_pcb*);
bool devolver_contexto(t_pcb*);
bool actualizar_contexto(t_pcb*);
bool actualizar_registros(t_pcb*);
bool exe_set(t_param,t_param);
bool exe_mov_in(t_param,t_param);
bool exe_mov_out(t_param,t_param);
bool exe_sum(t_param,t_param);
bool exe_sub(t_param,t_param);
bool exe_jnz(t_param,t_param);
bool exe_resize(void*);
bool exe_copy_string(void*);
bool exe_wait(/*recurso*/);
bool exe_signal(/*recurso*/);
bool exe_exit(t_pcb*);
bool exe_io_gen_sleep(t_param,t_param);
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
extern t_param PARAM1;
extern t_param PARAM2;
extern t_param PARAM3;
extern int cod_op_interrupt;
#endif //cpu_h
