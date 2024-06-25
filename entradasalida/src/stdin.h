#ifndef stdin_h
#define stdin_h
#include "entradasalida.h"


void io_stdin_read(uint32_t direccion_fisica, uint32_t tamanio);
t_paquete* armar_paquete_in(uint32_t direccion_fisica, char* entrada, uint32_t tamanio);
char* leer_texto_consola();

#endif