#include "cpu.h"

// t_log* logger;

t_config_cpu * config;
t_config_cpu* config_create_cpu(char* path){
	t_config* _config = iniciar_config(path);
	t_config_cpu* config_cpu = malloc(sizeof(t_config_cpu));
	 config_cpu->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
	 config_cpu->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
	 config_cpu->PUERTO_ESCUCHA_DISPATCH = config_get_int_value(_config,"PUERTO_ESCUCHA_DISPATCH");
	 config_cpu->PUERTO_ESCUCHA_INTERRUPT = config_get_int_value(_config,"PUERTO_ESCUCHA_INTERRUPT");
	 config_cpu->config = _config;

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

int intentar_conexion(char* ip, int puerto,char* modulo_servidor){
	 log_info(logger,"Intentando conectar con %s...",modulo_servidor);
     int conexion_memoria = crear_conexion(ip,puerto);
         if (conexion_memoria<0){
            log_info(logger,"Debe estar levantada la memoria, respuesta obtenida: %d", conexion_memoria);
        }else{
            log_info(logger,"Conexión exitosa con memoria");
        }
	return conexion_memoria;
    }

void loguear_config(){
	loguear("IP_MEMORIA: %s",config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d",config->PUERTO_MEMORIA);
	loguear("PUERTO_ESCUCHA_DISPATCH: %d",config->PUERTO_ESCUCHA_DISPATCH);
	loguear("PUERTO_ESCUCHA_INTERRUPT: %d",config->PUERTO_ESCUCHA_INTERRUPT);
}