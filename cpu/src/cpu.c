#include "cpu.h"

int kernel_dispatch, dispatch, interrupt, kernel_interrupt, conexion_memoria;
int tamanio_pagina;
//op_code cod_op_kernel_dispatch;
op_code cod_op_kernel_interrupt;
char *IR, *INSTID;
t_param PARAM1, PARAM2, PARAM3;
pthread_mutex_t mutex_interrupt= PTHREAD_MUTEX_INITIALIZER;
t_config_cpu *config;
t_registros_cpu *registros_cpu;
t_dictionary *diccionario_registros_cpu;

t_config_cpu *iniciar_config_cpu(char *path_config)
{
	t_config *_config = config_create(path_config);
	if (_config == NULL)
		return NULL;
	t_config_cpu *config_cpu = malloc(sizeof(t_config_cpu));
	config_cpu->IP_MEMORIA = config_get_string_value(_config, "IP_MEMORIA");
	config_cpu->PUERTO_MEMORIA = config_get_int_value(_config, "PUERTO_MEMORIA");
	config_cpu->PUERTO_ESCUCHA_DISPATCH = config_get_int_value(_config, "PUERTO_ESCUCHA_DISPATCH");
	config_cpu->PUERTO_ESCUCHA_INTERRUPT = config_get_int_value(_config, "PUERTO_ESCUCHA_INTERRUPT");
	config_cpu->config = _config;

	return config_cpu;
}

bool iniciar_log_config(char *path_config)
{
	decir_hola(MODULO);
	logger = iniciar_logger(MODULO);
	if (logger == NULL)
	{
		printf("EL LOGGER NO PUDO SER INICIADO.\n");
		return false;
	}
	config = iniciar_config_cpu(path_config);
	if (config == NULL)
	{
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}
	loguear_config();
	return true;
}

t_dictionary *iniciar_diccionario_cpu()
{
	t_dictionary *diccionario = dictionary_create();
	dictionary_put(diccionario, "AX", &registros_cpu->AX);
	dictionary_put(diccionario, "BX", &registros_cpu->BX);
	dictionary_put(diccionario, "CX", &registros_cpu->CX);
	dictionary_put(diccionario, "DX", &registros_cpu->DX);
	dictionary_put(diccionario, "EAX", &registros_cpu->EAX);
	dictionary_put(diccionario, "EBX", &registros_cpu->EBX);
	dictionary_put(diccionario, "ECX", &registros_cpu->ECX);
	dictionary_put(diccionario, "EDX", &registros_cpu->EDX);
	dictionary_put(diccionario, "DI", &registros_cpu->DI);
	dictionary_put(diccionario, "SI", &registros_cpu->SI);
	dictionary_put(diccionario, "PC",&registros_cpu->PC);

	return diccionario;
}

bool iniciar_registros_cpu()
{	
	loguear("CPU inicializará registros");
	registros_cpu = inicializar_registros(registros_cpu);

	loguear("Registros CPU logueados");

	IR = string_new();
	INSTID = string_new();
	if (registros_cpu == NULL)
	{
		loguear_error("No se pudieron iniciar los registros correctamente");
		return false;
	}

	diccionario_registros_cpu = iniciar_diccionario_cpu();

	return true;
}

bool iniciar_dispatch()
{
	dispatch = iniciar_servidor(config->PUERTO_ESCUCHA_DISPATCH);
	if (dispatch == -1)
	{
		loguear_error("El servidor (dispatch) no pudo ser iniciado");
		return false;
	}
	return true;
}

bool iniciar_conexion_memoria()
{
	conexion_memoria = crear_conexion(config->IP_MEMORIA, config->PUERTO_MEMORIA);
	if (conexion_memoria == -1)
	{
		loguear_error("Fallo en la conexión con Memoria");
		return false;
	}
	int codigo_operacion = recibir_operacion(conexion_memoria);
	if(codigo_operacion = TAMANIO_PAGINA){
		tamanio_pagina=atoi(recibir_mensaje(conexion_memoria));
		loguear("TAM_PAGINA: %d B");
	}

	return true;
}

