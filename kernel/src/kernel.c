#include "kernel.h"

int grado_multiprogamacion_actual;
int conexion_memoria, cpu_dispatch,cpu_interrupt;
int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
t_config_kernel* config;
t_dictionary * comandos_consola;
t_queue* estado_new, *estado_ready, *estado_blocked, *estado_exit, *estado_ready_plus, *estado_exec;
t_pcb* pcb_exec;  //CAMBIAR DE COLA A UNICO PCB POST MODIFICAR LAS FUNCIONES QUE IMPLEMENTO JOACO


t_planificador get_algoritmo(char* nombre){


	 // Crear el diccionario de algoritmos
    t_dictionary *algoritmos = dictionary_create();
	t_planificador planificador;

    // Función para agregar un algoritmo al diccionario
    void _agregar(char* _nombre, t_alg_planificador tipo,void(*funcion)(void*)){
		t_planificador* planif = malloc(sizeof(t_planificador));
		planif->id = tipo;
		planif->planificar = funcion; 		
		//t_planificador* planif_ptr=*planif;
        dictionary_put(algoritmos, _nombre, planif);
    };

	_agregar("FIFO",FIFO,&planificacion_FIFO);
	_agregar("RR",RR,&planificacion_RR);
	_agregar("VRR",VRR,&planificacion_VRR);

	t_planificador* planificador_ptr= (t_planificador*)dictionary_get(algoritmos, nombre);
	if(planificador_ptr==NULL)	
	{	
		perror("Algoritmo de planificación no válido");

		exit(EXIT_FAILURE);
	}
	else planificador=*planificador_ptr;
	
	dictionary_destroy_and_destroy_elements(algoritmos,free);
	return planificador;

}

t_config_kernel* iniciar_config_kernel(char* path_config){
	t_config* _config = config_create(path_config);
	if(_config ==NULL)
		return NULL;
	t_config_kernel* config_kernel = malloc(sizeof(t_config_kernel));	

	config_kernel->PUERTO_ESCUCHA= config_get_int_value(_config,"PUERTO_ESCUCHA");
	config_kernel->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
	config_kernel->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
	config_kernel->IP_CPU = config_get_string_value(_config,"IP_CPU");
	config_kernel->PUERTO_CPU_DISPATCH = config_get_int_value(_config,"PUERTO_CPU_DISPATCH");
	config_kernel->PUERTO_CPU_INTERRUPT = config_get_int_value(_config,"PUERTO_CPU_INTERRUPT");
	config_kernel->ALGORITMO_PLANIFICACION = get_algoritmo(config_get_string_value(_config,"ALGORITMO_PLANIFICACION"));
	config_kernel->QUANTUM = config_get_int_value(_config,"QUANTUM");
	config_kernel->RECURSOS = config_get_array_value(_config,"RECURSOS");
	config_kernel->INSTANCIAS_RECURSOS = config_get_array_value(_config,"INSTANCIAS_RECURSOS");
	config_kernel->GRADO_MULTIPROGRAMACION = config_get_int_value(_config,"GRADO_MULTIPROGRAMACION_INI");
	config_kernel->PATH_SCRIPTS = config_get_string_value(_config,"PATH_SCRIPTS");
	config_kernel->config = _config;
	if(config_kernel->ALGORITMO_PLANIFICACION.planificar ==NULL)
		return NULL;
	return config_kernel;
}

bool iniciar_logger_config(char* path_config){
	decir_hola(MODULO);
    logger = iniciar_logger(MODULO);
	if(logger == NULL) printf("EL LOGGER NO PUDO SER INICIADO.\n");
	config = iniciar_config_kernel(path_config);
	if(config == NULL) {
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}
	loguear_config();	
	return true;
}

