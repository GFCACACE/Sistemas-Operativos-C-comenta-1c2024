#ifndef stdin_h
#define stdin_h
#include "entradasalida.h"


void io_stdin_read(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio);
char* leer_texto_consola();

#endif