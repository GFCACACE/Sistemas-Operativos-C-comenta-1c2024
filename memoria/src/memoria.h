#ifndef kernel_h
#define kernel_h
#include<commons/config.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/queue.h>
#include <utils/utils-client.h>
#include <utils/utils-server.h>
#include <utils/utils-config.h>
#include <readline/readline.h>


typedef struct
{
	char* PUERTO_ESCUCHA;
	int TAM_MEMORIA;
	int TAM_PAGINA;
	char* PATH_INSTRUCCIONES;
	int RETARDO_RESPUESTA;
	
	t_config* config;
} t_config_memoria;


typedef enum
{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	ACCESO_TABLA_PAGINAS,
	AJUSTAR_TAMANIO_PROCESO,
	ACCESO_ESPACIO_USUARIO
}op_code_memoria;

t_config_memoria* iniciar_config_memoria(char*);
void config_memoria_destroy(t_config_memoria*);
void loguear_config_memoria();
extern t_log* logger_memoria;
extern t_config_memoria* config_memoria;



#endif /* kernel.h*/