bool inicializar_comandos(){
	comandos_consola  =  dictionary_create();
    agregar_comando(EJECUTAR_SCRIPT,"EJECUTAR_SCRIPT","[PATH]",&ejecutar_scripts_de_archivo);
    agregar_comando(INICIAR_PROCESO,"INICIAR_PROCESO","[PATH] [SIZE] [PRIORIDAD]",&iniciar_proceso);
	agregar_comando(FINALIZAR_PROCESO,"FINALIZAR_PROCESO","[PID]",&finalizar_proceso);
    agregar_comando(DETENER_PLANIFICACION,"DETENER_PLANIFICACION","[]",&iniciar_planificacion);
	agregar_comando(INICIAR_PLANIFICACION,"INICIAR_PLANIFICACION","[]",&multiprogramacion);
    agregar_comando(MULTIPROGRAMACION,"MULTIPROGRAMACION","[VALOR]",&detener_planificacion);
	agregar_comando(PROCESO_ESTADO,"PROCESO_ESTADO","[]",&proceso_estado);
    agregar_comando(EXIT,"EXIT","[]",&finalizar_consola);

	return true;
}

bool iniciar_conexion_memoria(){
	  conexion_memoria = crear_conexion(config->IP_MEMORIA,config->PUERTO_MEMORIA);
	if(conexion_memoria ==-1){
		
		loguear_error("No se pudo conectar memoria");
		return false;
	} 
	return true;
}

bool iniciar_dispatch(){
	cpu_dispatch = crear_conexion(config->IP_CPU, config->PUERTO_CPU_DISPATCH);
	if(cpu_dispatch ==-1){
		
		loguear_error("No se pudo conectar cpu (dispatch)");
		return false;
	} 
	return true;
}

bool iniciar_interrupt(){
	 cpu_interrupt = crear_conexion(config->IP_CPU, config->PUERTO_CPU_INTERRUPT);
	if(cpu_interrupt ==-1){	
		loguear_error("No se pudo conectar cpu (interrupt)");
		return false;
	} 
	return true;
}

bool iniciar_colas_entrada_salida(){
	t_queue* io_stdin = queue_create();
	t_queue* io_stdout = queue_create();
	t_queue* io_generica = queue_create();
	t_queue* io_dialfs = queue_create();
	///VALIDAR? SON COMMONS!!!!!!
	return true;
}

bool iniciar_estados_planificacion(){

	estado_new = queue_create();
	estado_ready = queue_create();
	estado_blocked = queue_create();
	estado_exit = queue_create();
	estado_exec = queue_create();
	if(es_vrr())
		estado_ready_plus = queue_create();
	
	//VALIDAR? SON COMMONS!!!!!!
	return true;
}



bool iniciar_kernel(char* path_config){
	return
	iniciar_logger_config(path_config)&&
	inicializar_comandos()&&
	iniciar_conexion_memoria()&&
	iniciar_dispatch()&&
	iniciar_interrupt()&&
	iniciar_estados_planificacion()&&
	iniciar_colas_entrada_salida();
	//iniciar_planificadores();
}

bool iniciar_planificadores(){
	pthread_t thread_planificador_largo;
	pthread_t thread_planificador_corto;//Inicializo el thread

	pthread_create(&thread_planificador_largo,NULL, planificador_largo,NULL);
	pthread_create(&thread_planificador_corto,NULL,planificador_corto,NULL);

	pthread_detach(thread_planificador_largo);
	if (thread_planificador_largo == -1){
		loguear_error("No se pudo iniciar el planificador de largo plazo.");
		return false;
	}
	pthread_detach(thread_planificador_corto);
	if (thread_planificador_corto == -1){
		loguear_error("No se pudo iniciar el planificador de corto plazo.");
		return false;
	}
	return true;
}

void planificador_largo(){
	loguear("Se inicio el planificador largo.");
	
	
}

/*uint32_t grado_multiprogamacion_actual(){
	uint32_t grado;
	grado = list_size(estado_ready->elements) + list_size(estado_blocked->elements);
	if (pcb_exec != NULL){
		grado++;
	}
}*/

void planificador_corto(){
		loguear("Se inicio el planificador corto.");

}

void config_kernel_destroy(t_config_kernel* config){

	string_array_destroy(config->RECURSOS);
	string_array_destroy(config->INSTANCIAS_RECURSOS);
	config_destroy(config->config);
	free(config);
}

