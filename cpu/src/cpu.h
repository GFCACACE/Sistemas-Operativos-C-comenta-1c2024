#ifndef cpu_h
#define cpu_h

#include<commons/config.h>
#include<commons/log.h>
#include<commons/string.h>
#include<math.h>
#include<commons/collections/queue.h>
#include<commons/collections/list.h>
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
	int CANTIDAD_ENTRADAS_TLB;
	char* ALGORITMO_TLB;
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

typedef struct 
{
	uint32_t PID;
	uint32_t numero_pagina;
	uint32_t numero_frame;
	time_t timestamp; 
}t_tlb;


bool iniciar_log_config();
bool iniciar_registros_cpu();
bool iniciar_dispatch();
bool iniciar_conexion_memoria();
bool iniciar_conexion_kernel();
bool iniciar_variables();//semáforos y variables globales
bool iniciar_gestion_interrupcion();
bool iniciar_tlb();
bool iniciar_cpu(char*);
bool es_io_handler(char*);
t_config_cpu* iniciar_config_cpu(char*);
t_dictionary* iniciar_diccionario_cpu();
void config_destroy_cpu(t_config_cpu*);
void finalizar_cpu();
void finalizar_estructuras_cpu();
void loguear_config();
void* gestionar_interrupcion();
int ejecutar_proceso_cpu();
int intentar_conexion(char*,int,char*);
char* pedir_proxima_instruccion(t_pcb* pcb);
uint32_t obtener_direccion_fisica (t_pcb* pcb, uint32_t numero_pagina,uint32_t desplazamiento);
//Ciclo de Instrucción//////////////////////
void ciclo_de_instruccion(t_pcb* pcb);
t_param interpretar_valor_instruccion(char*);
bool fetch(t_pcb*);
bool decode();
bool execute(t_pcb*);
bool check_interrupt();
bool devolver_contexto(t_pcb *pcb,op_code codigo_operacion);
bool actualizar_contexto(t_pcb*);
bool actualizar_registros(t_pcb*);
u_int32_t calcular_bytes_restantes(uint32_t valor);
bool exe_set(t_param,t_param);
bool exe_mov_in(t_pcb*,t_param,t_param);
bool exe_mov_out(t_pcb*,t_param,t_param);
bool exe_sum(t_param,t_param);
bool exe_sub(t_param,t_param);
bool exe_jnz(t_param,t_param);
bool exe_resize(t_pcb*,t_param);
bool exe_copy_string(t_pcb*,t_param);
//bool exe_stdin_read(t_pcb* pcb, t_param interfaz, t_param registro_direccion,t_param registro_tamanio);
//bool exe_stdout_write(t_pcb* pcb, t_param interfaz, t_param registro_direccion,t_param registro_tamanio);
bool exe_std(op_code cod_op, t_pcb* pcb, t_param interfaz, t_param registro_direccion,t_param registro_tamanio);

bool exe_wait(t_pcb* pcb,t_param);
bool exe_signal(t_pcb* pcb,t_param);
bool exe_exit(t_pcb*);
bool exe_io_gen_sleep(t_pcb*,t_param,t_param);
/*Faltan las instrucciones de IO*/
///////////////////////////////////
/*MMU*/
t_direcciones_proceso* obtener_paquete_direcciones(t_pcb* pcb,uint32_t direccion_logica, uint32_t size_registro);
uint32_t obtener_numero_pagina(uint32_t direccion_logica);
uint32_t obtener_desplazamiento(uint32_t direccion_logica,uint32_t numero_pagina);
uint32_t obtener_cantidad_paginas(uint32_t desplazamiento,uint32_t size_registro);
uint32_t mmu(t_pcb* pcb,uint32_t direccion_logica);
t_tlb* crear_registro_tlb(uint32_t PID, uint32_t numero_pagina, uint32_t numero_frame);
int tlb_hit(uint32_t pid, uint32_t numero_pagina);
bool actualizar_tlb(uint32_t PID, uint32_t numero_pagina, uint32_t numero_frame);
bool reemplazo_tlb(t_tlb* registro_nuevo);
void escribir_memoria(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion);
t_buffer* leer_memoria(t_direcciones_proceso* direcciones_fisicas_registros);
//////////////////////////////////
extern int tamanio_pagina;
extern t_log* logger;
extern t_config_cpu * config;
extern t_registros_cpu* registros_cpu;
extern int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
//extern op_code cod_op_kernel_dispatch;
extern t_dictionary* diccionario_registros_cpu;
// Cambiar funciones en base a esto!!!!
extern pthread_mutex_t mutex_interrupt;
extern char* IR;
extern char* INSTID;
extern t_param PARAM1;
extern t_param PARAM2;
extern t_param PARAM3;
extern op_code cod_op_kernel_interrupt;
extern t_list* lista_tlb;
#endif //cpu_h
