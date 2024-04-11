#ifndef utils_config_h
#define utils_config_h

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
#include <pthread.h>
#include <ctype.h>
#include <semaphore.h>

extern t_log* logger;
char* construir_path(char*,char*);
char* construir_path_log(char*);
char* construir_path_config(char*);

// ----- logging -----
t_log* iniciar_logger(char*);
t_config* iniciar_config(char*);
void loguear(const char*, ...)__attribute__((format(printf, 1, 2)));
void loguear_error(const char*, ...)__attribute__((format(printf, 1, 2)));

bool is_numeric(const char*);
void path_resolve(char*, const char*, const char*);

#endif /* utils_config_h*/