bool iniciar_conexion_kernel()
{
	kernel_dispatch = esperar_cliente(dispatch);
	interrupt = iniciar_servidor(config->PUERTO_ESCUCHA_INTERRUPT);
	if (interrupt == -1)
	{
		loguear_error("El servidor (interrupt) no pudo ser iniciado");
		return false;
	}
	kernel_interrupt = esperar_cliente(interrupt);
	return true;
}

bool iniciar_cpu(char *path_config)
{
	return iniciar_log_config(path_config) &&
		   iniciar_registros_cpu() &&
		   iniciar_dispatch() &&
		   iniciar_conexion_memoria() &&
		   iniciar_conexion_kernel() &&
		   iniciar_variables()	&&
		   iniciar_gestion_interrupcion();
}


bool iniciar_variables()
{
	cod_op_kernel_interrupt = EJECUTAR_CPU;
	//pthread_mutex_init(&mutex_interrupt, NULL);
	return true;
}

bool iniciar_gestion_interrupcion(){
	pthread_t thread_interrupt;
	pthread_create(&thread_interrupt, NULL, gestionar_interrupcion, NULL);
	pthread_detach(thread_interrupt);
	if (thread_interrupt == -1)
		loguear_error("No se pudo iniciar el hilo de interrupciones.");
	return true;
}

void config_destroy_cpu(t_config_cpu *config)
{

	config_destroy(config->config);
	free(config);
}

void liberar_param(t_param parametro){ // Para agregar declaratividad
	free(parametro.string_valor);
}


void finalizar_cpu()
{
	if (config)
		config_destroy_cpu(config);
	if (logger)
		log_destroy(logger);
	finalizar_estructuras_cpu();
	if (conexion_memoria != -1)
		liberar_conexion(conexion_memoria);
	//if (mutex_interrupt) pthread_mutex_destroy(&mutex_interrupt);
}

void finalizar_estructuras_cpu()
{
	if (registros_cpu != NULL)
	{
		if(INSTID !=NULL)free(INSTID);
		// if(PARAM1 !=NULL)free(PARAM1);
		// if(PARAM2 !=NULL)free(PARAM2);
		// if(PARAM3 !=NULL)free(PARAM3);
		free(registros_cpu);
	}
	if (diccionario_registros_cpu)
	{
		dictionary_clean(diccionario_registros_cpu);
		dictionary_destroy(diccionario_registros_cpu);
	}
	// if (mutex_interrupt != NULL)
	// {
	// 	pthread_mutex_destroy(mutex_interrupt);
	// }
}

