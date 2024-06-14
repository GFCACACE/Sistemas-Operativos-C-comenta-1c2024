#include "stdout.h"

void io_stdout_write(uint32_t direccion_fisica, uint32_t tamanio){
    char mensaje[60];
    //mensaje = pedir_a_memoria(direccion_fisica,tamanio, PEDIDO_DE_ESCRITURA , conexion_memoria);
    // TODO
    printf("Mensaje traido de memoria: \n %s",mensaje);
    //free(mensaje);
}