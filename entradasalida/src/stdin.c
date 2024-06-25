#include "stdin.h"


char* leer_texto_consola(){
    return readline(">");
}

t_paquete* armar_paquete_in(uint32_t direccion_fisica, char* entrada, uint32_t tamanio){
    t_paquete* paquete = crear_paquete(PEDIDO_STDIN);
    agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
    agregar_a_paquete(paquete, &entrada, strlen(entrada) + 1);
    agregar_a_paquete(paquete, &tamanio, sizeof(uint32_t));

    return paquete;
}


void io_stdin_read(uint32_t direccion_fisica, uint32_t tamanio){
    //leer_texto_consola();
    
    char* entrada = malloc(tamanio-1); // compensa el byte que se usa en strlen +1
    entrada = leer_texto_consola();
    // HABRÍA QUE ENCONTRAR UNA FORMA MAS ELEGANTE PARA LEER DE TECLADO
    t_paquete* paquete = armar_paquete_in(direccion_fisica, entrada, tamanio);
    enviar_paquete(paquete, conexion_memoria);
    free(entrada);
    paquete_destroy(paquete);
    //enviar_a_memoria(direccion_fisica, entrada, PEDIDO_STDIN , conexion_memoria);
    // TODO
}

