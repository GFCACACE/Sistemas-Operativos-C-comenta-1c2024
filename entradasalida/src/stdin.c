#include "stdin.h"


char* leer_texto_consola(){
    return readline(">");
}


void io_stdin_read(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio){
    //leer_texto_consola();
    
    char* entrada = malloc(tamanio-1); // compensa el byte que se usa en strlen +1
    entrada = leer_texto_consola();
    t_acceso_espacio_usuario* acceso_espacio_usuario =  acceso_espacio_usuario_create(pid, direccion_fisica,tamanio,entrada);
    _enviar_acceso_espacio_usuario(acceso_espacio_usuario,PEDIDO_STDIN,conexion_memoria);

    //if(operacion == ok)
    // avisar a kernel que la operación se ha completado
   

}
