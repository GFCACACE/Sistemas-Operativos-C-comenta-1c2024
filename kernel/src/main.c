#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "kernel.h"

int main(int argc, char* argv[]) {
    char* modulo = "kernel";
    decir_hola(modulo);

    logger = iniciar_logger(modulo);
	config = iniciar_config_kernel(modulo);
	loguear_config();	    

    int cpu_dispatch = crear_conexion(config->IP_CPU, config->PUERTO_CPU_DISPATCH);

    if(cpu_dispatch<0){
        printf("Debe estar levantado el CPU Dispatch\n");
        return -1;
    }

    int cpu_interrupt = crear_conexion(config->IP_CPU, config->PUERTO_CPU_INTERRUPT);
       if(cpu_interrupt<0){
        printf("Debe estar levantado el CPU Interrupt\n");
        return -1;
    }


    enviar_mensaje("Hola desde kernel",cpu_dispatch);

    //Recibimos respuesta de CPU
    int cod_op_dispatch = recibir_operacion(cpu_dispatch);
    printf("cod_op: %d",cod_op_dispatch);
    recibir_mensaje(cpu_dispatch);

    //Recibimos interrupt de CPU
    int cod_op_interrupt = recibir_operacion(cpu_interrupt);
    printf("cod_op: %d",cod_op_interrupt);
    recibir_mensaje(cpu_interrupt);

	//consola();

    finalizar_kernel(config);

    return 0;
}
