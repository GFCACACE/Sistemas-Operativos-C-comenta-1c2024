#include "stdout.h"


void io_stdout_write(t_direcciones_proceso* direcciones_proceso, int conexion){
    t_buffer* buffer_a_leer;
    uint32_t tamanio_en_bytes = direcciones_proceso->pid_size_total->valor;
    char* cadena = malloc(sizeof(tamanio_en_bytes)+1);

    buffer_a_leer = leer_memoria_completa(direcciones_proceso,conexion);
    cadena = (char*)buffer_a_leer->stream;
    printf("Resultado de la lectura: %s", cadena);

    // memcpy(registro_reconstruido_puntero,buffer_lectura->stream,buffer_lectura->size);

    // char* valor_reconstruido = malloc(buffer_lectura->size +);

    // memcpy(valor_reconstruido,buffer_lectura->stream,buffer_lectura->size);
    // loguear("<%s>",valor_reconstruido);

}
//     char mensaje[tamanio];
//     t_paquete* paquete_a_recibir;
//     //mensaje = pedir_a_memoria(direccion_fisica,tamanio, PEDIDO_STDOUT , conexion_memoria);
//     // TODO
//     t_acceso_espacio_usuario* acceso_espacio_usuario =  acceso_espacio_usuario_create(pid, direccion_fisica,tamanio,NULL);
//     enviar_acceso_espacio_usuario(acceso_espacio_usuario,PEDIDO_STDOUT,conexion_memoria);
//     // deserializar paquete y asignar lo almacenado en memoria a mensaje
//     int operacion_ok = recibir_operacion(conexion_memoria);
// 	if(operacion_ok == RESPUESTA_STDOUT){
//         char* valor_a_imprimir = malloc(tamanio+1);
//         valor_a_imprimir=recibir_mensaje(conexion_memoria);
//         printf("Mensaje traido de memoria: \n %s",valor_a_imprimir);
//         free(valor_a_imprimir);
//     }
// }

