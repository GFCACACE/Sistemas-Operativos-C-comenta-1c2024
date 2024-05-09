#include "cpu.h"

int kernel_dispatch, dispatch, interrupt, kernel_interrupt, conexion_memoria;
int cod_op_kernel_dispatch;
int cod_op_kernel_interrupt;
char *IR, *INSTID;
t_param PARAM1, PARAM2, PARAM3;
pthread_t *mutex_interrupt;
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

	return diccionario;
}

bool iniciar_registros_cpu()
{
	/*
	registros_cpu = malloc(sizeof(t_registros_cpu));
	registros_cpu->AX=malloc(sizeof(uint8_t));
	registros_cpu->BX=malloc(sizeof(uint8_t));
	registros_cpu->CX=malloc(sizeof(uint8_t));
	registros_cpu->DX=malloc(sizeof(uint8_t));
	registros_cpu->EAX = malloc(sizeof(uint32_t));
	registros_cpu->EBX = malloc(sizeof(uint32_t));
	registros_cpu->ECX = malloc(sizeof(uint32_t));
	registros_cpu->EDX = malloc(sizeof(uint32_t));
	registros_cpu->SI = malloc(sizeof(uint32_t));
	registros_cpu->DI = malloc(sizeof(uint32_t));*/
	// Si implementamos la verison que hariamos en las commons, todo lo de arriba se iria reemplazado por
	loguear("CPU inicializará registros");
	registros_cpu = inicializar_registros(registros_cpu);

	loguear("Registros CPU logueados");

	// Esto quedaria dentro de la funcion iniciar_registros_cpu
	IR = string_new();
	INSTID = string_new();
	// PARAM1 = malloc(sizeof(t_param));
	// PARAM2 = malloc(sizeof(t_param));
	// PARAM3 = malloc(sizeof(t_param));


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
		   // iniciar_dispatch() &&
		   iniciar_conexion_memoria() &&
		   // iniciar_conexion_kernel() &&
		   iniciar_variables();
}
bool iniciar_variables()
{
	cod_op_kernel_interrupt = EJECUTAR_CPU;
	// pthread_mutex_init(mutex_interrupt, NULL);
	return true;
}
void config_destroy_cpu(t_config_cpu *config)
{

	config_destroy(config->config);
	free(config);
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

void ejecutar_instruccion(t_pcb *pcb, char *instruccion)
{

	loguear("Ejecutando instrucción: %s ...", instruccion);
	// HACER INSTRUCCION
	pcb->program_counter++;
}

bool es_exit(char *comando)
{
	return string_equals_ignore_case(comando, (char *)EXIT_PROGRAM);
}

 void ciclo_de_instruccion(t_pcb* pcb){
	bool flag_interrupt = false;
	do{		
		fetch(pcb);
		decode();
		execute(pcb);
		flag_interrupt=check_interrupt(pcb);

	}while (es_exit(IR)==false && flag_interrupt==false);
	enviar_texto("fin",FIN_PROGRAMA,conexion_memoria);
	if(IR!=NULL) free(IR);	
 }




t_param interpretar_valor_instruccion(char* valor){
	t_param parametro;
	if(dictionary_has_key(diccionario_registros_cpu,valor)==true){
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
	// TODO;	actualizar_pcb(pcb); //sincronizar registros cpu con pcb
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
	if (!strcmp(INSTID, "EXIT"))
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
		exe_io_gen_sleep(PARAM1, PARAM2);

		actualizar_contexto(pcb);
		liberar_param(PARAM1);
		liberar_param(PARAM2);
		return true;
	}
	if (!strcmp(INSTID, "EXIT"))
	{
		loguear("PID: <%d> - Ejecutando: <%s>", pcb->PID, INSTID);
		exe_exit(pcb);
		loguear("Saliendo...");
		return true;
	}

	return false;
}


void liberar_param(t_param parametro){ // Para agregar declaratividad
	free(parametro.string_valor);
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

bool exe_io_gen_sleep(t_param interfaz, t_param unidades_de_trabajo)
{
	(uint32_t)registros_cpu->PC++;
	return true;
}

bool exe_exit(t_pcb *pcb)
{
	// devolver_contexto(pcb);
	return true;
}

bool check_interrupt(t_pcb *pcb)
{
	/*cod_op_interrupt debería ser modificada por un thread
	 dedicado a recibir de kernel si desea interrumpir*/

	// enviar a mensaje a kernel para que decremente el quantum
	if (cod_op_kernel_interrupt == INTERRUMPIR_CPU)
	{
		// devolver_contexto(pcb);
		return true;
	}
	return false;
}
bool devolver_contexto(t_pcb *pcb)
{
	// el pcb Siempre debe devolverse por dispatch
	if (strcmp(INSTID, "EXIT"))
		enviar_pcb(pcb, CPU_EXIT, dispatch);
	else
		enviar_pcb(pcb, CPU_INTERRUPT, dispatch);

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
	*registros_cpu = *pcb->registros_cpu;
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
		loguear("Cod op: %d", cod_op);
        switch (cod_op) {
            case EJECUTAR_PROCESO:
                t_pcb *pcb = recibir_pcb(paquete); 
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