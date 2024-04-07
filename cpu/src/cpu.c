#include "cpu.h"

// t_log* logger;
t_config_cpu* config_create_cpu(){
	t_config* config = iniciar_config("cpu");
	t_config_cpu* config_cpu = malloc(sizeof(t_config_cpu));
	 config_cpu->IP_MEMORIA = config_get_string_value(config,"IP_MEMORIA");
	 config_cpu->PUERTO_MEMORIA = config_get_string_value(config,"PUERTO_MEMORIA");
	 config_cpu->PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	 config_cpu->PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
	 config_cpu->config = config;

	 return config_cpu;
}

void config_destroy_cpu(t_config_cpu* config){

	 config_destroy(config->config);
	 free(config);
}

void finalizar_cpu(int socket_dispatch,int socket_interrupt,t_config_cpu* config){
	config_destroy_cpu(config);
	log_destroy(logger);
	close(socket_dispatch);
	close(socket_interrupt);

}

int intentar_conexion(char* ip, char*puerto,char* modulo_servidor){
	 log_info(logger,"Intentando conectar con %s...",modulo_servidor);
     int conexion_memoria = crear_conexion(ip,puerto);
         if (conexion_memoria<0){
            log_info(logger,"Debe estar levantada la memoria, respuesta obtenida: %d", conexion_memoria);
        }else{
            log_info(logger,"Conexión exitosa con memoria");
        }
	return conexion_memoria;
    }
    