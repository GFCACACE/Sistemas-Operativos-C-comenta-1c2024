#ifndef cpu_h
#define cpu_h

#include<commons/config.h>
#include<commons/log.h>
#include "utils/utils-server.h"
#include "utils/utils-client.h"
#include "utils/utils-config.h"
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

typedef struct 
{
	uint32_t* PC;
	uint8_t* AX;
	uint8_t* BX;
	uint8_t* CX;
	uint8_t* DX;
	uint32_t* EAX;
	uint32_t* EBX;
	uint32_t* ECX;
	uint32_t* EDX;
	uint32_t* SI;
	uint32_t* DI;
} t_regist_cpu;

bool iniciar_log_config();
bool iniciar_registros_cpu();
bool iniciar_dispatch();
bool iniciar_conexion_memoria();
bool iniciar_conexion_kernel();

bool iniciar_cpu(char*);

t_config_cpu* iniciar_config_cpu(char*);
void config_destroy_cpu(t_config_cpu*);
void finalizar_cpu();
void finalizar_estructuras_cpu();
void loguear_config();
void ejecutar_programa(t_pcb* pcb);
int intentar_conexion(char*,int,char*);
char* pedir_proxima_instruccion(t_pcb* pcb);
//Instrucciones//////////////////////
bool exe_set(uint32_t*,uint32_t);
bool exe_mov_in(uint32_t*,uint32_t*);
bool exe_mov_out(uint32_t*,uint32_t*);
bool exe_sum(uint32_t*,uint32_t);
bool exe_sub(uint32_t*,uint32_t);
bool exe_jnz(uint32_t*,uint32_t);
bool exe_resize(uint32_t);
bool exe_copy_string(uint32_t);
bool exe_wait(/*recurso*/);
bool exe_signal(/*recurso*/);
bool exe_exit();
bool exe_io_gen_sleep(/*interfaz, unidades de trabajo*/);
/*Faltan las instrucciones de IO*/
///////////////////////////////////
extern t_log* logger;
extern t_config_cpu * config;
extern t_regist_cpu* registros_cpu;
extern int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
extern int cod_op_kernel_dispatch;
#endif //cpu_h
