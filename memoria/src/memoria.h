#ifndef memoria_h
#define memoria_h
#include <utils/hello.h>
#include<commons/config.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/queue.h>
#include <utils/utils-client.h>
#include <utils/utils-server.h>
#include <utils/utils-commons.h>
#include <utils/utils-config.h>
#include <readline/readline.h>
#define MODULO "memoria"

typedef struct
{
	int PUERTO_ESCUCHA;
	int TAM_MEMORIA;
	int TAM_PAGINA;
	char* PATH_INSTRUCCIONES;
	int RETARDO_RESPUESTA;
	
	t_config* config;
} t_config_memoria;


typedef struct
{
	t_list* instrucciones;
	t_pcb* pcb;
	t_list* tabla_paginas;
} t_proceso;


typedef enum
{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	//ACCESO_TABLA_PAGINAS,
	AJUSTAR_TAMANIO_PROCESO
	
}op_code_memoria;

t_config_memoria* iniciar_config_memoria(char*);
void config_memoria_destroy();
void loguear_config_memoria();
bool iniciar_memoria(char*);
void finalizar_memoria();
extern t_log* logger;
extern t_config_memoria* config_memoria;
extern int memoria_escucha,conexion_cpu, conexion_kernel;
extern t_dictionary* procesos;
char* proxima_instruccion_de(t_pcb*);
void cargar_programa_de(t_pcb*,char*);
void enviar_proxima_instruccion (t_pcb* pcb);
int buscar_instrucciones();
int recibir_procesos();
void crear_frames_memoria_principal(int);
int asignar_frame();
void remover_proceso_del_frame(int);
void imprimir_uso_frames();
int convertir_bytes_a_paginas(int);
bool validar_ampliacion_proceso(int);
void ampliar_proceso(t_list* ,int );
void reducir_proceso(t_list* ,int);
t_pid_valor* recibir_pid_value(t_paquete*);
void liberar_frame(int nro_frame);
char* obtener_frame(t_list* tabla_de_paginas,int nro_pagina);


void efectuar_retardo();

#endif /* memoria.h*/