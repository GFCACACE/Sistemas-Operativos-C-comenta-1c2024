#include "kernel.h"

int conexion_memoria, cpu_dispatch,cpu_interrupt;
int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
t_config_kernel* config;
t_list* comandos_consola;

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

bool iniciar_kernel(char* path_config){
	decir_hola(MODULO);
    logger = iniciar_logger(MODULO);
	if(logger == NULL) printf("EL LOGGER NO PUDO SER INICIADO.\n");
	config = iniciar_config_kernel(path_config);
	if(config == NULL) {
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}
	loguear_config();	    
	comandos_consola = get_comandos();
   conexion_memoria = crear_conexion(config->IP_MEMORIA,config->PUERTO_MEMORIA);
	if(conexion_memoria ==-1){
		
		loguear_error("No se pudo conectar memoria");
		return false;
	} 
    cpu_dispatch = crear_conexion(config->IP_CPU, config->PUERTO_CPU_DISPATCH);
	if(cpu_dispatch ==-1){
		
		loguear_error("No se pudo conectar cpu (dispatch)");
		return false;
	} 
    cpu_interrupt = crear_conexion(config->IP_CPU, config->PUERTO_CPU_INTERRUPT);
	if(cpu_interrupt ==-1){
		
		loguear_error("No se pudo conectar cpu (interrupt)");
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

char * codigo_op_a_texto( op_code_kernel codigo ) {

	switch ( codigo ) {
		case EJECUTAR_SCRIPT:       return "EJECUTAR_SCRIPT"; break;
		case INICIAR_PROCESO:       return "INICIAR_PROCESO"; break;
		case FINALIZAR_PROCESO: 	return "FINALIZAR_PROCESO"; break;
		case DETENER_PLANIFICACION: return "DETENER_PLANIFICACION"; break;
		case INICIAR_PLANIFICACION: return "INICIAR_PLANIFICACION"; break;
		case MULTIPROGRAMACION:		return "MULTIPROGRAMACION"; break;
		case PROCESO_ESTADO:   		return "PROCESO_ESTADO"; break;
		case EXIT:   				return "EXIT"; break;
		default:          			return NULL; break;
	}

}
t_list* get_comandos(){
	  t_list* comandos = list_create();

    for (op_code_kernel i = EJECUTAR_SCRIPT; i <= EXIT; i++) {
        op_code_kernel *code_ptr = malloc(sizeof(op_code_kernel));
        *code_ptr = i;
        list_add(comandos, code_ptr);
    }
	return comandos;
}


op_code_kernel* codigo_comando(char* comando){
/*
	if ( es_comando(comando)) return atoi(comando); else
	return EXIT_FAILURE;*/

	bool es_comando_del_texto(void* codigo){
		char* tex= codigo_op_a_texto(*(op_code_kernel*)codigo);
		return tex!=NULL && string_equals_ignore_case(tex,comando);
	}
	bool es_comando_del_numero(void* codigo){
		return is_numeric(comando) && (atoi(comando)== *(op_code_kernel*)codigo) ;
	}
	bool _es_el_comando(void* codigo){
		return es_comando_del_numero(codigo) || es_comando_del_texto(codigo); 
	}

	return  (op_code_kernel*)list_find(comandos_consola,_es_el_comando);
}



bool es_comando(char* comando){

	op_code_kernel* cod = codigo_comando(comando);
	return cod >0;
}

char * parametros_segun_comando( op_code_kernel codigo ) {

	switch ( codigo ) {
		case EJECUTAR_SCRIPT:		return "[PATH]"; break;
		case INICIAR_PROCESO:       return "[PATH] [SIZE] [PRIORIDAD]"; break;
		case FINALIZAR_PROCESO: 	return "[PID]"; break;
		case DETENER_PLANIFICACION: return "[]"; break;
		case INICIAR_PLANIFICACION: return "[]"; break;
		case MULTIPROGRAMACION:		return "[VALOR]"; break;
		case PROCESO_ESTADO:   		return "[]"; break;
		case EXIT:   				return "[]"; break;
		default:          			return NULL; break;
	}

}


void imprimir_valores_leidos(char** substrings){

	int index=0;
	void imprimir_valor(char* leido){
		printf("substring[%d] vale:%s\n",index++,leido);
	}
	string_iterate_lines(substrings,imprimir_valor);
}

bool parametros_iniciar_proceso_validos(char** parametros){
	bool validado = 
	string_array_size(parametros)==4 &&
	is_numeric(parametros[2]) &&
	is_numeric(parametros[3]);
	
	if(!validado)
		printf("\tINICIAR_PROCESO debe recibir 3 parámetros:\n\tPath (string)\n\tSize (int)\n\tPrioridad (int)\n");

	return validado;
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
	printf("Script: %s", (char*)script);
}
void ejecutar_scripts_de_archivo(char** parametros){
	loguear("Ejecutando script...");
	imprimir_valores_leidos(parametros);

	if(!parametros_ejecutar_script_validos(parametros))
	return;

	char *path = string_duplicate(parametros[1]);

	t_list* instrucciones_sript = get_instrucciones_kernel(path);
	free(path);

	if(instrucciones_sript!=NULL)
	list_iterate(instrucciones_sript,ejecutar_sript);

}

void iniciar_proceso(char** parametros){

 		loguear("iniciando proceso...");
		imprimir_valores_leidos(parametros);	

		if(!parametros_iniciar_proceso_validos(parametros))
		return;

		char *path = string_duplicate(parametros[1]);//malloc(sizeof(parametros[1]));
		//strcpy(path, parametros[1]);
		 loguear("PATH: %s",path);
		 free(path);
	
}

void finalizar_proceso(char** substrings){	
		imprimir_valores_leidos(substrings);

		loguear("Finaliza el proceso <PID> - Motivo: Finalizado por consola");
}



void listar_comandos(){
	printf("Los comandos disponibles son:\n");
	printf("\t\t*****************************************************\n");

	for (int i = 0; i <=EXIT; i++) {
        printf("\t%d. %s %s\n",i ,codigo_op_a_texto(i),parametros_segun_comando(i));
    }
	printf("\t\t*****************************************************\n");
}

void iniciar_planificacion(char** substrings){
	
	

}

void multiprogramacion(char** substrings){

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
	

}
void detener_planificacion(char** substrings){}
void proceso_estado(char** substrings){}

void consola(){
	char *cadenaLeida;
	void _liberar(char*cadena,char**params){
		string_array_destroy(params);
		free(cadena);		
	}
	void _finalizar_consola(){
		
		loguear("Consola finalizada.");	
	}
	listar_comandos();
	 while (1) {	
		
		char** parametros;    
        cadenaLeida =  leer_texto_consola();	

        if (!cadenaLeida)                  
			break;                                                     
        

		parametros = string_split(cadenaLeida," ");    
		op_code_kernel* codigo_operacion = codigo_comando(parametros[0]);                
        
		switch( *codigo_operacion) {
			case EJECUTAR_SCRIPT: ejecutar_scripts_de_archivo(parametros);break;
			case INICIAR_PROCESO: iniciar_proceso(parametros); break;
			case FINALIZAR_PROCESO: finalizar_proceso(parametros); break;
			case INICIAR_PLANIFICACION: iniciar_planificacion(parametros); break;
			case MULTIPROGRAMACION: multiprogramacion(parametros);break;
			case DETENER_PLANIFICACION: detener_planificacion(parametros); break;
			case PROCESO_ESTADO: proceso_estado(parametros); break;
			case EXIT:  _liberar(cadenaLeida,parametros);
						_finalizar_consola();
						return;break;
			default: listar_comandos();break;

		}
		_liberar(cadenaLeida,parametros);
    }
	_finalizar_consola();

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
	if(comandos_consola!=NULL) list_destroy(comandos_consola);
}