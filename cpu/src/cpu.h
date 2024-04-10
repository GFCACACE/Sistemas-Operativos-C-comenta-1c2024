#ifndef cpu_h
#define cpu_h

#include<commons/config.h>
#include<commons/log.h>
#include "utils/utils-server.h"
#include "utils/utils-client.h"
#include "utils/utils-config.h"

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

t_config_cpu* config_create_cpu(char*);
void config_destroy_cpu(t_config_cpu*);
void finalizar_cpu(int,int,t_config_cpu*);
void loguear_config();
int intentar_conexion(char*,int,char*);
extern t_log* logger;
extern t_config_cpu * config;
#endif //cpu_h
