#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "memoria.h"

int main(int argc, char** argv) {

    //Iniciamos mediante una funcion las config, el logger y las conexiones
    //Asignamos un flag que nos devolverá si se pudo iniciar correctamente el módulo o no.
    bool flag_iniciar_memoria = iniciar_memoria(argv[1]);
    if(flag_iniciar_memoria == false){
        //En caso de que no se haya inicializado correctamente, finalizamos memoria y salimos del programa
        finalizar_memoria();
        return EXIT_FAILURE;
    }
    enviar_mensaje("Hola desde memoria",conexion_kernel);

    buscar_instrucciones();

   


/*
 
    t_pcb* pcb_prueba = pcb_create("programa1.txt");
    //cargar_programa_de(pcb_prueba,"programa1.txt");
    proxima_instruccion_de(pcb_prueba);

    pcb_prueba->program_counter++;
    proxima_instruccion_de(pcb_prueba);

    
    pcb_destroy(pcb_prueba);*/

    //Si llega hasta acá es porque ya se ejecutó todo lo necesario
    //Finalizamos memoria y salimos del programa
    finalizar_memoria();
    return EXIT_SUCCESS;

}
