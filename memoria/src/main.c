#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "memoria.h"

int main(int argc, char** argv) {

    
    bool flag_iniciar_memoria = iniciar_memoria(argv[1]);
    if(flag_iniciar_memoria == false){
        finalizar_memoria();
        return EXIT_FAILURE;
    }
    
    finalizar_memoria();
    return EXIT_SUCCESS;

}
