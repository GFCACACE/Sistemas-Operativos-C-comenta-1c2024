#include "kernel.h"


t_queue* estado_new;
int conexion_memoria, cpu_dispatch,cpu_interrupt;
int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
t_config_kernel* config;
t_dictionary * comandos_consola;

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
	config_kernel->ALGORITMO_PLANIFICACION = config_get_string_value(_config,"ALGORITMO_PLANIFICACION");
	config_kernel->QUANTUM = config_get_int_value(_config,"QUANTUM");
	config_kernel->RECURSOS = config_get_array_value(_config,"RECURSOS");
	config_kernel->INSTANCIAS_RECURSOS = config_get_array_value(_config,"INSTANCIAS_RECURSOS");
	config_kernel->GRADO_MULTIPROGRAMACION = config_get_int_value(_config,"GRADO_MULTIPROGRAMACION_INI");
	config_kernel->PATH_SCRIPTS = config_get_string_value(_config,"PATH_SCRIPTS");
	config_kernel->config = _config;

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

	t_queue* estado_new = queue_create();
	t_queue* estado_ready = queue_create();
	t_queue* estado_blocked = queue_create();
	t_queue* estado_exit = queue_create();
	if(config->ALGORITMO_PLANIFICACION == VRR){
		t_queue* estado_ready_plus = queue_create();
	}
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
	//iniciar_consola();
}

bool iniciar_consola(){
	pthread_t thread_consola; //Inicializo el thread
	pthread_create(&thread_consola,NULL,consola,NULL);
	pthread_detach(thread_consola);
	if (thread_consola == -1){
		loguear_error("No se pudo iniciar la consola");
		return false;
	}
	return true;
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


bool existe_comando(char* comando){
   return (dictionary_has_key(comandos_consola,comando));
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
	
	printf("Ejecutando script...%s",(char*)script);
	ejecutar_comando_consola((char*)script);
}

bool ejecutar_scripts_de_archivo(char** parametros){
	loguear("Ejecutando script...");
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
bool ejecutar_comando_consola(char*params){

	char** parametros = string_split(params," ");  
	char* comando = parametros[0]; 
	string_to_upper(comando);
	bool existe = existe_comando(comando); 
	bool ejecutado = false;
		 if(existe)
			{
				t_comando_consola* comando_consola = dictionary_get(comandos_consola,comando);
				if(comando_consola->comando!=EXIT)
       				ejecutado = comando_consola->funcion(parametros);
			}
			else listar_comandos();
	free(parametros);
	return ejecutado;
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
	
	char *path = string_duplicate(parametros[1]);//malloc(sizeof(parametros[1]));
	//strcpy(path, parametros[1]);
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
bool proceso_estado(char** substrings){return true;}
bool finalizar_consola(char** parametros){
	loguear("Consola finalizada.");	
	return false;}

void consola(){
	char *cadenaLeida;
	listar_comandos();
	bool continuar=true;
	 while (continuar) {	
        cadenaLeida =  leer_texto_consola();	
        if (!cadenaLeida)                  
			break; 
		continuar = ejecutar_comando_consola(cadenaLeida);  		 		
		free(cadenaLeida);
    }	

}



void config_destroy_kernel(t_config_kernel * config){
	config_destroy(config->config);
	string_array_destroy(config->INSTANCIAS_RECURSOS);
	string_array_destroy(config->RECURSOS);
	free(config);
}

void finalizar_kernel(){
	
	if (conexion_memoria != -1) liberar_conexion(conexion_memoria);
	if (cpu_dispatch != -1) liberar_conexion(cpu_dispatch);
	if (cpu_interrupt != -1) liberar_conexion(cpu_interrupt);
	if(config!=NULL) config_destroy_kernel(config);
	if(logger!=NULL) log_destroy(logger);
	if(comandos_consola!=NULL) dictionary_destroy(comandos_consola);
}