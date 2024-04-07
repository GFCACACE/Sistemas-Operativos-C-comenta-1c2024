#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "memoria.h"

int main(int argc, char* argv[]) {
    char* modulo = "memoria";
    decir_hola(modulo);
    logger_memoria= iniciar_logger(modulo);
	config_memoria = iniciar_config_memoria(modulo);
	loguear_config_memoria();	    
    int memoria_fd = iniciar_servidor(config_memoria->PUERTO_ESCUCHA);
    log_info(logger_memoria,"El Servidor ha sido iniciado");
    log_info(logger_memoria,"Esperando conexiones...");

    int cpu_cl = esperar_cliente(memoria_fd,logger_memoria);
    int kernel_fd = esperar_cliente(memoria_fd,logger_memoria);
    
    
    


    

}
