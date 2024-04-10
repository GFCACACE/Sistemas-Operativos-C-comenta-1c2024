#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <unistd.h>
#include "cpu.h"




int main(int argc, char* argv[]) {
    char* modulo = "cpu";
    decir_hola(modulo);

    logger = iniciar_logger(modulo);   

    config = config_create_cpu(argv[1]);
    
    int dispatch = iniciar_servidor(config->PUERTO_ESCUCHA_DISPATCH);
    loguear("Servidor iniciado, dispatch");
    
    int conexion_memoria = intentar_conexion(config->IP_MEMORIA,config->PUERTO_MEMORIA,"memoria");

    int kernel_dispatch = esperar_cliente(dispatch,logger);

    int interrupt= iniciar_servidor(config->PUERTO_ESCUCHA_INTERRUPT);
    loguear("servidor iniciado, interrupt");

    
    int kernel_interrupt = esperar_cliente(interrupt,logger);

    int cod_op = recibir_operacion(kernel_dispatch);
    printf("cod_op: %d",cod_op);

    recibir_mensaje(kernel_dispatch);

    enviar_mensaje("Respuesta desde CPU",kernel_dispatch);

    enviar_mensaje("Interrupción desde CPU",kernel_interrupt);

    finalizar_cpu(dispatch,interrupt,config);

    return 0;







}