void loguear_config(){

	loguear("IP_MEMORIA: %s",config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d",config->PUERTO_MEMORIA);
	loguear("QUANTUM: %d",config->QUANTUM);
	loguear("IP_CPU: %s",config->IP_CPU);
	loguear("PUERTO DISPATCH: %d",config->PUERTO_CPU_DISPATCH);
	loguear("PUERTO INTERRUPT: %d",config->PUERTO_CPU_DISPATCH);
	loguear("CANT. RECURSOS: %d", string_array_size(config->RECURSOS));
}


char* leer_texto_consola(){
	
	return readline(">");

}

void agregar_comando(op_code_kernel code,char* nombre,char* params,bool(*funcion)(char**)){
    
    t_comando_consola* comando = malloc(sizeof(t_comando_consola));
    comando->comando = code;
    comando->parametros = params;
    comando->funcion = funcion;
	comando->nombre= nombre;

    void _agregar_comando_(char* texto){
        dictionary_put(comandos_consola,texto,comando);
    }

    _agregar_comando_(string_itoa(code));
    _agregar_comando_(nombre);

}




void imprimir_valores_leidos(char** substrings){

	int index=0;
	void imprimir_valor(char* leido){
		printf("substring[%d] vale:%s\n",index++,leido);
	}
	string_iterate_lines(substrings,imprimir_valor);
}


bool parametros_ejecutar_script_validos(char** parametros){
	bool validado = string_array_size(parametros)==2;
	
	if(!validado)
		printf("\tEJECUTAR_SCRIPT debe recibir 1 parámetro:\n\tPath (string)\n");

	return validado;
}



t_list* get_instrucciones_kernel(char* archivo){
	return get_instrucciones(config->PATH_SCRIPTS,archivo);
}

void ejecutar_sript(void* script){
	
	printf("Ejecutando script...%s\n",(char*)script);
	ejecutar_comando_consola((char*)script);
}

bool ejecutar_scripts_de_archivo(char** parametros){
	
	imprimir_valores_leidos(parametros);

	if(!parametros_ejecutar_script_validos(parametros))
		return false;

	char *path = string_duplicate(parametros[1]);

	t_list* instrucciones_sript = get_instrucciones_kernel(path);
	free(path);

	if(instrucciones_sript!=NULL)
	list_iterate(instrucciones_sript,ejecutar_sript);
	
	return true;

}

bool existe_comando(char* comando){
   return (dictionary_has_key(comandos_consola,comando));
}
int ejecutar_comando_consola(char*params){

	char** parametros = string_split(params," ");  
	char* comando = parametros[0]; 
	string_to_upper(comando);
	t_comando_consola* comando_consola = NULL;
	int numero_comando = NULL;
	if(existe_comando(comando)){
		comando_consola = dictionary_get(comandos_consola,comando);
		numero_comando = comando_consola->comando;
		if(comando_consola->comando != EXIT){
       		comando_consola->funcion(parametros);
		}
	}
	free(parametros);
	return numero_comando;
}


bool iniciar_proceso(char** parametros){

	bool parametros_iniciar_proceso_validos(char** parametros){
		bool validado = 
		string_array_size(parametros)==4 &&
		is_numeric(parametros[2]) &&
		is_numeric(parametros[3]);
		
		if(!validado)
			printf("\tINICIAR_PROCESO debe recibir 3 parámetros:\n\tPath (string)\n\tSize (int)\n\tPrioridad (int)\n");

		return validado;
	}

	loguear("iniciando proceso...");
	imprimir_valores_leidos(parametros);	

	if(!parametros_iniciar_proceso_validos(parametros))
	return false;
	
	char *path = string_duplicate(parametros[1]);
	loguear("PATH: %s",path);
	t_pcb* pcb = pcb_create(path);   // Se crea el PCB y se agrega a New
	queue_push(estado_new,pcb);
	free(path);
		
	return true;
}

void planificador_largo_plazo(){

}


bool finalizar_proceso(char** substrings){	
		imprimir_valores_leidos(substrings);

		loguear("Finaliza el proceso <PID> - Motivo: Finalizado por consola");

		return true;
}



void listar_comandos(){
	printf("Los comandos disponibles son:\n");
	char* decoracion= "\t\t*****************************************************\n";
	printf("%s",decoracion);
	int cantidad = dictionary_size(comandos_consola)/2;


	for (int i = 0; i < cantidad; i++) {
		t_comando_consola* comando = dictionary_get(comandos_consola,string_itoa(i));
        printf("\t%d. %s %s\n",i ,comando->nombre ,comando->parametros);
    }
	printf("%s",decoracion);
}

bool iniciar_planificacion(char** substrings){
	
	return true;

}

bool multiprogramacion(char** substrings){

	loguear("El grado de multiprogramación anterior es: %d",config->GRADO_MULTIPROGRAMACION);
	if(string_array_size(substrings)>0){
		char* valor = substrings[1];
		if(is_numeric(valor)){
			char* endptr; // Used to check for conversion errors
			int number;
			number = strtol(valor, &endptr, 10);
			config->GRADO_MULTIPROGRAMACION = number;
			loguear("El nuevo grado de multiprogramación es: %d",config->GRADO_MULTIPROGRAMACION);
		}
		else loguear("El valor: %s no es un grado de multiprogramación válido.",valor );
	}
	else{
		loguear("No se especificó ningún grado de multiprogramación");
	}
	
	return true;

}
bool detener_planificacion(char** substrings){return true;}

void imprimir_cola(t_queue *cola, const char *estado) {

	void imprimir_estado(void* elem){
		t_pcb* pcb = (t_pcb*)elem;
		 printf("| %-10d | %-20s | %-15s |\n", pcb->PID,pcb->path ,estado);
	}
    printf("Estado: %s\n", estado);
    printf("| %-10s | %-20s | %-15s |\n", "PID", "Nombre", "Estado");
    printf("|------------|----------------------|-----------------|\n");

	list_iterate(cola->elements,imprimir_estado);

    printf("|------------|----------------------|-----------------|\n");
}

bool es_vrr(){
	return config->ALGORITMO_PLANIFICACION.id == VRR;
}

bool proceso_estado(){
	
	imprimir_cola(estado_new, "Nuevo");
    imprimir_cola(estado_ready, "Listo");
    imprimir_cola(estado_blocked, "Suspendido");
	imprimir_cola(estado_exec, "Ejecutando");
	imprimir_cola(estado_exit, "Finalizado");
	if( es_vrr())
	imprimir_cola(estado_ready_plus,"Listo VRR");
	
	return true;
}


bool finalizar_consola(char** parametros){
	loguear("Consola finalizada.");	
	return false;
}

void iniciar_consola(){
	char *cadenaLeida;
	int comando = NULL;
	 while (comando == NULL || comando != EXIT) {
		listar_comandos();
        cadenaLeida =  leer_texto_consola();	
		comando = ejecutar_comando_consola(cadenaLeida);
		free(cadenaLeida);
    }
	

}

void ejecutar_planificacion(){
	config->ALGORITMO_PLANIFICACION.planificar();
}

void planificacion_FIFO(){
	loguear("Planificando por FIFO");
	t_pcb* pcb = (t_pcb*)queue_pop(estado_ready);
	if(pcb!=NULL)
	{	pcb_exec = pcb;
		loguear_pcb(pcb_exec);
	}

};
void planificacion_RR(){
	loguear("Planificando por Round Robbin");
	// if(pcb_exec->quantum==0)
	// 	{
	// 		t_pcb* pcb = list_get_minimum(estado_ready.el)
	// 	}

}
void planificacion_VRR(){
	loguear("Planificando por Virtual Round Robbin");
}


void config_destroy_kernel(t_config_kernel * config){
	config_destroy(config->config);
	string_array_destroy(config->INSTANCIAS_RECURSOS);
	string_array_destroy(config->RECURSOS);
	free(config);
}

void liberar_colas(){
	void liberar_cola(t_queue* cola){
		if(cola!=NULL)
			queue_destroy(cola);
	};

	liberar_cola(estado_blocked);
	liberar_cola(estado_exec);
	liberar_cola(estado_exit);
	liberar_cola(estado_new);
	liberar_cola(estado_ready);
	liberar_cola(estado_ready_plus);
	
}

void finalizar_kernel(){
	
	if (conexion_memoria != -1) liberar_conexion(conexion_memoria);
	if (cpu_dispatch != -1) liberar_conexion(cpu_dispatch);
	if (cpu_interrupt != -1) liberar_conexion(cpu_interrupt);
	if(config!=NULL) config_destroy_kernel(config);
	if(logger!=NULL) log_destroy(logger);
	if(comandos_consola!=NULL) dictionary_destroy(comandos_consola);
	liberar_colas();
}