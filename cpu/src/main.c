#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <unistd.h>
#include "cpu.h"




int main(int argc, char** argv) {
    
    bool flag_iniciar_cpu = iniciar_cpu(argv[1]);
    if(flag_iniciar_cpu == false){ 
        finalizar_cpu();
        return EXIT_FAILURE;
        }

    cod_op_kernel_dispatch = recibir_operacion(kernel_dispatch);
    printf("cod_op: %d",cod_op_kernel_dispatch);

    char* mje = recibir_mensaje(kernel_dispatch);
    free(mje);

    enviar_mensaje("Respuesta desde CPU",kernel_dispatch);

    enviar_mensaje("Interrupción desde CPU",kernel_interrupt);


      t_pcb* pcb_prueba = pcb_create("programa3.txt");
      ejecutar_programa(pcb_prueba);  
      pcb_destroy(pcb_prueba);

    finalizar_cpu();

    return EXIT_SUCCESS;







}


