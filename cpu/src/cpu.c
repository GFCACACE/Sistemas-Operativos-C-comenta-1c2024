#include "cpu.h"

int kernel_dispatch,dispatch,interrupt,kernel_interrupt,conexion_memoria;
int cod_op_kernel_dispatch;
int cod_op_interrupt=EJECUTAR_CPU;
char* IR, *INSTID;
void* PARAM1, *PARAM2, *PARAM3;
pthread_t * mutex_interrupt;
t_config_cpu * config;
t_registros_cpu* registros_cpu;
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

bool iniciar_log_config(char* path_config){
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
	return true;
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

	return diccionario;	
}

bool iniciar_registros_cpu(){
	registros_cpu = malloc(sizeof(t_registros_cpu));
	IR=string_new();
	INSTID=string_new();
	PARAM1=malloc(sizeof(uint32_t));
	PARAM2=malloc(sizeof(uint32_t));
	PARAM3=malloc(sizeof(uint32_t));

	
	if(registros_cpu == NULL){
		loguear_error("No se pudieron iniciar los registros correctamente");
		return false;
	}

	diccionario_registros_cpu = iniciar_diccionario_cpu();

	return true;
 }

bool iniciar_dispatch(){
	  dispatch = iniciar_servidor(config->PUERTO_ESCUCHA_DISPATCH);
	if(dispatch == -1 ) {
		loguear_error("El servidor (dispatch) no pudo ser iniciado");
		return false;
		}
	return true;
}

bool iniciar_conexion_memoria(){
	conexion_memoria = crear_conexion(config->IP_MEMORIA,config->PUERTO_MEMORIA);
	if(conexion_memoria == -1 ) {
	loguear_error("Fallo en la conexión con Memoria");
	return false;
	}

	return true;
}

bool iniciar_conexion_kernel(){
	kernel_dispatch = esperar_cliente(dispatch);
    interrupt= iniciar_servidor(config->PUERTO_ESCUCHA_INTERRUPT);
	if(interrupt == -1 ) {
		loguear_error("El servidor (interrupt) no pudo ser iniciado");
		return false;
		}
    kernel_interrupt = esperar_cliente(interrupt);
	return true;
}

bool iniciar_cpu(char* path_config){
	return
	iniciar_log_config(path_config) &&
	iniciar_registros_cpu()	&&
	iniciar_dispatch()&&
	iniciar_conexion_memoria()
	&&
	iniciar_conexion_kernel()
	&&
	iniciar_semaforos();

}
bool iniciar_semaforos(){
	pthread_mutex_init(mutex_interrupt,NULL);
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
	if(registros_cpu != NULL){
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
		free(IR);
		free(INSTID);
		free(PARAM1);
		free(PARAM2);
		free(PARAM3);
		free(registros_cpu);
	}
	if(diccionario_registros_cpu){
		dictionary_clean(diccionario_registros_cpu);
		dictionary_destroy(diccionario_registros_cpu);
	}
	if(mutex_interrupt != NULL){
		pthread_mutex_destroy(mutex_interrupt);
	}
}



