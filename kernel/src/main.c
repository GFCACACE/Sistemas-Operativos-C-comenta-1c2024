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

    finalizar_kernel();

    return EXIT_SUCCESS;
}
