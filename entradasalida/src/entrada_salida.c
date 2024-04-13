#include <stdlib.h>
#include <stdio.h>
#include "entrada_salida.h"

int entrada_salida_escucha,conexion_memoria, conexion_kernel;
t_config_entrada_salida* config_entrada_salida;

t_config_entrada_salida* iniciar_config_entrada_salida(char* config_path){

	t_config* _config = config_create(config_path);
	if(_config ==NULL)
		return NULL;
	t_config_entrada_salida* config_entrada_salida = malloc(sizeof(t_config_entrada_salida));	

	config_entrada_salida->TIPO_INTERFAZ = config_get_string_value(_config,"TIPO_INTERFAZ");
	config_entrada_salida->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
	config_entrada_salida->IP_KERNEL = config_get_string_value(_config,"IP_KERNEL");
	config_entrada_salida->PUERTO_KERNEL = config_get_int_value(_config,"PUERTO_KERNEL");
	config_entrada_salida->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
    config_entrada_salida->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
	config_entrada_salida->PATH_BASE_DIALFS = config_get_string_value(_config,"PATH_BASE_DIALFS");
	config_entrada_salida->BLOCK_SIZE = config_get_int_value(_config,"BLOCK_SIZE");
	config_entrada_salida->BLOCK_COUNT = config_get_int_value(_config,"BLOCK_COUNT");

	config_entrada_salida->config=_config;

	return config_entrada_salida;
}

void finalizar_entrada_salida(){
	
	if (conexion_memoria != -1) liberar_conexion(conexion_memoria);
	if (conexion_kernel != -1) liberar_conexion(conexion_kernel);
	if(config_entrada_salida!=NULL) config_destroy_entrada_salida(config_entrada_salida);
}


bool iniciar_entrada_salida(char* path_config){
	decir_hola(MODULO);
    logger = iniciar_logger(MODULO);

	if(logger == NULL) printf("EL LOGGER NO PUDO SER INICIADO.\n");

	config_entrada_salida = iniciar_config_entrada_salida(path_config);

	if(config_entrada_salida == NULL) {
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}
	loguear_config_entrada_salida();	    
    conexion_memoria = crear_conexion(config_entrada_salida->IP_MEMORIA,config_entrada_salida->PUERTO_MEMORIA);
	if(conexion_memoria ==-1){
		loguear_error("No se pudo conectar con memoria");
		return false;
	} 
    /*conexion_kernel = crear_conexion(config_entrada_salida->IP_KERNEL, config_entrada_salida->PUERTO_KERNEL);
	if(conexion_kernel ==-1){
		loguear_error("No se pudo conectar con el kernel");
		return false;
	}*/
	return true;
}


void loguear_config_entrada_salida(){

	loguear("TIPO_INTERFAZ: %s",config_entrada_salida->TIPO_INTERFAZ);
	loguear("TIEMPO_UNIDAD_TRABAJO: %d",config_entrada_salida->TIEMPO_UNIDAD_TRABAJO);
	loguear("IP_KERNEL: %s",config_entrada_salida->IP_KERNEL);
	loguear("PUERTO_KERNEL: %d",config_entrada_salida->PUERTO_KERNEL);
	loguear("IP_MEMORIA: %s",config_entrada_salida->IP_MEMORIA);
    loguear("PUERTO_MEMORIA: %d",config_entrada_salida->PUERTO_MEMORIA);
	loguear("PATH_BASE_DIALFS: %s",config_entrada_salida->PATH_BASE_DIALFS);
	loguear("BLOCK_SIZE: %d",config_entrada_salida->BLOCK_SIZE);
	loguear("BLOCK_COUNT: %d",config_entrada_salida->BLOCK_COUNT);
}

void config_destroy_entrada_salida(){
	//Para asegurarnos que liberamos toda la memoria de una estructura 
	//primero borramos lo de adentro y luego lo de afuera

	config_destroy(config_entrada_salida->config);//Primero borramos la config que está adentro
	free(config_entrada_salida); // Finalmente la estructura que lo contenía
}