void loguear_config(){
	loguear("IP_MEMORIA: %s",config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d",config->PUERTO_MEMORIA);
	loguear("PUERTO_ESCUCHA_DISPATCH: %d",config->PUERTO_ESCUCHA_DISPATCH);
	loguear("PUERTO_ESCUCHA_INTERRUPT: %d",config->PUERTO_ESCUCHA_INTERRUPT);
}

 
 
 char* recibir_instruccion(){
	char* mje_inst = NULL;
	int op = recibir_operacion(conexion_memoria);
	if(op==MENSAJE)
		mje_inst =  recibir_mensaje(conexion_memoria);
	
	return mje_inst;
 }

 char* pedir_proxima_instruccion(t_pcb* pcb){
	enviar_pcb(pcb,PROXIMA_INSTRUCCION,conexion_memoria);
	return recibir_instruccion(); 
 }


 void ejecutar_instruccion(t_pcb* pcb,char* instruccion){

	loguear("Ejecutando instrucción: %s ...", instruccion);
	// HACER INSTRUCCION
	pcb->program_counter++;

 }

bool es_exit(char* comando){
		return string_equals_ignore_case(comando,(char*)EXIT_PROGRAM);
	}

 void ciclo_de_instruccion(t_pcb* pcb){
	
	fetch(pcb);
	while (!es_exit(IR ))
	{		
		// ejecutar_instruccion(pcb,IR);
	/*	pedirinteerupicion(mensaje a kernel)
		esperarrespuestakernel
		hayINterrupcion?*/
		decode();
		execute(pcb);
		// free(IR);
		check_interrupt();
		fetch(pcb);		
	//	mje_inst = pedir_proxima_instruccion(pcb);

	}
	enviar_texto("fin",FIN_PROGRAMA,conexion_memoria);
	if(IR!=NULL)
	free(IR);	
 }




void* interpretar_valor_instruccion(char* valor){
	if(dictionary_has_key(diccionario_registros_cpu,valor)==true){
	return dictionary_get(diccionario_registros_cpu,valor);
	} else{
		
		void* puntero_numerico = malloc(sizeof(uint32_t));
		 * (uint32_t*)puntero_numerico = atoi(valor); 

		return puntero_numerico;
	}
}

bool fetch(t_pcb* pcb){	
	
	actualizar_registros(pcb);
	IR = pedir_proxima_instruccion(pcb);
	loguear( "PID: <%i> - FETCH - Program Counter: <%i>",
	pcb->PID,
	pcb->program_counter);
//TODO;	actualizar_pcb(pcb); //sincronizar registros cpu con pcb
	if (IR == NULL) return false;
	return true;
}

bool decode(){
	INSTID = NULL;
	PARAM1=NULL;
	PARAM2=NULL;
	PARAM3=NULL;
	char**sep_instruction = string_array_new();
	char* registros = string_new();
	registros=string_duplicate(IR);
	sep_instruction = string_split(registros," ");
	INSTID = string_duplicate(sep_instruction[0]);
	if(registros_cpu == NULL) return false;  // Que valida esto???
	//Acá están las funciones
	if (sep_instruction[1]) PARAM1=interpretar_valor_instruccion(sep_instruction[1]);
	if (sep_instruction[2]) PARAM2=interpretar_valor_instruccion(sep_instruction[2]);//esta de acá
	if (sep_instruction[3]) PARAM3=interpretar_valor_instruccion(sep_instruction[3]);
	string_array_destroy(sep_instruction);
	free(registros);
	return true;
}
bool execute(t_pcb* pcb){
	if(strcmp(INSTID,"SET")==0) {
		exe_set(PARAM1,PARAM2);
		loguear("PID: <%d> - Ejecutando: <%s> - <%d> <%d>",pcb->PID,INSTID,*(uint32_t*)PARAM1,*(uint32_t*)PARAM2);
		actualizar_contexto(pcb);
		return true;
	}
	if(!strcmp(INSTID,"SUM")){ 
		exe_sum(PARAM1,PARAM2);
		loguear("PID: <%d> - Ejecutando: <%s> - <%d> <%d>",pcb->PID,INSTID,*(uint32_t*)PARAM1,*(uint32_t*)PARAM2);
		actualizar_contexto(pcb);
		return true;
	}
	if(!strcmp(INSTID,"SUB")){
		exe_sub(PARAM1,PARAM2);
		loguear("PID: <%d> - Ejecutando: <%s> - <%d> <%d>",pcb->PID,INSTID,*(uint32_t*)PARAM1,*(uint32_t*)PARAM2);
		actualizar_contexto(pcb);
		return true;
	}
	if(!strcmp(INSTID,"JNZ")){
		exe_jnz(PARAM1,PARAM2);
		actualizar_contexto(pcb);
		return true;
	}
	if(!strcmp(INSTID,"IO_GEN_SLEEP")){
		exe_io_gen_sleep(PARAM1,PARAM2);

		actualizar_contexto(pcb);
		return true;
	}
	
	return false;
}
bool exe_set(void* registro,void* valor){
	*(uint32_t*)registro =*(uint32_t*)valor;
	loguear("PC pre execute:%d",registros_cpu->PC);
	registros_cpu->PC++;
	loguear("PC post execute:%d",registros_cpu->PC);
	return true;
}
bool exe_sum(void* registro_destino,void* incremento){
	*(uint32_t*)registro_destino = *(uint32_t*)registro_destino + *(uint32_t*)incremento;
	registros_cpu->PC++;
	return true;
}

bool exe_sub(void* registro_destino,void *decremento){
	*(uint32_t*)registro_destino = *(uint32_t*)registro_destino - *(uint32_t*)decremento;
	registros_cpu->PC++;
	return true;
}

bool exe_jnz(void*registro_destino,void *nro_instruccion){

	if(registro_destino!=NULL && nro_instruccion!=NULL) {
		if(*(uint32_t*)registro_destino != 0)
		*registros_cpu->PC = *(uint32_t*)nro_instruccion; 
		else registros_cpu->PC++;
	}

	return true;
}

bool exe_io_gen_sleep(void*interfaz,void*unidades_de_trabajo){
	*(uint32_t*)registros_cpu->PC++;
	return true;
}

bool check_interrupt(t_pcb* pcb){
	/*cod_op_interrupt debería ser modificada por un thread
	 dedicado a recibir de kernel si desea interrumpir*/
	
	if(cod_op_interrupt = INTERRUMPIR_CPU){
		devolver_contexto(pcb);
	}
	return true;
}
bool devolver_contexto(t_pcb* pcb){
	// el pcb Siempre debe devolverse por dispatch
	enviar_pcb(pcb,DEVOLVER_CONTEXTO,dispatch);

	return true;
}

bool actualizar_contexto(t_pcb* pcb){

	*pcb->registros_cpu=*registros_cpu;
	pcb->program_counter = registros_cpu->PC;

	loguear("PC:%d AX:%d EAX:%d",
	pcb->registros_cpu->PC,
	pcb->registros_cpu->AX,
	pcb->registros_cpu->EAX
	);
	return true;
}
bool actualizar_registros(t_pcb* pcb){
	*registros_cpu = *pcb->registros_cpu;
	registros_cpu->PC = pcb->program_counter;
	return true;
}
int ejecutar_proceso_cpu(){
	loguear("Arranco la ejecucion del proceso");
	 while (1) {
        t_paquete *paquete = recibir_paquete(kernel_dispatch);
        int cod_op = paquete->codigo_operacion;
		loguear("Cod op: %d", cod_op);
        switch (cod_op) {
            case EJECUTAR_PROCESO:
                t_pcb *pcb = recibir_pcb(paquete); 
                ciclo_de_instruccion(pcb);
				paquete_destroy(paquete);
                break;				
			return EXIT_SUCCESS;
            case -1:
			loguear_error("el cliente se desconectó. Terminando servidor");
			paquete_destroy(paquete);
			return EXIT_FAILURE;
		    default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			paquete_destroy(paquete);
			return EXIT_FAILURE;
        }
		// Si no se agrega otro caso, convertir switch en IF

    }
}
