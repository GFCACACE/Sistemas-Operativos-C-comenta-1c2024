#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "entradasalida.h"

int main(int argc, char* argv[]) {
    //decir_hola("una Interfaz de Entrada/Salida");
    //Iniciamos mediante una funcion las config, el logger y las conexiones
    //Asignamos un flag que nos devolverá si se pudo iniciar correctamente el módulo o no.
    bool flag_iniciar_io = iniciar_io(argv[1]);
    if(flag_iniciar_io == false){
        //En caso de que no se haya inicializado correctamente, finalizamos memoria y salimos del programa
        finalizar_io();
        return EXIT_FAILURE;
    }
 
    //Si llega hasta acá es porque ya se ejecutó todo lo necesario
    //Finalizamos memoria y salimos del programa
    finalizar_io();
    return EXIT_SUCCESS;
}
