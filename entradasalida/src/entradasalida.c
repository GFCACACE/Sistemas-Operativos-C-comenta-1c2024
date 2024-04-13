#include "entradasalida.h"


int conexion_memoria, conexion_kernel;
int cod_op_kernel,cod_op_memoria;
t_config_io* config;

t_config_io* iniciar_config_io(char* path_config){
	t_config* _config = config_create(path_config);
	if(_config ==NULL)
		return NULL;
	t_config_io* config_io = malloc(sizeof(t_config_io));	

	config_io->TIPO_INTERFAZ = config_get_string_value(_config,"TIPO_INTERFAZ");
	config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
	config_io->IP_KERNEL = config_get_string_value(_config,"IP_KERNEL");
	config_io->PUERTO_KERNEL = config_get_int_value(_config,"PUERTO_KERNEL");
	config_io->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
	config_io->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
	config_io->PATH_BASE_DIALFS = config_get_string_value(_config,"PATH_BASE_DIALFS");
	config_io->BLOCK_SIZE = config_get_int_value(_config,"BLOCK_SIZE");
	config_io->BLOCK_COUNT = config_get_int_value(_config,"BLOCK_COUNT");
	config_io->config = _config;

	return config_io;
}

bool iniciar_io(char* path_config){
	decir_hola(MODULO);
    logger = iniciar_logger(MODULO);
	if(logger == NULL) printf("EL LOGGER NO PUDO SER INICIADO.\n");
	config = iniciar_config_io(path_config);
	if(config == NULL) {
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}
	loguear_config();	    
    conexion_memoria = crear_conexion(config->IP_MEMORIA,config->PUERTO_MEMORIA);
	if(conexion_memoria ==-1){
		
		loguear_error("No se pudo conectar memoria");
		return false;
	} 
    
    conexion_kernel = crear_conexion(config->IP_KERNEL, config->PUERTO_KERNEL);
	if(conexion_kernel ==-1){
		
		loguear_error("No se pudo conectar kernel");
		return false;
	} 
	return true;
}

void config_io_destroy(t_config_io* config){

	config_destroy(config->config);
	free(config);
}

void loguear_config(){

	loguear("TIPO_INTERFAZ: %s",config->TIPO_INTERFAZ);
	loguear("TIEMPO_UNIDAD_TRABAJO: %d",config->TIEMPO_UNIDAD_TRABAJO);
	loguear("IP_KERNEL: %s",config->IP_KERNEL);
    loguear("PUERTO_KERNEL: %d",config->PUERTO_KERNEL);
	loguear("IP_MEMORIA: %s",config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d",config->PUERTO_MEMORIA);
	loguear("PATH_BASE_DIALFS: %s",config->PATH_BASE_DIALFS);
	loguear("BLOCK_SIZE: %d",config->BLOCK_SIZE);
    loguear("BLOCK_COUNT: %d",config->BLOCK_COUNT);
}

void finalizar_io(){
	if(config != NULL) config_io_destroy(config);
	if(logger != NULL) log_destroy(logger);
}