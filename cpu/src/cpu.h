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

bool iniciar_cpu(char*);
t_config_cpu* iniciar_config_cpu(char*);
void config_destroy_cpu(t_config_cpu*);
void finalizar_cpu();
void loguear_config();
void ejecutar_programa(t_pcb* pcb);
int intentar_conexion(char*,int,char*);
extern t_log* logger;
extern t_config_cpu * config;
extern int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
extern int cod_op_kernel_dispatch;
#endif //cpu_h
