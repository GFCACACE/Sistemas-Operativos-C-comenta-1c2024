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
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA_DISPATCH;
	char* PUERTO_ESCUCHA_INTERRUPT;
	t_config* config;
} t_config_cpu;

typedef struct {
	int dispatch;
	int interrupt;
	int memoria;
} t_connections_cpu;

t_config_cpu* config_create_cpu(void);
void config_destroy_cpu(t_config_cpu*);
void finalizar_cpu(int,int,t_config_cpu*);

extern t_log* logger;

#endif //cpu_h
