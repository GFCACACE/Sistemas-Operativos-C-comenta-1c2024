#ifndef utils_interfaz_h
#define utils_interfaz_h

#include<commons/config.h>
#include<commons/log.h>
#include<commons/string.h>
#include "utils/utils-logger.h"
#include<math.h>
#include<commons/collections/dictionary.h>
#include "utils/utils-server.h"
#include "utils/utils-client.h"
#include "utils/utils-config.h"
#include "utils/utils-commons.h"
#include <stdint.h>
#include "utils/utils-serializacion.h"

void escribir_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion);
t_buffer* leer_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros,int conexion);
void escribir_memoria_completa_io(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion_a_memoria,op_code op_code);
t_buffer* leer_memoria_completa_io(t_direcciones_proceso* direcciones_fisicas_registros,int conexion, op_code op_code);
#endif /* utils_interfaz_h */