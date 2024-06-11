#ifndef utils_server_h
#define utils_server_h

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
#include<string.h>
#include<utils/utils-serializacion.h>
#include<utils/utils-commons.h>

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(int);
int esperar_cliente(int);
t_paquete* recibir_paquete(int);
char* recibir_mensaje(int);
int recibir_operacion(int);
t_pcb* recibir_pcb(t_paquete*);
t_pid_valor* recibir_pid_value(t_paquete* paquete);

#endif /* utils_server_h */
