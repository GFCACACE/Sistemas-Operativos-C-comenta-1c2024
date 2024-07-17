#include "stdout.h"


void io_stdout_write(t_direcciones_proceso* direcciones_proceso, int conexion){
    t_buffer* buffer_a_leer;
    uint32_t tamanio_en_bytes = direcciones_proceso->pid_size_total.valor;
    char* cadena = malloc(sizeof(tamanio_en_bytes)+1);
    char* cadena2 = malloc(sizeof(tamanio_en_bytes)+1);
    buffer_a_leer = leer_memoria_completa_io(direcciones_proceso,conexion);
    cadena = (char*)buffer_a_leer->stream;
    loguear("Resultado de la lectura 1: %s", cadena);
    memcpy(cadena2,buffer_a_leer->stream,sizeof(tamanio_en_bytes)+1);
    loguear("Resultado de la lectura con memcpy: %s", cadena);
    free(cadena);
    free(cadena2);
}