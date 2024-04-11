#include "memoria.h"

int memoria_escucha,conexion_cpu, conexion_kernel;
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



void config_memoria_destroy(){

	config_destroy(config_memoria->config);
	free(config_memoria);
}

void loguear_config_memoria(){

	loguear("PUERTO_ESCUCHA: %d",config_memoria->PUERTO_ESCUCHA);
	loguear("TAM_MEMORIA: %d",config_memoria->TAM_MEMORIA);
	loguear("TAM_PAGINA: %d",config_memoria->TAM_PAGINA);

	loguear("PATH_INSTRUCCIONES: %s",config_memoria->PATH_INSTRUCCIONES);
	loguear("RETARDO_RESPUESTA: %d",config_memoria->RETARDO_RESPUESTA);
}

bool iniciar_memoria(char* path_config){
	
    decir_hola(MODULO);
    logger= iniciar_logger(MODULO);
	if(logger ==NULL ){
		printf("EL LOGGER NO PUDO SER INICIADO.\n");
		return false;
	} 
	config_memoria = iniciar_config_memoria(path_config);
	if(config_memoria == NULL){ 
		loguear_error("Fallo al iniciar las config");
		return false;
	}
	loguear_config_memoria();	    
    memoria_escucha= iniciar_servidor(config_memoria->PUERTO_ESCUCHA);
	if(memoria_escucha == -1){
		loguear_error("El servidor no pudo ser iniciado");
		return false;
	}
    loguear("El Servidor iniciado correctamente");

    conexion_cpu = esperar_cliente(memoria_escucha);
	if(conexion_cpu == -1){
		loguear_error("Falló la conexión con cpu");
		return false;
	}
    conexion_kernel = esperar_cliente(memoria_escucha);
	if(conexion_kernel == -1){
		loguear_error("Falló la conexión con kernel");
		return false;
	}
	return true;
}

void finalizar_memoria(){
	if(config_memoria != NULL) config_memoria_destroy();
	if(logger != NULL) log_destroy(logger);
}