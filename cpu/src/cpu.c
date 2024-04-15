#include "cpu.h"

int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
int cod_op_kernel_dispatch;
t_config_cpu * config;

t_config_cpu* iniciar_config_cpu(char* path_config){
	t_config* _config = config_create(path_config);
	if(_config ==NULL)
		return NULL;
	t_config_cpu* config_cpu = malloc(sizeof(t_config_cpu));
	 config_cpu->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
	 config_cpu->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
	 config_cpu->PUERTO_ESCUCHA_DISPATCH = config_get_int_value(_config,"PUERTO_ESCUCHA_DISPATCH");
	 config_cpu->PUERTO_ESCUCHA_INTERRUPT = config_get_int_value(_config,"PUERTO_ESCUCHA_INTERRUPT");
	 config_cpu->config = _config;

	 return config_cpu;
}

bool iniciar_cpu(char* path_config){
	decir_hola(MODULO);
    logger = iniciar_logger(MODULO);
	if(logger == NULL){
		printf("EL LOGGER NO PUDO SER INICIADO.\n");
		return false;
	} 
	config = iniciar_config_cpu(path_config);
	if(config == NULL){ 
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}
	loguear_config();	    
    dispatch = iniciar_servidor(config->PUERTO_ESCUCHA_DISPATCH);
	if(dispatch == -1 ) {
		loguear_error("El servidor (dispatch) no pudo ser iniciado");
		return false;
		}
    conexion_memoria = crear_conexion(config->IP_MEMORIA,config->PUERTO_MEMORIA);
    if(conexion_memoria == -1 ) {
		loguear_error("Fallo en la conexión con Memoria");
		return false;
		}
	
	kernel_dispatch = esperar_cliente(dispatch);
    interrupt= iniciar_servidor(config->PUERTO_ESCUCHA_INTERRUPT);
	if(interrupt == -1 ) {
		loguear_error("El servidor (interrupt) no pudo ser iniciado");
		return false;
		}
    kernel_interrupt = esperar_cliente(interrupt);
	
	return true;
}

void config_destroy_cpu(t_config_cpu* config){

	 config_destroy(config->config);
	 free(config);
}

void finalizar_cpu(){
	if (config) config_destroy_cpu(config);
	if(logger) log_destroy(logger);
	if (conexion_memoria !=-1) 
		liberar_conexion(conexion_memoria);

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

 void pedir_proxima_instruccion(t_pcb* pcb){
	enviar_pcb(pcb,PROXIMA_INSTRUCCION,conexion_memoria);
 }

 
 char* recibir_instruccion(){
	char* mje_inst = NULL;
	int op = recibir_operacion(conexion_memoria);
	if(op==MENSAJE)
		mje_inst =  recibir_mensaje(conexion_memoria);
	
	return mje_inst;
 }


 void ejecutar_instruccion(t_pcb* pcb,char* instruccion){

	loguear("Ejecutando instrucción: %s ...", instruccion);
	pcb->program_counter++;

 }

bool es_exit(char* comando){
		return string_equals_ignore_case(comando,(char*)EXIT_PROGRAM);
	}

 void ejecutar_programa(t_pcb* pcb){
	pedir_proxima_instruccion(pcb);
	char* mje_inst = recibir_instruccion();

	bool es_fin = es_exit(mje_inst);
	while (mje_inst!=NULL && !es_fin)
	{		
		if(!es_fin){
			ejecutar_instruccion(pcb,mje_inst);
			pedir_proxima_instruccion(pcb);
			mje_inst = recibir_instruccion();
		}	
		
		es_fin = es_exit(mje_inst);

	}
	enviar_texto("fin",FIN_PROGRAMA,conexion_memoria);
	free(mje_inst);	
 }

