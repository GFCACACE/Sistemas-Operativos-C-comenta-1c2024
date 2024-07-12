#include "stdin.h"


char* leer_texto_consola(){
    return readline(">");
}


void io_stdin_read(t_direcciones_proceso direcciones_proceso, int conexion){
    //leer_texto_consola();
    
    // char* entrada = malloc(tamanio-1); // compensa el byte que se usa en strlen +1
    // entrada = leer_texto_consola();
    // t_acceso_espacio_usuario* acceso_espacio_usuario =  acceso_espacio_usuario_create(pid, direccion_fisica,tamanio,entrada);
    // enviar_acceso_espacio_usuario(acceso_espacio_usuario,PEDIDO_STDIN,conexion_memoria);
    // free(entrada);
    // int operacion_ok = recibir_operacion(conexion_memoria);
	// if(operacion_ok== RESPUESTA_STDIN){
    //     char *valor_memoria= malloc(50);
    //     valor_memoria=recibir_mensaje(conexion_memoria);
    //     printf("Respuesta de escritura en memoria: %s", valor_memoria); //VER SI DEBE MEJORARSE
    //     //Hacer el print

    //     free(valor_memoria);
   
    // }
}
