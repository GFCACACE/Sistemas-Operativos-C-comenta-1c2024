#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "kernel.h"

int main(int argc, char** argv) {

    //Iniciamos mediante una funcion las config, el logger y las conexiones
    //Asignamos un flag que nos devolverá si se pudo iniciar el módulo o no.
    bool flag_modulo = iniciar_kernel(argv[1]);
    //En caso de que no se haya inicializado, finalizamos el programa
    if(flag_modulo == false){
        finalizar_kernel();
        return EXIT_FAILURE;
    } 

    enviar_mensaje("Hola desde kernel",cpu_dispatch);

    //Recibimos respuesta de Memoria
    cod_op_memoria = recibir_operacion(conexion_memoria);
    printf("cod_op: %d",cod_op_memoria);
    char* mje = recibir_mensaje(conexion_memoria);
    //Recibimos respuesta de CPU
    cod_op_dispatch = recibir_operacion(cpu_dispatch);
    printf("cod_op: %d",cod_op_dispatch);
    mje = recibir_mensaje(cpu_dispatch);

    //Recibimos interrupt de CPU
    cod_op_interrupt = recibir_operacion(cpu_interrupt);
    printf("cod_op: %d",cod_op_interrupt);
    mje = recibir_mensaje(cpu_interrupt);

    free(mje);

	//consola();

    finalizar_kernel();

    return EXIT_SUCCESS;
}
