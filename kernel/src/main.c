#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "kernel.h"
#include "test.h"

int main_kernel(int argc, char** argv) {

    //Iniciamos mediante una funcion las config, el logger y las conexiones
    //Asignamos un flag que nos devolverá si se pudo iniciar el módulo o no.
    bool modulo_iniciado = iniciar_kernel(argv[1]);
   // consola();
    //En caso de que no se haya inicializado, finalizamos el programa
    if(!modulo_iniciado){
        finalizar_kernel();
        return EXIT_FAILURE;
    } 
    /*
    bool iniciar_planifi = iniciar_planificadores();
    if(!iniciar_planifi){
        finalizar_kernel();
        return EXIT_FAILURE;
    }
    else{
        loguear("iniciar planificador se abrio correctamente.");
    }
*/
    

    
    // enviar_mensaje("Hola desde kernel",cpu_dispatch);

    // //Recibimos respuesta de Memoria
    // cod_op_memoria = recibir_operacion(conexion_memoria);
    // printf("cod_op: %d",cod_op_memoria);
    // char* mje = recibir_mensaje(conexion_memoria);
    // free(mje);
    // //Recibimos respuesta de CPU
    // cod_op_dispatch = recibir_operacion(cpu_dispatch);
    // printf("cod_op: %d",cod_op_dispatch);
    // mje = recibir_mensaje(cpu_dispatch);
    //  free(mje);

    // //Recibimos interrupt de CPU
    // cod_op_interrupt = recibir_operacion(cpu_interrupt);
    // printf("cod_op: %d",cod_op_interrupt);
    // mje = recibir_mensaje(cpu_interrupt);
    // free(mje);
    
	iniciar_consola();

    finalizar_kernel();

    return EXIT_SUCCESS;
}





// TESTS
int main(int argc, char** argv) {
        if(argc > 1 && strcmp(argv[1],"-test")==0)
        run_tests();
    else 
        main_kernel(argc,argv);
}

