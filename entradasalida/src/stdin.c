#include "stdin.h"


char* leer_texto_consola(){return readline(">");}

void io_stdin_read(uint32_t direccion_fisica, uint32_t tamanio){
    leer_texto_consola();
    char* entrada = malloc(tamanio);
    entrada = leer_texto_consola();
    //enviar_a_memoria(direccion_fisica, entrada, PEDIDO_DE_ESCRITURA , conexion_memoria);
    // TODO
}

