#include "memoria.h"

t_log* logger_memoria;
t_config_memoria* config_memoria;

t_config_memoria* iniciar_config_memoria(char* config_path){

	t_config* _config = config_create(config_path);
	t_config_memoria* config_memoria = malloc(sizeof(t_config_memoria));	

	config_memoria->PUERTO_ESCUCHA = config_get_int_value(_config,"PUERTO_ESCUCHA");
	config_memoria->TAM_MEMORIA = config_get_int_value(_config,"TAM_MEMORIA");
	config_memoria->TAM_PAGINA = config_get_int_value(_config,"TAM_PAGINA");
	config_memoria->PATH_INSTRUCCIONES = config_get_string_value(_config,"PATH_INSTRUCCIONES");
	config_memoria->RETARDO_RESPUESTA = config_get_int_value(_config,"RETARDO_RESPUESTA");

	config_memoria->config=_config;

	return config_memoria;
}



void config_memoria_destroy(t_config_memoria* config){

	free(config->PATH_INSTRUCCIONES);
	config_destroy(config->config);
	free(config);
}

void loguear_config_memoria(){

	log_info(logger_memoria,"PUERTO_ESCUCHA: %d",config_memoria->PUERTO_ESCUCHA);
	log_info(logger_memoria,"TAM_MEMORIA: %d",config_memoria->TAM_MEMORIA);
	log_info(logger_memoria,"TAM_PAGINA: %d",config_memoria->TAM_PAGINA);

	log_info(logger_memoria,"PATH_INSTRUCCIONES: %s",config_memoria->PATH_INSTRUCCIONES);
	log_info(logger_memoria,"RETARDO_RESPUESTA: %d",config_memoria->RETARDO_RESPUESTA);
}

