#ifndef utils_interfaz_h
#define utils_interfaz_h

#include<commons/config.h>
#include<commons/log.h>
#include<commons/string.h>
#include<math.h>
#include<commons/collections/dictionary.h>
#include "utils/utils-server.h"
#include "utils/utils-client.h"
#include "utils/utils-config.h"
#include "utils/utils-commons.h"
#include <stdint.h>

void escribir_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion);
t_buffer* leer_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros,int conexion);
#endif /* utils_interfaz_h */