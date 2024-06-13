#include "stdout.h"

void io_stdout_write(/*registro direccion*/){

    
    //enviar_a_memoria(registro direccion, PEDIDO_DE_LECTURA , conexion_memoria);
    enviar_texto("Listo", TERMINO_STDOUT, conexion_kernel);
}