void loguear_config()
{
	loguear("IP_MEMORIA: %s", config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d", config->PUERTO_MEMORIA);
	loguear("PUERTO_ESCUCHA_DISPATCH: %d", config->PUERTO_ESCUCHA_DISPATCH);
	loguear("PUERTO_ESCUCHA_INTERRUPT: %d", config->PUERTO_ESCUCHA_INTERRUPT);
}

char *recibir_instruccion()
{
	char *mje_inst = NULL;
	int op = recibir_operacion(conexion_memoria);
	if (op == MENSAJE)
		mje_inst = recibir_mensaje(conexion_memoria);

	return mje_inst;
}

char *pedir_proxima_instruccion(t_pcb *pcb)
{
	enviar_pcb(pcb, PROXIMA_INSTRUCCION, conexion_memoria);
	return recibir_instruccion();
}

bool check_interrupt(){return (cod_op_kernel_interrupt != EJECUTAR_CPU && !es_exit(IR) && !es_io_handler(INSTID));};
bool es_io_handler(char* INSTID){
if(!strcmp(INSTID,"IO_GEN_SLEEP")) return true;
return false;
}
bool continuar_ciclo_instruccion(){return (!es_exit(IR)) && !check_interrupt() && !es_io_handler(INSTID);};

 void ciclo_de_instruccion(t_pcb* pcb){
	bool flag_execute;
	do{		
		fetch(pcb);
		decode();
		flag_execute = execute(pcb);
		
		if(check_interrupt())
			devolver_contexto(pcb,cod_op_kernel_interrupt);

	}while (continuar_ciclo_instruccion() && flag_execute);

	if(es_exit(IR))
		enviar_texto("fin",FIN_PROGRAMA,conexion_memoria);


	if(IR!=NULL) free(IR);	
 }




t_param interpretar_valor_instruccion(char* valor){
	t_param parametro;
	if(dictionary_has_key(diccionario_registros_cpu,valor)){
	 parametro.puntero=dictionary_get(diccionario_registros_cpu,valor);
	 if(!strcmp(valor,"AX")||!strcmp(valor,"BX")||!strcmp(valor,"CX")||!strcmp(valor,"DX")){
	 parametro.size = sizeof(uint8_t);
	 parametro.string_valor=string_new();
	 sprintf(parametro.string_valor,"%d",*(uint8_t*)parametro.puntero);
	 }
	 else {
	 parametro.size=sizeof(uint32_t);
	 parametro.string_valor = string_new();
	 sprintf(parametro.string_valor,"%d",*(uint32_t*)parametro.puntero);
	 }
	 return parametro;

	} else{
		
		parametro.puntero=malloc(sizeof(uint32_t));
		uint32_t valor_uint32 = atoi(valor);
		memcpy(parametro.puntero,&valor_uint32,sizeof(valor_uint32));
		parametro.size = sizeof(uint32_t);
		parametro.string_valor = string_duplicate(valor);
		return parametro;
	}
}

bool fetch(t_pcb *pcb)
{

	actualizar_registros(pcb);
	IR = pedir_proxima_instruccion(pcb);
	loguear("PID: <%i> - FETCH - Program Counter: <%i>",
			pcb->PID,
			pcb->program_counter);
	if (IR == NULL)
		return false;
	return true;
}

bool decode()
{
	
	char **sep_instruction = string_array_new();
	char *registros = string_new();
	registros = string_duplicate(IR);
	sep_instruction = string_split(registros, " ");
	INSTID = string_duplicate(sep_instruction[0]);
	if (sep_instruction == NULL || registros_cpu == NULL)
		return false;
	if (es_exit(INSTID))
		return true;
	// Acá están las funciones
	if (sep_instruction[1])
		PARAM1 = interpretar_valor_instruccion(sep_instruction[1]);
	if (sep_instruction[2])
		PARAM2 = interpretar_valor_instruccion(sep_instruction[2]); // esta de acá
	if (sep_instruction[3])
		PARAM3 = interpretar_valor_instruccion(sep_instruction[3]);
	string_array_destroy(sep_instruction);
	free(registros);
	return true;
}


bool execute(t_pcb *pcb)
{
	if (!strcmp(INSTID, "SET"))
	{
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor);
		exe_set(PARAM1, PARAM2);
		actualizar_contexto(pcb);
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if (!strcmp(INSTID, "SUM"))
	{	
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor);
		exe_sum(PARAM1, PARAM2);
		actualizar_contexto(pcb);
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if (!strcmp(INSTID, "SUB"))
	{
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor);
		exe_sub(PARAM1, PARAM2);
		actualizar_contexto(pcb);
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if (!strcmp(INSTID, "JNZ"))
	{
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor);
		exe_jnz(PARAM1, PARAM2);
		actualizar_contexto(pcb);
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if (!strcmp(INSTID, "IO_GEN_SLEEP"))
	{
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor);
		exe_io_gen_sleep(pcb,PARAM1, PARAM2);

		
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if(!strcmp(INSTID, "COPY_STRING")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s>", pcb->PID, INSTID, PARAM1.string_valor);
		exe_copy_string(PARAM1);
		actualizar_contexto(pcb);
		liberar_param(PARAM1);
		return true;
	}
	if(!strcmp(INSTID, "RESIZE")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s>", pcb->PID, INSTID, PARAM1.string_valor);
		bool flag_resize=exe_resize(pcb,PARAM1);
		
		liberar_param(PARAM1);
		return flag_resize;
	}
	if(!strcmp(INSTID, "MOV_IN")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor);
		exe_mov_in(pcb,PARAM1,PARAM2);

		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if(!strcmp(INSTID, "MOV_OUT")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor);
		exe_mov_out(pcb,PARAM1,PARAM2);

		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if (es_exit(INSTID))
	{	

		loguear("PID: <%d> - Ejecutando: <%s>", pcb->PID, INSTID);
		exe_exit(pcb);
		loguear("Saliendo...");
		return true;
	}

	loguear_error("INSTRUCCION NO ENCONTRADA");
	return false;
}


bool exe_set(t_param registro, t_param valor)
{
	
	printf("Tamaño de registro: %d, Tamaño de valor: %d\n", registro.size, valor.size);
    printf("Dirección de memoria de registro: %p, Dirección de memoria de valor: %p\n", registro.puntero, valor.puntero);

    // Copia el valor de 'valor.puntero' al puntero 'registro.puntero'
    memcpy(registro.puntero, valor.puntero, registro.size < valor.size ? registro.size : valor.size);
	registros_cpu->PC++;
    return true;
}
bool exe_sum(t_param registro_destino, t_param incremento)
{

	int *registro_destino_valor = (int *)(registro_destino.puntero);
    int *incremento_valor = (int *)(incremento.puntero);
    *registro_destino_valor += *incremento_valor;
	registros_cpu->PC++;
	return true;
}

bool exe_sub(t_param registro_destino,t_param incremento)
{
	int *registro_destino_valor = (int *)(registro_destino.puntero);
    int *incremento_valor = (int *)(incremento.puntero);
	if( *registro_destino_valor> *incremento_valor)
    	*registro_destino_valor -= *incremento_valor;
	else 
		 *registro_destino_valor-= *registro_destino_valor;
	registros_cpu->PC++;
	return true;
}

bool exe_jnz(t_param registro_destino, t_param nro_instruccion)
{

	if (*(uint8_t*)registro_destino.puntero != 0)
		 registros_cpu->PC = *(uint32_t*)nro_instruccion.puntero;
	else
		 registros_cpu->PC++;


	return true;
}

bool exe_io_gen_sleep(t_pcb* pcb,t_param interfaz, t_param unidades_de_trabajo)
{
	char* texto = string_new();
	
	(uint32_t)registros_cpu->PC++;
	actualizar_contexto(pcb);
	enviar_pcb(pcb,IO_HANDLER,kernel_dispatch);
	sprintf(texto,"%s %s",interfaz.string_valor,unidades_de_trabajo.string_valor);
	enviar_texto(texto,IO_GEN_SLEEP,kernel_dispatch);
	
	free(texto);
	return true;
}
bool exe_mov_in(t_pcb* pcb_recibido,t_param registro_datos,t_param registro_direccion){
	char* direccion_enviar = string_new();
	char* valor_memoria = string_new();
	uint32_t direccion_fisica = mmu(pcb_recibido,*(uint32_t*)registro_direccion.puntero);
	sprintf(direccion_enviar,"%d",direccion_fisica);
	enviar_texto(direccion_enviar,LECTURA_MEMORIA,conexion_memoria);
	
	int response = recibir_operacion(conexion_memoria);
	if(response == VALOR_CONSULTA_CPU){
		valor_memoria = recibir_mensaje(conexion_memoria);
		registro_datos.string_valor =string_duplicate(valor_memoria);
		registro_datos.size = sizeof(uint32_t);
		uint32_t* valor = malloc(registro_datos.size);
		*valor = (uint32_t) atoi(valor_memoria);
		registro_datos.puntero = &valor;
	}
	registros_cpu->PC++;
	free(direccion_enviar);
	free(valor_memoria);
	return true;
}
bool exe_mov_out(t_pcb* pcb_recibido,t_param registro_direccion ,t_param registro_datos){
	char* direccion_enviar = string_new();
	char* valor_memoria = string_new();
	uint32_t direccion_fisica = mmu(pcb_recibido,*(uint32_t*)registro_direccion.puntero);
	sprintf(direccion_enviar,"%d %d %s",pcb_recibido->PID,direccion_fisica, registro_datos.string_valor);
	enviar_texto(direccion_enviar,ESCRITURA_MEMORIA,conexion_memoria);
	int operacion_ok = recibir_operacion(conexion_memoria);
	if(operacion_ok== MOV_OUT_OK){
		valor_memoria=recibir_mensaje(conexion_memoria);
		loguear(valor_memoria);
		return true;
	}
	return false;
}
bool exe_resize(t_pcb* pcb,t_param p_tamanio){
//	enviar_texto(tamanio.string_valor,RESIZE,conexion_memoria);
	uint32_t tamanio = atoi(p_tamanio.string_valor);
	t_pid_valor* tamanio_proceso= pid_value_create(pcb->PID,tamanio);
    enviar_pid_value(tamanio_proceso,RESIZE,conexion_memoria);
    loguear_pid_value(tamanio_proceso);
    free(tamanio_proceso);


	int response = recibir_operacion(conexion_memoria);
	if(response == OUT_OF_MEMORY){
		actualizar_contexto(pcb);
		devolver_contexto(pcb, OUT_OF_MEMORY);
		return false;
	}
	return true;
}
bool exe_copy_string(t_param tamanio){
	memcpy(&registros_cpu->DI,&registros_cpu->SI,atoi(tamanio.string_valor));
	registros_cpu->PC++;
	return true;
}
bool exe_exit(t_pcb *pcb)
{
	devolver_contexto(pcb,CPU_EXIT);
	return true;
}

uint32_t mmu (t_pcb* pcb,uint32_t direccion_logica){
	uint32_t direccion_fisica;
	uint32_t numero_pagina = floor(direccion_logica/tamanio_pagina);
	char* nro_frame = string_new();

	// Agrego el pcb y ple paso un proceso a la memoria para que pueda encontrar que proceso le está pidiendo la CPU
	t_pid_valor* tamanio_proceso= pid_value_create(pcb->PID,numero_pagina); //Vamos con esta conversion?
	// FALTA LOGICA SEGUIR PIDIENDO FRAMES HASTA TENER UN FIN DE CADENA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Haciendo esta reutilización, capaz hay que cambiar el nombre de tamanio. Acá en tamanio le estamos pasando una página
	enviar_pid_value(tamanio_proceso,ACCESO_TABLA_PAGINAS,conexion_memoria);
	//enviar_texto(nro_pagina,ACCESO_TABLA_PAGINAS,conexion_memoria);

	int response = recibir_operacion(conexion_memoria);
	if(response == RESPUESTA_NRO_FRAME)
		nro_frame = recibir_mensaje(conexion_memoria);

	uint32_t desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
	direccion_fisica = tamanio_pagina * (uint32_t)atoi(nro_frame) + desplazamiento;
	return direccion_fisica;
}

bool devolver_contexto(t_pcb *pcb,op_code codigo_operacion)
{
	loguear_warning("Código de operación a enviar: %d para el PID %d:",codigo_operacion,pcb->PID);
	enviar_pcb(pcb,codigo_operacion,kernel_dispatch);
	// el pcb Siempre debe devolverse por dispatch
	loguear_warning("Código de operación enviado!!"); 
	return true;
}

bool actualizar_contexto(t_pcb *pcb)
{
	// aca deberiamos actualizar el quantum
	*pcb->registros_cpu = *registros_cpu;
	pcb->program_counter = registros_cpu->PC;

	return true;
}
bool actualizar_registros(t_pcb *pcb)
{
	memcpy(registros_cpu,pcb->registros_cpu,sizeof(t_registros_cpu));
	registros_cpu->PC = pcb->program_counter;
	return true;
}
int ejecutar_proceso_cpu()
{
	loguear("Arranco la ejecucion del proceso");
	while (1)
	{
		t_paquete *paquete = recibir_paquete(kernel_dispatch);
		int cod_op = paquete->codigo_operacion;
		loguear("Cod op Kernel: %d", cod_op);
        switch (cod_op) {
            case EJECUTAR_PROCESO:
                t_pcb *pcb = recibir_pcb(paquete); 
				cod_op_kernel_interrupt = EJECUTAR_CPU;
                ciclo_de_instruccion(pcb);
				paquete_destroy(paquete);
                break;	
            case -1:
			loguear_error("el cliente se desconectó. Terminando servidor");
			paquete_destroy(paquete);
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			paquete_destroy(paquete);
			return EXIT_FAILURE;
		}
		// Si no se agrega otro caso, convertir switch en IF
	}
}

void* gestionar_interrupcion(){
	int estado_guardado;
    while(1){
        estado_guardado=recibir_operacion(kernel_interrupt);
        recibir_paquete(kernel_interrupt);
        pthread_mutex_lock(&mutex_interrupt);
        cod_op_kernel_interrupt=estado_guardado;
        pthread_mutex_unlock(&mutex_interrupt);
    }

}