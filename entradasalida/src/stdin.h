#ifndef stdin_h
#define stdin_h
#include "entradasalida.h"
#include <utils/utils-interfaz.h>


void io_stdin_read(t_direcciones_proceso*, int conexion);
char* leer_texto_consola();

#endif