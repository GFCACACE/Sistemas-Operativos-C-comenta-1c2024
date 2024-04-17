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
typedef enum
{
	MENSAJE,
	PAQUETE,
	PROXIMA_INSTRUCCION,
	FIN_PROGRAMA
}op_code;

typedef uint32_t t_registro;
typedef struct{
 t_registro AX;
 t_registro BX;
 t_registro CX;
 t_registro DX;

}t_registros_cpu;


typedef struct
{
	uint32_t PID;
	uint32_t program_counter;
	uint8_t prioridad;
	t_registros_cpu* registros_cpu;
	t_list* archivos_abiertos;
	char* path;
} t_pcb;

t_pcb* pcb_create(char*);
void pcb_destroy(t_pcb*);
bool is_numeric(const char*);
void loguear_pcb(t_pcb*);
void path_resolve(char*, const char*, const char*);
char * uint_a_string(uint);
t_list* get_instrucciones(char* path_inicial,char *nombre_archivo);

#endif /* utils_commons_h */