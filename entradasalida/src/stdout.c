#include "stdout.h"


void io_stdout_write(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio){
    char mensaje[tamanio];
    t_paquete* paquete_a_recibir;
    //mensaje = pedir_a_memoria(direccion_fisica,tamanio, PEDIDO_STDOUT , conexion_memoria);
    // TODO
    t_acceso_espacio_usuario* acceso_espacio_usuario =  acceso_espacio_usuario_create(pid, direccion_fisica,tamanio,NULL);
    enviar_acceso_espacio_usuario(acceso_espacio_usuario,PEDIDO_STDOUT,conexion_memoria);
    // deserializar paquete y asignar lo almacenado en memoria a mensaje

    printf("Mensaje traido de memoria: \n %s",mensaje);

}