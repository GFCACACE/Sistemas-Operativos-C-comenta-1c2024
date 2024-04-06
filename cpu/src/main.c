#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "cpu.h"

int main(int argc, char* argv[]) {
    char* modulo = "cpu";
    decir_hola(modulo);

    logger = iniciar_logger(modulo);   

    t_config_cpu* config = config_create_cpu();
    char* puerto =config->PUERTO_ESCUCHA_DISPATCH;
    
    int dispatch = iniciar_servidor(puerto);
    loguear("servidor iniciado, dispatch");

    int kernel_dispatch = esperar_cliente(dispatch);

    int interrupt= iniciar_servidor(puerto);
    loguear("servidor iniciado, interrupt");

    int kernel_interrupt = esperar_cliente(interrupt);

    int cod_op = recibir_operacion(kernel_dispatch);
    printf("cod_op: %d",cod_op);

    recibir_mensaje(kernel_dispatch);

    enviar_mensaje("Respuesta desde CPU",kernel_dispatch);

     enviar_mensaje("Interrupción desde CPU",kernel_interrupt);

    finalizar_cpu(dispatch,interrupt,config);

    return 0;
}
