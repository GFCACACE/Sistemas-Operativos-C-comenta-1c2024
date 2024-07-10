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
t_list* lista_tlb;

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
	config_cpu->CANTIDAD_ENTRADAS_TLB = config_get_int_value(_config,"CANTIDAD_ENTRADAS_TLB");
	config_cpu->ALGORITMO_TLB = config_get_string_value(_config,"ALGORITMO_TLB");
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
	if(codigo_operacion == TAMANIO_PAGINA){
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

bool iniciar_tlb(){
	lista_tlb=list_create();
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
		   iniciar_gestion_interrupcion() &&
		   iniciar_tlb();
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
	// if(lista_tlb)
	// 	list_destroy_and_destroy_elements(lista_tlb);
	
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
	loguear("CANTIDAD_ENTRADAS_TLB: %d",config->CANTIDAD_ENTRADAS_TLB);
	loguear("ALGORITMO_TLB: %s", config->ALGORITMO_TLB);

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

bool check_interrupt(){ return (cod_op_kernel_interrupt != EJECUTAR_CPU && !es_exit(IR) && !es_io_handler(INSTID));};
bool es_io_handler(char* INSTID){	
	return (strncmp(INSTID, "IO_", 3) == 0);
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
	char *registros = string_duplicate(IR);	
	char **sep_instruction = string_split(registros, " ");
	INSTID = string_duplicate(sep_instruction[0]);
	if (sep_instruction == NULL || registros_cpu == NULL)
		return false;
	if (es_exit(INSTID))
		return true;
	// Acá están las funciones

	// LOS IF HACEN LO MISMO. PODRIAMOS PASARLO CON UNA FUNCION (5 PARAM MAX)
	if (sep_instruction[1])
		PARAM1 = interpretar_valor_instruccion(sep_instruction[1]);
	if (sep_instruction[2]){
		PARAM2 = interpretar_valor_instruccion(sep_instruction[2]); // esta de acá
		if (sep_instruction[3])
			PARAM3 = interpretar_valor_instruccion(sep_instruction[3]);
	}
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
	if(!strcmp(INSTID,"WAIT")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s>", pcb->PID, INSTID, PARAM1.string_valor);
		exe_wait(pcb,PARAM1);
		liberar_param(PARAM1);
		return false;
	}
	if(!strcmp(INSTID,"SIGNAL")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s>", pcb->PID, INSTID, PARAM1.string_valor);
		exe_signal(pcb,PARAM1);
		liberar_param(PARAM1);
		return false;
	}
	if(!strcmp(INSTID, "COPY_STRING")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s>", pcb->PID, INSTID, PARAM1.string_valor);
		exe_copy_string(pcb,PARAM1);
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
	if(!strcmp(INSTID,"IO_STDIN_READ")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor,PARAM3.string_valor);
		exe_std(IO_STDIN_READ, pcb,PARAM1,PARAM2,PARAM3);
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		liberar_param(PARAM3);
		return true;
	}
	if(!strcmp(INSTID,"IO_STDOUT_WRITE")){
		loguear("PID: <%d> - Ejecutando: <%s> - <%s> <%s> <%s>", pcb->PID, INSTID, PARAM1.string_valor, PARAM2.string_valor,PARAM3.string_valor);
		exe_std(IO_STDOUT_WRITE, pcb,PARAM1,PARAM2,PARAM3);
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		liberar_param(PARAM3);
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

bool exe_std(op_code cod_op, t_pcb* pcb,t_param interfaz,t_param registro_direccion, t_param registro_tamanio)
{
	uint32_t direccion_logica = *(uint32_t*)registro_direccion.puntero;
	uint32_t tamanio = *(uint32_t*)registro_tamanio.string_valor;
	t_direcciones_proceso* direcciones_fisicas_registros = obtener_paquete_direcciones(pcb,direccion_logica,tamanio);
	
	(uint32_t)registros_cpu->PC++;
	actualizar_contexto(pcb);
	
	enviar_pcb(pcb,IO_HANDLER,kernel_dispatch);
	enviar_texto(interfaz.string_valor,cod_op,kernel_dispatch);
	enviar_direcciones_proceso(direcciones_fisicas_registros,cod_op,kernel_dispatch);
	
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
	
	uint32_t size_registro = (uint32_t)registro_datos.size;
	uint32_t direccion_logica = *(uint32_t*)registro_direccion.puntero;
	t_direcciones_proceso* direcciones_fisicas_registros = obtener_paquete_direcciones(pcb_recibido,direccion_logica,size_registro);
	loguear_direccion_proceso(direcciones_fisicas_registros);
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	
	t_direccion_registro* direccion_registro_inicial =  list_get(direcciones_registros,0);
	uint32_t direccion_fisica_inicial = direccion_registro_inicial->direccion_fisica;
	int registro_reconstruido;
	void* registro_reconstruido_puntero =  &registro_reconstruido;

	t_buffer* buffer_lectura = leer_memoria_completa(direcciones_fisicas_registros,conexion_memoria);

	int registro_valor = 0;
	void* registro_valor_puntero = &registro_valor;
	memcpy(registro_valor_puntero,buffer_lectura->stream,buffer_lectura->size);

	memcpy(registro_reconstruido_puntero,registro_valor_puntero,buffer_lectura->size);
	
	loguear("Valor post a reconstruir <%d>",registro_reconstruido);

	loguear("PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>",
	pcb_recibido->PID,
	direccion_fisica_inicial,
	registro_reconstruido);
	buffer_destroy(buffer_lectura);
	
	registros_cpu->PC++;
	actualizar_contexto(pcb_recibido);
	
	return true;
}

bool exe_mov_out(t_pcb* pcb_recibido,t_param registro_direccion ,t_param registro_datos){
	uint32_t size_registro = (uint32_t)registro_datos.size; 
	char* registro_dato = (char*)registro_datos.string_valor; 
	uint32_t direccion_logica = *(uint32_t*)registro_direccion.puntero;
	t_direcciones_proceso* direcciones_fisicas_registros = obtener_paquete_direcciones(pcb_recibido,direccion_logica,size_registro);
	loguear_direccion_proceso(direcciones_fisicas_registros);	
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_direccion_registro* direccion_registro_inicial =  list_get(direcciones_registros,0);
	uint32_t direccion_fisica_inicial = direccion_registro_inicial->direccion_fisica;
	
	
	escribir_memoria_completa(direcciones_fisicas_registros,registro_dato,conexion_memoria);
	
		loguear("PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>",
		pcb_recibido->PID,
		direccion_fisica_inicial,
		registro_dato);


		registros_cpu->PC++;
		actualizar_contexto(pcb_recibido);
		return true;
//	}
//	return false;
}


bool exe_resize(t_pcb* pcb,t_param p_tamanio){
//	enviar_texto(tamanio.string_valor,RESIZE,conexion_memoria);
	uint32_t tamanio = atoi(p_tamanio.string_valor);
	t_pid_valor* tamanio_proceso= pid_value_create(pcb->PID,tamanio);
    enviar_pid_value(tamanio_proceso,RESIZE,conexion_memoria);
    loguear_pid_value(tamanio_proceso);
    free(tamanio_proceso);


	int response = recibir_operacion(conexion_memoria);
	recibir_paquete(conexion_memoria);
	if(response == OUT_OF_MEMORY){
		actualizar_contexto(pcb);
		devolver_contexto(pcb, OUT_OF_MEMORY);
		return false;
	}

	registros_cpu->PC++;
	actualizar_contexto(pcb);
	return true;
}
bool exe_copy_string(t_pcb* pcb,t_param tamanio){
	
	//uint32_t tamanio_valor = *(uint32_t*) tamanio.string_valor;
	//t_direcciones_proceso* direcciones_origen = obtener_paquete_direcciones(pcb,registros_cpu->SI,tamanio_valor);
	//t_direcciones_proceso* direcciones_destino = obtener_paquete_direcciones(pcb,registros_cpu->DI,tamanio_valor);
	//uint32_t size_registro = tamanio.size;
	//t_direccion_registro* direccion_tamaño_registro;
	
	



	//
	registros_cpu->PC++;
	actualizar_contexto(pcb);
	return true;
}
// bool exe_stdin_read(t_pcb* pcb, t_param interfaz, t_param registro_direccion,t_param registro_tamanio){
// 	loguear_warning("STDIN_READ no implementado");
// 	(uint32_t)registros_cpu->PC++;
// 	actualizar_contexto(pcb);
// 	return true;
// }
// bool exe_stdout_write(t_pcb* pcb, t_param interfaz, t_param registro_direccion,t_param registro_tamanio){
// 	loguear_warning("STDOUT_WRITE no implementado");
// 	(uint32_t)registros_cpu->PC++;
// 	actualizar_contexto(pcb);
// 	return true;
// }
bool exe_wait(t_pcb* pcb,t_param recurso){
	(uint32_t)registros_cpu->PC++;
	actualizar_contexto(pcb);
	enviar_pcb(pcb,REC_HANDLER,kernel_dispatch);
	enviar_texto(recurso.string_valor,WAIT,kernel_dispatch);

	return true;
}
bool exe_signal(t_pcb* pcb,t_param recurso){
	(uint32_t)registros_cpu->PC++;
	actualizar_contexto(pcb);
	enviar_pcb(pcb,REC_HANDLER,kernel_dispatch);
	enviar_texto(recurso.string_valor,SIGNAL,kernel_dispatch);

	return true;
}
bool exe_exit(t_pcb *pcb)
{
	devolver_contexto(pcb,CPU_EXIT);
	return true;
}
t_tlb* crear_registro_tlb(uint32_t PID, uint32_t numero_pagina, uint32_t numero_frame){
	t_tlb* registro_tlb=malloc(sizeof(t_tlb));
	registro_tlb->PID = PID;
	registro_tlb->numero_pagina = numero_pagina;
	registro_tlb->numero_frame = numero_frame;
	registro_tlb->timestamp = time(NULL);
	return registro_tlb;
}

int tlb_hit(uint32_t pid, uint32_t numero_pagina){
	
	t_tlb* registro;
	for(int i=0;i<list_size(lista_tlb);i++){
		registro = list_get(lista_tlb,i);
		if(pid==registro->PID && numero_pagina == registro->numero_pagina){
			if(!strcmp("LRU",config->ALGORITMO_TLB))
				registro->timestamp = time(NULL);
			
			return registro->numero_frame;
		}
	}
	

	return -1;
}
bool actualizar_tlb(uint32_t PID, uint32_t numero_pagina, uint32_t numero_frame){
	t_tlb* registro_tlb = crear_registro_tlb(PID,numero_pagina, numero_frame);
	if(list_size(lista_tlb)< config->CANTIDAD_ENTRADAS_TLB){
		list_add(lista_tlb,registro_tlb);
		return true;
	}else{
		return reemplazo_tlb(registro_tlb);
	}
	return false;
}
bool reemplazo_tlb(t_tlb* registro_nuevo){
	t_tlb* registro_a_comparar,*registro_menor_prioridad;
	int i=0;
	registro_menor_prioridad=list_get(lista_tlb,i);

	i++;
	while(i < list_size(lista_tlb)){
		registro_a_comparar=list_get(lista_tlb,i);
		if(registro_a_comparar->timestamp < registro_menor_prioridad->timestamp)
			registro_menor_prioridad = registro_a_comparar;
		i++;
	}
	list_remove_element(lista_tlb,registro_menor_prioridad);
	list_add(lista_tlb,registro_nuevo);
	return true;
}

uint32_t obtener_numero_pagina(uint32_t direccion_logica){
	return  floor(direccion_logica/tamanio_pagina);
}

uint32_t obtener_desplazamiento(uint32_t direccion_logica,uint32_t numero_pagina){
	return direccion_logica - numero_pagina * tamanio_pagina;
}
uint32_t obtener_cantidad_paginas(uint32_t desplazamiento,uint32_t size_registro){
	return (int)ceil((double) (desplazamiento + size_registro) / tamanio_pagina);
}

t_direcciones_proceso* obtener_paquete_direcciones(t_pcb* pcb,uint32_t direccion_logica, uint32_t size_registro){
	t_direcciones_proceso* direcciones_registros = direcciones_proceso_create(pcb->PID,size_registro);
	uint32_t numero_pagina = obtener_numero_pagina(direccion_logica);
	uint32_t desplazamiento = obtener_desplazamiento(direccion_logica,numero_pagina);
	uint32_t cantidad_paginas = obtener_cantidad_paginas(desplazamiento,size_registro);
	
	uint32_t direccion_fisica;
	uint32_t size_restante_registro = size_registro;
	uint32_t indice_pagina;
	uint32_t size_registro_pagina_actual;
	
	for (indice_pagina=numero_pagina;indice_pagina < numero_pagina + cantidad_paginas;indice_pagina++){

		desplazamiento = obtener_desplazamiento(direccion_logica,indice_pagina);
		
		direccion_fisica = mmu(pcb,direccion_logica);
		
		uint32_t bytes_restantes_pagina = tamanio_pagina - desplazamiento;
	
		if(size_restante_registro > bytes_restantes_pagina){
			size_registro_pagina_actual = bytes_restantes_pagina;
		}else{
			size_registro_pagina_actual = size_restante_registro;
		}
		
		t_direccion_registro* direccion_registro = direccion_registro_new(direccion_fisica,size_registro_pagina_actual);
		
		list_add(direcciones_registros->direcciones,direccion_registro);
		direccion_logica = direccion_logica + size_registro_pagina_actual;

		size_restante_registro = size_restante_registro - size_registro_pagina_actual; //Cantidad de bytes que restan consumir
		
	}
	return direcciones_registros;
} 

t_buffer* leer_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros,int conexion){
	
	int response;
	t_acceso_espacio_usuario* acceso_espacio_usuario;
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_pid_valor pid_size_total = direcciones_fisicas_registros->pid_size_total;
	uint32_t size_leido=0;
	uint32_t size_registro_pagina_actual;
    t_buffer* dato_final_puntero = crear_buffer(pid_size_total.valor);
	
	/////BORRAR
	int registro_reconstr;
    void* registro_puntero_recons = &registro_reconstr;
	/////BORRAR


	void _enviar_direcciones_memoria(void* element){	
			t_direccion_registro* direccion_registro = (t_direccion_registro*) element;
			size_registro_pagina_actual = direccion_registro->size_registro_pagina;
			
			
			acceso_espacio_usuario =  acceso_espacio_usuario_create(
			pid_size_total.PID,
			direccion_registro->direccion_fisica,
			direccion_registro->size_registro_pagina,
			NULL);		
			enviar_acceso_espacio_usuario(acceso_espacio_usuario,LECTURA_MEMORIA,conexion);
			
			free(acceso_espacio_usuario);
			response = recibir_operacion(conexion);
				
		//	if(response == VALOR_LECTURA_MEMORIA){
				
				void* dato_recibido = recibir_buffer(&size_registro_pagina_actual,conexion);		

				memcpy(dato_final_puntero->stream + size_leido,dato_recibido, size_registro_pagina_actual);
				
				/////BORRAR
				memcpy(registro_puntero_recons + size_leido, dato_recibido ,size_registro_pagina_actual);
				/////BORRAR
				
				size_leido += size_registro_pagina_actual;
	
				
			//	loguear("PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>",
		   // pid_size_total.PID,direccion_registro->direccion_fisica,dato_recibido);
			free(dato_recibido);
			//}
			
			
		}
		list_iterate(direcciones_registros, &_enviar_direcciones_memoria);
		/////BORRAR
		//loguear("Valor leido: <%d>",registro_reconstr);
		/////BORRAR
	return dato_final_puntero;
}

void escribir_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion){
	int operacion_ok;
	t_acceso_espacio_usuario* acceso_espacio_usuario;
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_pid_valor pid_size_total = direcciones_fisicas_registros->pid_size_total;
	uint32_t size_leido=0;
	uint32_t size_registro_pagina_actual;
    uint32_t registro_int = atoi(registro_dato);
    void* registro_puntero = &registro_int;
	int registro_reconstr;
    void* registro_puntero_recons = &registro_reconstr;
 

		void _enviar_direcciones_memoria(void* element){
		
			t_direccion_registro* direccion_registro = (t_direccion_registro*) element;
			size_registro_pagina_actual = direccion_registro->size_registro_pagina;
			
			void* dato_parcial = malloc(size_registro_pagina_actual);

			memcpy(dato_parcial, registro_puntero + size_leido,size_registro_pagina_actual);
			
			memcpy(registro_puntero_recons + size_leido, dato_parcial ,size_registro_pagina_actual);

			acceso_espacio_usuario =  acceso_espacio_usuario_create(
			pid_size_total.PID,
			direccion_registro->direccion_fisica,
			direccion_registro-> size_registro_pagina,
			dato_parcial);
			
			
			enviar_acceso_espacio_usuario(acceso_espacio_usuario,ESCRITURA_MEMORIA,conexion);
			size_leido += size_registro_pagina_actual;	

			operacion_ok = recibir_operacion(conexion);
			
			if(operacion_ok==MOV_OUT_OK){
				char* valor_memoria =recibir_mensaje(conexion);
				free(valor_memoria);
			}
			
			//loguear("PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>",
			//pid_size_total.PID,direccion_registro->direccion_fisica,registro_dato + size_leido);
			free(acceso_espacio_usuario);
			free(dato_parcial);
			
			
		};
		
	list_iterate(direcciones_registros, &_enviar_direcciones_memoria);

	loguear("Valor escrito: <%d>",registro_reconstr);


		
}

uint32_t mmu (t_pcb* pcb, uint32_t direccion_logica){
	uint32_t direccion_fisica;
	uint32_t numero_pagina = obtener_numero_pagina(direccion_logica);
	uint32_t desplazamiento = obtener_desplazamiento(direccion_logica,numero_pagina);
	int registro_tlb = tlb_hit(pcb->PID,numero_pagina);
	

	if(registro_tlb != -1){
		loguear("PID: <%d> - TLB HIT - Pagina: <%d>",pcb->PID,numero_pagina);
		direccion_fisica = tamanio_pagina * (uint32_t)registro_tlb + desplazamiento;
		return direccion_fisica;
	}
	loguear("PID: <%d> - TLB MISS - Pagina: <%d>",pcb->PID,numero_pagina);
	char* nro_frame = string_new();
	// Agrego el pcb y le paso un proceso a la memoria para que pueda encontrar que proceso le está pidiendo la CPU
	
	
	t_pid_valor* pid_nro_pagina= pid_value_create(pcb->PID,numero_pagina); //Vamos con esta conversion?
	// FALTA LOGICA SEGUIR PIDIENDO FRAMES HASTA TENER UN FIN DE CADENA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Haciendo esta reutilización, capaz hay que cambiar el nombre de tamanio. Acá en tamanio le estamos pasando una página
	
	//Start For -> Devolver paquete 
	
	enviar_pid_value(pid_nro_pagina,ACCESO_TABLA_PAGINAS,conexion_memoria);
	

	int response = recibir_operacion(conexion_memoria);
	if(response == RESPUESTA_NRO_FRAME)
		nro_frame = recibir_mensaje(conexion_memoria);
	loguear("PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%s>",
	pcb->PID,
	numero_pagina,
	nro_frame
	);
	
	direccion_fisica = tamanio_pagina * (uint32_t)atoi(nro_frame) + desplazamiento;
	actualizar_tlb(pcb->PID,numero_pagina,(uint32_t)atoi(nro_frame));

	// End for
	return direccion_fisica;
}



uint32_t calcular_bytes_restantes(uint32_t direcc_fisica){
	int inicio_pagina = direcc_fisica/tamanio_pagina;
	if (inicio_pagina == 0){  // Si estamos en la pagina 0, la cuenta debe ser distinta
		return tamanio_pagina - direcc_fisica;
	}
	// Si no es la primera, debemos ubicarnos en la pagina siguiente a la que debemos leer
	// Entonces la direccion de la pagina siguiente menos la direccion desde donde arrancamos a leer nos va a dar 
	// la cantidad de bytes que debemos leer a partir de esa direccion fisica
	return (inicio_pagina+1)*tamanio_pagina - direcc_fisica;
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
			case DIRECCIONES_PROCESO:
			  	t_direcciones_proceso *dir_proceso = recibir_direcciones_proceso(paquete); 
				loguear_direccion_proceso(dir_proceso);
				direcciones_proceso_destroy(dir_proceso);
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
/*
t_id_valor* generar_direccion_registro(uint32_t direccion_fisica,uint32_t size_registro_pagina){
	t_id_valor* direccion_registro = malloc(sizeof(t_id_valor));
	direccion_registro->direccion_fisica = direccion_fisica;
	direccion_registro->size_registro_pagina = size_registro_pagina;
	return direccion_registro;
}
*/