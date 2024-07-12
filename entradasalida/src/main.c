#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "entradasalida.h"

int main(int argc, char* argv[]) {   
   
    // Iniciamos mediante una funcion las config, el logger y las conexiones
    // Asignamos un flag que nos devolverá si se pudo iniciar correctamente el módulo o no.
    bool flag_iniciar_io = iniciar_io(argv[1],argv[2]);
    if(flag_iniciar_io == false){
        //En caso de que no se haya inicializado correctamente, finalizamos memoria y salimos del programa
        finalizar_io();
        return EXIT_FAILURE;
    }
    recibir_io();

    //Si llega hasta acá es porque ya se ejecutó todo lo necesario
    //Finalizamos memoria y salimos del programa
    finalizar_io();
  
    // iniciar_log_config(argv[1],argv[2]);
    // iniciar_archivos_dialfs();
    // io_fs_create("fede_crack.txt");
    // io_fs_create("fede_crack_II.txt");
    return EXIT_SUCCESS;
}


// int main() {
//     // Write C code here
//     char* texto = malloc(20);
//     char* texto_a_enviar = malloc(4);
//     //t_pid_valor pid_valor = direcciones_proceso.pid_size_total;
//     texto = leer_texto_consola();
//     while(strlen(texto)  < 4){
//         //loguear_error("La cadena escrita es de %d bytes, debe ser de %d bytes.",(int)strlen(texto),4);
//         printf("La cadena escrita es de %d bytes, debe ser de %d bytes.\n",(int)strlen(texto),4);
//         texto = leer_texto_consola();
//     }
//     //loguear("TEXTO INGRESADO: %s",texto);}
//     printf("TEXTO INGRESADO: %s \n",texto);
//     memcpy(texto_a_enviar,texto,4);
//     //loguear("TEXTO A ENVIAR: %s",texto_a_enviar);
//     printf("TEXTO A ENVIAR: %s \n",texto_a_enviar);
//     // int valor_falopa;
//     // valor_falopa =(int) sizeof(texto_a_enviar);
//     printf("Cantidad de bytes: %d \n",strlen(texto_a_enviar));
//     free(texto);
//     free(texto_a_enviar);
    
//     return 0;
// }