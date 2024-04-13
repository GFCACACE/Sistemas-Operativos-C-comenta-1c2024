#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "entrada_salida.h"

int main(int argc, char* argv[]) {
    decir_hola("una Interfaz de Entrada/Salida");

    bool flag_modulo = iniciar_entrada_salida(argv[1]);
    //En caso de que no se haya inicializado, finalizamos el programa
    if(flag_modulo == false){
        finalizar_entrada_salida();
        return EXIT_FAILURE;
    } 
    
    finalizar_entrada_salida();
    return 0;
}
