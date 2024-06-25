#include "stdout.h"


t_paquete* armar_paquete_out(uint32_t direccion_fisica, uint32_t tamanio){
    t_paquete* paquete = crear_paquete(PEDIDO_STDOUT);
    agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
    agregar_a_paquete(paquete, &tamanio, sizeof(uint32_t));

    return paquete;
}
void io_stdout_write(uint32_t direccion_fisica, uint32_t tamanio){
    char mensaje[tamanio];
    t_paquete* paquete_a_enviar, *paquete_a_recibir;
    //mensaje = pedir_a_memoria(direccion_fisica,tamanio, PEDIDO_STDOUT , conexion_memoria);
    // TODO
    paquete_a_enviar = armar_paquete_out(direccion_fisica, tamanio);
    enviar_paquete(paquete_a_enviar, conexion_memoria);
    paquete_a_recibir = recibir_paquete(conexion_memoria);
    
    // deserializar paquete y asignar lo almacenado en memoria a mensaje

    printf("Mensaje traido de memoria: \n %s",mensaje);
    //free(mensaje);
}