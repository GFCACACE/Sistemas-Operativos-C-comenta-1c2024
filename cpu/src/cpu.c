#include "cpu.h"

int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
int cod_op_kernel_dispatch;
t_config_cpu * config;
t_regist_cpu* registros_cpu;
t_dictionary* diccionario_registros_cpu;

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

	registros_cpu = iniciar_registros_cpu();
	if(registros_cpu == NULL){
		loguear_error("No se pudieron iniciar los registros correctamente");
		return false;
	}
	diccionario_registros_cpu = iniciar_diccionario_cpu();
	if(diccionario_registros_cpu == NULL){
		loguear_error("No se pudo iniciar el diccionario de registros");
		return false;
	}
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
	finalizar_estructuras_cpu();
	if (conexion_memoria !=-1) 
		liberar_conexion(conexion_memoria);

}
void finalizar_estructuras_cpu(){
	if(registros_cpu){
		free(registros_cpu->AX);
		free(registros_cpu->BX);
		free(registros_cpu->CX);
		free(registros_cpu->DX);
		free(registros_cpu->EAX);
		free(registros_cpu->EBX);
		free(registros_cpu->ECX);
		free(registros_cpu->EDX);
		free(registros_cpu->DI);
		free(registros_cpu->SI);
		free(registros_cpu->IR);
		free(registros_cpu->INSTID);
		free(registros_cpu->PARAM1);
		free(registros_cpu->PARAM2);
		free(registros_cpu->PARAM3);
		free(registros_cpu);
	}
	if(diccionario_registros_cpu){
		dictionary_clean(diccionario_registros_cpu);
		dictionary_destroy(diccionario_registros_cpu);
	}
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
	//Signal semáforo de memoria
 }


 char* recibir_instruccion(){
	char* mje_inst = NULL;
	//Wait semáforo de memoria
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

	
	while (mje_inst)
	{	
		bool es_fin = es_exit(mje_inst);
		if(es_fin)
			break;		
		ejecutar_instruccion(pcb,mje_inst);
		free(mje_inst);
		pedir_proxima_instruccion(pcb);
		mje_inst = recibir_instruccion();		

	}
	enviar_texto("fin",FIN_PROGRAMA,conexion_memoria);
	if(mje_inst!=NULL)
	free(mje_inst);	
 }

t_regist_cpu* iniciar_registros_cpu(){
	t_regist_cpu* reg_cpu = malloc(sizeof(t_regist_cpu));
	reg_cpu->IR=string_new();
	reg_cpu->INSTID=string_new();
	reg_cpu->PARAM1=malloc(sizeof(uint32_t));
	reg_cpu->PARAM2=malloc(sizeof(uint32_t));
	reg_cpu->PARAM3=malloc(sizeof(uint32_t));
	return reg_cpu;
 }

t_dictionary* iniciar_diccionario_cpu(){
	t_dictionary* diccionario = dictionary_create();
	dictionary_put(diccionario,"AX",&registros_cpu->AX);
	dictionary_put(diccionario,"BX",&registros_cpu->BX);
	dictionary_put(diccionario,"CX",&registros_cpu->CX);
	dictionary_put(diccionario,"DX",&registros_cpu->DX);
	dictionary_put(diccionario,"EAX",&registros_cpu->EAX);
	dictionary_put(diccionario,"EBX",&registros_cpu->EBX);
	dictionary_put(diccionario,"ECX",&registros_cpu->ECX);
	dictionary_put(diccionario,"EDX",&registros_cpu->EDX);
	dictionary_put(diccionario,"DI",&registros_cpu->DI);
	dictionary_put(diccionario,"SI",&registros_cpu->SI);
}

void* interpretar_valor_instruccion(char* valor){
	if(dictionary_has_key(diccionario_registros_cpu,valor)==true){
	return dictionary_get(diccionario_registros_cpu,valor);
	} else{
		return (void*)(uint32_t)atoi(valor);
	}
}

bool fetch(t_pcb* pcb){
	pedir_proxima_instruccion(pcb);
	registros_cpu->IR = recibir_instruccion();
	if (registros_cpu->IR == NULL) return false;
	return true;
}

bool decode(){
	registros_cpu->INSTID = NULL;
	registros_cpu->PARAM1=NULL;
	registros_cpu->PARAM2=NULL;
	registros_cpu->PARAM3=NULL;
	char**sep_instruction = string_array_new();
	char* registros = string_new();
	registros=string_duplicate(registros_cpu->IR);
	sep_instruction = string_split(registros," ");
	registros_cpu->INSTID = string_duplicate(sep_instruction[0]);
	if(registros_cpu == NULL) return false;
	if (sep_instruction[1]) registros_cpu->PARAM1=interpretar_valor_instruccion(sep_instruction[1]);
	if (sep_instruction[2]) registros_cpu->PARAM2=interpretar_valor_instruccion(sep_instruction[2]);
	if (sep_instruction[3]) registros_cpu->PARAM3=interpretar_valor_instruccion(sep_instruction[3]);
	string_array_destroy(sep_instruction);
	free(registros);
	return true;
}
bool execute(){
	if(strcmp(registros_cpu->INSTID,"SET")) {
		exe_set(&registros_cpu->PARAM1,registros_cpu->PARAM2);
		return true;
	}
	if(strcmp(registros_cpu->INSTID,"SUM")){ 
		exe_sum(&registros_cpu->PARAM1,registros_cpu->PARAM2);
		return true;
	}
	if(strcmp(registros_cpu->INSTID,"SUB")){
		exe_sub(&registros_cpu->PARAM1,registros_cpu->PARAM2);
		return true;
	}
	if(strcmp(registros_cpu->INSTID,"JNZ")){
		exe_jnz(&registros_cpu->PARAM1,registros_cpu->PARAM2);
		return true;
	}
	return false;
}
bool exe_set(uint32_t* registro,uint32_t valor){
	*registro = valor;
	return true;
}
bool exe_sum(uint32_t* registro_destino,uint32_t incremento){
	*registro_destino = *registro_destino + incremento;
	return true;
}

bool exe_sub(uint32_t* registro_destino,uint32_t decremento){
	*registro_destino = *registro_destino - decremento;
	return true;
}

bool exe_jnz(uint32_t*registro_destino,uint32_t nro_instruccion){

	if(*registro_destino) registros_cpu->PC = nro_instruccion; 

	return true;
}
