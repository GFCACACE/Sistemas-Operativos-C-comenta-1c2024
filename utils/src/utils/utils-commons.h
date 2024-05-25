#ifndef utils_commons_h
#define utils_commons_h

#include <stdio.h>
#include <stdbool.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/utils-config.h>
#include<string.h>
#include <ctype.h>
#include <utils/hello.h>
#define EXIT_PROGRAM "EXIT"
typedef enum
{
	//Genérico
	MENSAJE,
	PAQUETE,
	FIN_PROGRAMA,
	//CPU - Memoria
	PROXIMA_INSTRUCCION,
	//Kernel - CPU
	EJECUTAR_PROCESO,
	INTERRUMPIR_CPU,
	EJECUTAR_CPU,
	FIN_QUANTUM,
	//CPU - Kernel
	CPU_INTERRUPT,
	CPU_EXIT,
	//Kernel - Memoria
	CREACION_PROCESO,
	ELIMINACION_PROCESO,
	CREACION_PROCESO_FALLIDO,
	ELIMINACION_PROCESO_FALLIDO,
	//IO
	IO_GEN_SLEEP
}op_code;

// BRAND NEW
// IDEA: la CPU le envia a kernel un struct con la info que necesita 
// para gestionar las instancias de IO generica.
//
typedef struct{
	int process_id;
	op_code operacion;
	char* tipo_interfaz;
	char* nombre_interfaz;
	int unidades_de_trabajo;
}t_peticion_generica;
// BRAND NEW



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
	
} t_registros_cpu;


typedef struct
{
	uint32_t PID;
	uint32_t program_counter;
	uint8_t prioridad;
	uint32_t quantum;
	t_registros_cpu* registros_cpu;
	t_list* archivos_abiertos;
	char* path;
} t_pcb;

typedef struct t_validacion
{
	bool resultado;
	char* descripcion;
}t_validacion;

t_validacion* validacion_new();
t_pcb* pcb_create(char*);
t_pcb* pcb_create_copy(char*);
t_pcb* pcb_create_quantum(char* path_programa,int quantum);
void pcb_destroy(t_pcb*);
bool is_numeric(const char*);
void loguear_registros(t_registros_cpu* registros);
void loguear_pcb(t_pcb*);
char* path_resolve(char*, char*);
char * uint_a_string(uint);
t_list* get_instrucciones(char* path_inicial,char *nombre_archivo);
char* get_linea_archivo(char* directorio,char* nombre_archivo,int posicion);
bool es_exit(void *comando);
t_registros_cpu* inicializar_registros (t_registros_cpu*);

#endif /* utils_commons_h */