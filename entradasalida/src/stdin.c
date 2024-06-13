#include "stdin.h"


char* leer_texto_consola(){return readline(">");}

void io_stdin_read(/*registro direccion*/){
    leer_texto_consola();
    char* entrada = leer_texto_consola();
    // validación cantidad de caracteres
    //enviar_a_memoria(entrada almacenada en registro_direccion, PEDIDO_DE_ESCRITURA , conexion_memoria);
    enviar_texto("Listo", TERMINO_STDIN, conexion_kernel);
}

