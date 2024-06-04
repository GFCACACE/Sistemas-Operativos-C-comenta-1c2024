#include "stdin.h"


char* leer_texto_consola(){return readline(">");}

void io_stdin_read(/*direccion pedida*/){
    leer_texto_consola();
    char* entrada = leer_texto_consola();
    //enviar_a_memoria(entrada, direccion pedida);
    enviar_texto("Listo", TERMINO_STDIN, conexion_kernel);
}

