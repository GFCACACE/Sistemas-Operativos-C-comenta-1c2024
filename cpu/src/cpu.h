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
	uint32_t PC;
	uint8_t AX;
	uint8_t BX;
	uint8_t CX;
	uint8_t DX;
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
	uint32_t SI;
	uint32_t DI;
} t_regist_cpu;


bool iniciar_cpu(char*);
t_regist_cpu* iniciar_registros_cpu();
t_config_cpu* iniciar_config_cpu(char*);
void config_destroy_cpu(t_config_cpu*);
void finalizar_cpu();
void loguear_config();
void ejecutar_programa(t_pcb* pcb);
int intentar_conexion(char*,int,char*);
//Instrucciones//////////////////////
bool set(char*,uint32_t);
bool mov_in(char*,char*);
bool mov_out(char*,char*);
bool sum(char*,char*);
bool sub(char*,char*);
bool jnz(char*,char*);
bool resize(uint32_t);
bool copy_string(uint32_t);
bool wait(/*recurso*/);
bool signal(/*recurso*/);
bool exit();
/*Faltan las instrucciones de IO*/
///////////////////////////////////
extern t_log* logger;
extern t_config_cpu * config;
extern t_regist_cpu* registros_cpu;
extern int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
extern int cod_op_kernel_dispatch;
#endif //cpu_h
