#include "kernel.h"
#include "semaphore.h"
#include <time.h>

sem_t sem_cont_grado_mp;
sem_t sem_bin_new; //Sincroniza que plp (hilo new) no actúe hasta que haya un nuevo elemento en new
sem_t sem_bin_ready; //Sincroniza que pcp no actúe hasta que haya un nuevo elemento en ready
sem_t sem_bin_exit; //Sincroniza que plp (hilo exit) no actúe hasta que haya un nuevo elemento en exit
sem_t sem_bin_cpu_libre; // Sincroniza que no haya ningun PCB ejecutando en CPU

sem_t sem_bin_recibir_pcb;
sem_t sem_bin_plp_procesos_nuevos_iniciado,sem_bin_plp_procesos_finalizados_iniciado,sem_bin_planificador_corto_iniciado;

pthread_mutex_t mx_new = PTHREAD_MUTEX_INITIALIZER; // Garantiza mutua exclusion en estado_new. Podrían querer acceder consola y plp al mismo tiempo
pthread_mutex_t mx_ready = PTHREAD_MUTEX_INITIALIZER; //Garantiza mutua exclusion en estado_ready. Podrían querer acceder plp y pcp al mismo tiempo
pthread_mutex_t mx_ready_plus = PTHREAD_MUTEX_INITIALIZER; //Garantiza mutua exclusion en estado_ready. Podrían querer acceder plp y pcp al mismo tiempo
pthread_mutex_t mx_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_exit = PTHREAD_MUTEX_INITIALIZER; // Garantiza mutua exclusion en estado_exit. Podrían querer acceder consola, plp y pcp al mismo tiempo
pthread_mutex_t mx_pcb_exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_temp = PTHREAD_MUTEX_INITIALIZER;

time_t tiempo_inicial, tiempo_final;

int conexion_memoria, cpu_dispatch,cpu_interrupt, kernel_escucha, conexion_io;
int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
bool planificacion_detenida = false;
t_config_kernel* config;
t_dictionary * comandos_consola,*estados_dictionary,*estados_mutexes_dictionary, *diccionario_nombre_conexion, *diccionario_nombre_qblocked, *diccionario_conexion_qblocked;
t_queue* estado_new, *estado_ready, *estado_exit, *estado_ready_plus, *estado_temp,
		*io_stdin, *io_stdout, *io_generica, *io_dialfs;
//
t_blocked_interfaz* blocked_interfaz;
//
t_pcb* pcb_exec;
t_list* lista_interfaces_blocked;


 // Crear el diccionario de algoritmo
t_planificador get_algoritmo(char* nombre){


	 // Crear el diccionario de algoritmos
    t_dictionary *algoritmos = dictionary_create();
	t_planificador planificador;

    // Función para agregar un algoritmo al diccionario
    void _agregar(char* _nombre, t_alg_planificador tipo,void(*funcion)(void)){
		t_planificador* planif = malloc(sizeof(t_planificador));
		planif->id = tipo;
		planif->planificar = funcion; 	
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
    logger = iniciar_logger_(MODULO,0);
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
    agregar_comando(INICIAR_PROCESO,"INICIAR_PROCESO","[PATH]",&iniciar_proceso);
	agregar_comando(FINALIZAR_PROCESO,"FINALIZAR_PROCESO","[PID]",&finalizar_proceso);
    agregar_comando(DETENER_PLANIFICACION,"DETENER_PLANIFICACION","[]",&detener_planificacion);
	agregar_comando(INICIAR_PLANIFICACION,"INICIAR_PLANIFICACION","[]",&iniciar_planificacion);
    agregar_comando(MULTIPROGRAMACION,"MULTIPROGRAMACION","[VALOR]",&multiprogramacion);
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

// bool iniciar_colas_entrada_salida(){
// 	io_stdin = queue_create();
// 	io_stdout = queue_create();
// 	io_generica = queue_create();
// 	io_dialfs = queue_create();
// 	return true;
// }

bool iniciar_estados_planificacion(){

	estado_new = queue_create();
	estado_ready = queue_create();
	estado_exit = queue_create();	
	estado_temp = queue_create();	
	if(es_vrr())
		estado_ready_plus = queue_create();
	
	
	return true;
}



bool iniciar_kernel(char* path_config){
	return
	iniciar_logger_config(path_config)&&
	inicializar_comandos() &&
	iniciar_servidor_kernel()&&
	iniciar_conexion_memoria()&&
	iniciar_dispatch()&&
	iniciar_interrupt()&&
	iniciar_estados_planificacion()&&
	//iniciar_colas_entrada_salida()&&
	iniciar_semaforos()&&
	inicializar_dictionario_mutex_colas()&&
	iniciar_threads_io();
}
bool iniciar_semaforos(){
	sem_init(&sem_cont_grado_mp,0,config->GRADO_MULTIPROGRAMACION);
	sem_init(&sem_bin_new,0,0);
	sem_init(&sem_bin_ready,0,0);
	sem_init(&sem_bin_exit,0,0);
	sem_init(&sem_bin_cpu_libre,0,1);
	sem_init(&sem_bin_recibir_pcb,0,1);

	sem_init(&sem_bin_plp_procesos_nuevos_iniciado,0,1);
	sem_init(&sem_bin_plp_procesos_finalizados_iniciado,0,1);
	sem_init(&sem_bin_planificador_corto_iniciado,0,1);

	return true;
}


bool inicializar_dictionario_mutex_colas(){
	estados_dictionary = dictionary_create();
	estados_mutexes_dictionary = dictionary_create();

 
	void _agregar(t_codigo_estado codigo,void* estado,t_dictionary* diccionario){
		char* clave = string_itoa(codigo);
		dictionary_put(diccionario,clave,estado);
		free(clave);
	};

	void _agregar_estado(t_codigo_estado codigo,void* estado){
		_agregar(codigo,estado,estados_dictionary);
	};

	void _agregar_mx(t_codigo_estado codigo,pthread_mutex_t* mx){
		_agregar(codigo,mx,estados_mutexes_dictionary);
	};


	_agregar_estado(NEW,estado_new);
	_agregar_estado(READY,estado_ready);
	_agregar_estado(EXEC,pcb_exec);
	_agregar_estado(EXIT_STATE,estado_exit);

	_agregar_mx(NEW,&mx_new);
	_agregar_mx(READY,&mx_ready);
	_agregar_mx(EXEC,&mx_pcb_exec);
	_agregar_mx(EXIT_STATE,&mx_exit);
	return true;
}

void liberar_diccionario_colas(){
	if(estados_dictionary)
	dictionary_destroy(estados_dictionary);
	if(estados_mutexes_dictionary)
	dictionary_destroy(estados_mutexes_dictionary);
}
void bloquear_mutex_colas(){
	void _bloquear(char* _,void* element){
		pthread_mutex_t* mutex = (pthread_mutex_t*)element;
		pthread_mutex_lock(mutex);
	};
	dictionary_iterator(estados_mutexes_dictionary,_bloquear);
}

void desbloquear_mutex_colas(){
	void _desbloquear(char* _,void* element){
		pthread_mutex_t* mutex = (pthread_mutex_t*)element;
		pthread_mutex_unlock(mutex);
	};
	dictionary_iterator(estados_mutexes_dictionary,_desbloquear);
}


bool pcb_es_id (void* elem, uint32_t pid){
	t_pcb* pcb = (t_pcb*)elem;
	return pcb->PID == pid;
}


t_queue* buscar_cola_de_pcb(uint32_t pid){

	bool _es_id (void* elem){
		return pcb_es_id(elem,pid);
	};

	bool _es_del_pcb(void* elem){
		t_queue* cola = (t_queue*)elem;
		return list_any_satisfy(cola->elements,_es_id);
	};

	return list_find(get_estados(),_es_del_pcb);
}

t_pcb* buscar_pcb_en_cola(t_queue* cola,uint32_t pid){

	bool _es_id (void* elem){
		return pcb_es_id(elem,pid);
	}
	return list_find(cola->elements,_es_id);
}

bool esta_en_exec(uint32_t pid){
	return pcb_exec!=NULL && pcb_exec->PID==pid;
}

t_pcb_query* buscar_pcb(uint32_t pid){
	bloquear_mutex_colas();
	t_queue* cola = NULL;
	if(!esta_en_exec(pid))
		cola = buscar_cola_de_pcb(pid);
	t_pcb_query* pcb_query = malloc(sizeof(t_pcb_query));
	pcb_query->estado = cola;
	pcb_query->pcb = cola!=NULL? buscar_pcb_en_cola(cola,pid):pcb_exec;
	desbloquear_mutex_colas();

	return pcb_query;
}

/// @brief 
	//Retorna una lista de t_queue* con las colas de estados
/// @return 
t_list* get_estados(){return dictionary_elements(estados_dictionary);}


bool iniciar_planificadores(){
	pthread_t thread_plp_new;
	pthread_t thread_plp_exit;
	pthread_t thread_pcp;

	pthread_create(&thread_plp_new,NULL, (void*)plp_procesos_nuevos,NULL);
	pthread_create(&thread_plp_exit,NULL, (void*)plp_procesos_finalizados,NULL);
	pthread_create(&thread_pcp,NULL,(void*)planificador_corto,NULL);
	
	pthread_detach(thread_plp_new);
	if (thread_plp_new == -1){
		loguear_error("No se pudo iniciar el planificador de largo plazo.");
		return false;
	}
	pthread_detach(thread_plp_exit);
	if (thread_plp_exit == -1){
		loguear_error("No se pudo iniciar el planificador de largo plazo.");
		return false;
	}
	pthread_detach(thread_pcp);
	if (thread_pcp == -1){
		loguear_error("No se pudo iniciar el planificador de corto plazo.");
		return false;
	}
	return true;
}

t_queue* get_cola_pcb(t_pcb* pcb){
	return NULL;
}

//Este método se llama cuando se inicia un proceso
void plp_procesos_nuevos(){
	while(1){
		
			
			sem_wait(&sem_bin_new); //Bloquea plp hasta que aparezca un proceso
			sem_wait(&sem_bin_plp_procesos_nuevos_iniciado); 
			sem_wait(&sem_cont_grado_mp); //Se bloquea en caso de que el gradodemultiprogramación esté lleno
			bool proceso_new_a_ready = cambio_de_estado(estado_new, estado_ready,&mx_new,&mx_ready);
			if(proceso_new_a_ready){
				sem_post(&sem_bin_ready);
				loguear("El proceso ingresó correctamente a la lista de ready");
				sem_post(&sem_bin_plp_procesos_nuevos_iniciado); 
			}
			else {
				loguear("Se fue todo al carajo en plp_procesos_nuevos");
			}	
	}
}


void plp_procesos_finalizados(){
	while(1){
		
			sem_wait(&sem_bin_exit);
			sem_wait(&sem_bin_plp_procesos_finalizados_iniciado); 
			loguear_warning("Se va a sacar un PCB de temp");
			t_pcb* pcb = pop_estado_get_pcb(estado_temp,&mx_temp);
			loguear_warning("Se sacó el PCB: %d de cola temp para eliminar.", pcb->PID );
			eliminar_proceso_en_memoria(pcb);
			loguear_warning("Se eliminó el proceso: %d de memoria.", pcb->PID );
			push_proceso_a_estado(pcb,estado_exit,&mx_exit);
			sem_post(&sem_cont_grado_mp);
		sem_post(&sem_bin_plp_procesos_finalizados_iniciado); 
		
	}
}


//Muestra por pantalla el valor actual del semáforo
//Ejempolo: //loguear_semaforo("sem_bin_planificador_corto_iniciado: %d\n",&sem_bin_planificador_corto_iniciado);	
void loguear_semaforo(char* texto,sem_t* semaforo){
	int sval;
	sem_getvalue(semaforo,&sval);
	printf(texto,sval);

}

void planificador_corto(){
	while(1){					
			sem_wait(&sem_bin_ready); //Hay que ver si tiene que estar acá. En este caso se considera que cada replanificación pasa por aca
			sem_wait(&sem_bin_planificador_corto_iniciado); 
			ejecutar_planificacion();
			// Esperar la vuelta del PCB
			//sem_post(&sem_bin_recibir_pcb);
			recibir_pcb_de_cpu();			
			/* Cuando recibe un pcb con centexto finalizado, lo agrega a la cola de exit  */
			sem_post(&sem_bin_planificador_corto_iniciado); 
		
	}
}
void liberar_pcb_exec(){

	pthread_mutex_lock(&mx_pcb_exec);
	pcb_exec = NULL;
	pthread_mutex_unlock(&mx_pcb_exec);
}

t_pcb_query * recibir_pcb_y_actualizar(t_paquete* paquete){

	t_pcb* pcb_recibido = recibir_pcb(paquete);  
	t_pcb_query* pcb_query = buscar_pcb(pcb_recibido->PID);
	reemplazar_pcb_con(pcb_query->pcb,pcb_recibido);
	pcb_destroy(pcb_recibido);
	return pcb_query;
}



void modificar_quantum_restante(t_pcb* pcb){
	tiempo_final = time(NULL);
	if ( (difftime(tiempo_final,tiempo_inicial)*1000) < config->QUANTUM ){
		pcb->quantum = pcb->quantum - ((int)difftime(tiempo_final,tiempo_inicial)*1000);
	}
}

void io_gen_sleep(int pid,char** splitter){
	loguear_warning("Entra al case");
	char pid_mas_unidades [20];
	sprintf(pid_mas_unidades,"%u",pid);
	loguear_warning("El pid es %s", pid_mas_unidades);
	strcat(pid_mas_unidades," ");
	strcat(pid_mas_unidades, splitter[1]);
	loguear_warning("El mensaje es %s", pid_mas_unidades);

	loguear_warning("IO_GEN_SLEEP -> Interfaz:%s Unidades:%s", splitter[0], splitter[1]);
	void *ptr_conexion = dictionary_get(diccionario_nombre_conexion, splitter[0]);
	int conexion_io = *(int *)ptr_conexion;

	enviar_texto(pid_mas_unidades,
				IO_GEN_SLEEP,
				conexion_io);
	loguear_warning("Peticion a IO enviada");
}
// void io_stdin(int pid, char** splitter){
// 	loguear_warning("Entra al case");
// 	char pid_direccion_tamanio [30];
// 	sprintf(pid_direccion_tamanio,"%u",pid);
// 	loguear_warning("El pid es %s", pid_direccion_tamanio);
// 	strcat(pid_direccion_tamanio," ");
// 	strcat(pid_direccion_tamanio, splitter[1]);
// 	strcat(pid_direccion_tamanio," ");
// 	strcat(pid_direccion_tamanio,splitter[2]);
// 	loguear_warning("El mensaje es %s", pid_direccion_tamanio);

// 	loguear_warning("IO_STDIN_READ -> Interfaz:%s Direccion:%s Tamanio:%s", splitter[0], splitter[1], splitter[2]);
// 	void *ptr_conexion = dictionary_get(diccionario_nombre_conexion, splitter[0]);
// 	int conexion_io = *(int *)ptr_conexion;

// 	enviar_texto(pid_direccion_tamanio,
// 				IO_STDIN_READ,
// 				conexion_io);
// 	loguear_warning("Peticion a IO enviada");
// }



void io_handler_exec(t_pcb* pcb_recibido){
	int cod_op_io = recibir_operacion(cpu_dispatch);
	char* peticion;
	char** splitter = string_array_new();
	peticion = recibir_mensaje(cpu_dispatch);
	splitter = string_split(peticion," ");

	if(!existe_interfaz(splitter[0]/*Nombre*/)){
		pasar_a_exit(pcb_recibido);		
		return;
	}
	
	t_blocked_interfaz* interfaz = dictionary_get(diccionario_nombre_qblocked, splitter[0]);
	proceso_a_estado(pcb_recibido, interfaz->estado_blocked, &interfaz->mx_blocked);

	switch(cod_op_io){
		case IO_GEN_SLEEP:
				io_gen_sleep(pcb_recibido->PID,splitter);
			break;
		case IO_STDIN_READ:
			// io_stdin(pcb_recibido->PID, splitter);
			break;
		case IO_STDOUT_WRITE:
		//  io_stdout(pcb_recibido->PID, splitter);
			break;

		default:
			pasar_a_exit(pcb_recibido);			
			break;
	}
	free(peticion);	
}


void recibir_pcb_de_cpu(){
	loguear_warning("Intento recibir de CPU!");
	t_paquete *paquete = recibir_paquete(cpu_dispatch);
	int cod_op = paquete->codigo_operacion;
	loguear("Cod op CPU: %d", cod_op);
	t_pcb_query* pcb_query = recibir_pcb_y_actualizar(paquete);
	t_pcb* pcb_recibido = pcb_query->pcb;
	if(es_vrr()) modificar_quantum_restante(pcb_recibido);
	liberar_pcb_exec();
	paquete_destroy(paquete);
	// PAUSAR POR DETENER PLANI
	
	sem_wait(&sem_bin_recibir_pcb);
	switch (cod_op)
	{
		case FINALIZAR_PROCESO_POR_CONSOLA:
			pasar_a_exit(pcb_recibido);	
			break;
		case CPU_EXIT:
			pasar_a_exit(pcb_recibido);			 
			break;
		case FIN_QUANTUM:
			proceso_a_estado(pcb_recibido, estado_ready,&mx_ready); 
			if (es_vrr()){
				pcb_recibido->quantum = config->QUANTUM;
			}
			sem_post(&sem_bin_ready);
			break;
		case IO_HANDLER:
            io_handler_exec(pcb_recibido);
			break;
		default:
			
			break;
	}
	sem_post(&sem_bin_cpu_libre);
	sem_post(&sem_bin_recibir_pcb);
	free(pcb_query);
	
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

t_comando_consola* comando_consola_create(op_code_kernel code,char* nombre,char* params,bool(*funcion)(char**)){
    t_comando_consola* comando = malloc(sizeof(t_comando_consola));
    comando->comando = code;
    comando->parametros =  string_duplicate(params);
    comando->funcion = funcion;
    comando->nombre= string_duplicate(nombre);

    return comando;
}


void agregar_comando(op_code_kernel code,char* nombre,char* params,bool(*funcion)(char**)){
    void _agregar_comando_(char* texto){
		 t_comando_consola* comando = comando_consola_create(code,nombre,params,funcion);
        dictionary_put(comandos_consola,texto,comando);				
    };
	 char* code_str = string_itoa(code);
    _agregar_comando_(code_str);
	free(code_str);
    _agregar_comando_(nombre);
}


void imprimir_valores_leidos(char** substrings){

	int index=0;
	void imprimir_valor(char* leido){
		loguear("substring[%d] vale:%s\n",index++,leido);
	};
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
	{	list_iterate(instrucciones_sript,ejecutar_sript);
		list_destroy_and_destroy_elements(instrucciones_sript,free);
	}
	
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
	int numero_comando = -1;
	if(existe_comando(comando)){
		comando_consola = dictionary_get(comandos_consola,comando);
		numero_comando = comando_consola->comando;
		if(comando_consola->comando != EXIT){
       		comando_consola->funcion(parametros);
		}
	}
	
	string_array_destroy(parametros);
	return numero_comando;
}

bool crear_proceso_en_memoria(t_pcb* pcb){
	enviar_pcb(pcb,CREACION_PROCESO,conexion_memoria); // Enviar proceso a memoria para que inicialice 
	op_code operacion = recibir_operacion(conexion_memoria);
	switch (operacion)
	{
	case CREACION_PROCESO:
		char* mensaje = recibir_mensaje(conexion_memoria);
		loguear("OK: %s",mensaje);
		free(mensaje);
		break;
	case CREACION_PROCESO_FALLIDO:
		char* mensaje_falla = recibir_mensaje(conexion_memoria);
		loguear_error("No se pudo crear el proceso: %s",mensaje_falla);
		free(mensaje_falla);
		return false;
		break;
	default:
		break;
	}
	
	return true;
}

// Se pueden unificar estas dos funciones en un solo Switch??????

bool eliminar_proceso_en_memoria(t_pcb* pcb){
	loguear_warning("eliminar_proceso_en_memoria");
	enviar_pcb(pcb,ELIMINACION_PROCESO,conexion_memoria); // Enviar proceso a memoria para que inicialice 
	
	op_code operacion = recibir_operacion(conexion_memoria);
	switch (operacion)
	{
	case ELIMINACION_PROCESO:
		char* mensaje = recibir_mensaje(conexion_memoria);
		loguear("OK: %s",mensaje);
		free(mensaje);
		break;
	case ELIMINACION_PROCESO_FALLIDO:
		char* mensaje_falla = recibir_mensaje(conexion_memoria);
		loguear_error("No se pudo eliminar el proceso: %s",mensaje_falla);
		free(mensaje_falla);
		return false;
		break;
	default:
		break;
	}

	return true;
}


bool iniciar_proceso(char** parametros){
	
	bool parametros_iniciar_proceso_validos(char** parametros){
		bool validado = 
		string_array_size(parametros)==2;
		
		if(!validado)
			printf("\tINICIAR_PROCESO debe recibir 1 parámetro:\n\tPath (string)\n");

		return validado;
	};

	loguear("iniciando proceso...");
	imprimir_valores_leidos(parametros);	

	bool parametros_validos = parametros_iniciar_proceso_validos(parametros);

	if(parametros_validos){
	
		char *path = string_duplicate(parametros[1]);
		loguear("PATH: %s",path);
		t_pcb* pcb = pcb_create(path);   // Se crea el PCB
		
		bool proceso_creado = crear_proceso_en_memoria(pcb);
		if(proceso_creado){
			push_proceso_a_estado(pcb,estado_new,&mx_new); //Pasa el PCB a New
			sem_post(&sem_bin_new);
		}
		else pcb_destroy(pcb);

		free(path);
	}
			
	return parametros_validos;
}


void proceso_a_estado(t_pcb* pcb, t_queue* estado,pthread_mutex_t* mx_estado){
	pthread_mutex_lock(mx_estado);
	queue_push(estado,pcb);
	pthread_mutex_unlock(mx_estado);
}


bool finalizar_proceso(char** substrings){	
		imprimir_valores_leidos(substrings);
		uint32_t pid = atoi(substrings[1]);
		eliminar_proceso(pid);
		loguear("Finaliza el proceso <%s> - Motivo: Finalizado por consola",substrings[1]);
		return true;
}



void listar_comandos(){
	printf("Los comandos disponibles son:\n");
	char* decoracion= "\t\t*****************************************************\n";
	printf("%s",decoracion);
	int cantidad = dictionary_size(comandos_consola)/2;


	for (int i = 0; i < cantidad; i++) {
		char* id = string_itoa(i);
		t_comando_consola* comando = dictionary_get(comandos_consola,id);
        printf("\t%d. %s %s\n",i ,comando->nombre ,comando->parametros);
		free(id);
    }
	printf("%s",decoracion);
}

bool iniciar_planificacion(char** substrings){
	if(planificacion_detenida){
		sem_post(&sem_bin_plp_procesos_nuevos_iniciado);
		sem_post(&sem_bin_plp_procesos_finalizados_iniciado);
		sem_post(&sem_bin_planificador_corto_iniciado);
		sem_post(&sem_bin_recibir_pcb);

		planificacion_detenida = false;
	}
	return true;
}

void detener_plani_nuevos(){
	sem_wait(&sem_bin_plp_procesos_nuevos_iniciado);
}
void detener_plani_finalizados(){
	sem_wait(&sem_bin_plp_procesos_finalizados_iniciado);
}
void detener_plani_corto(){
	sem_wait(&sem_bin_planificador_corto_iniciado);
}
void detener_recibir_pcb(){
	sem_wait(&sem_bin_recibir_pcb);
}

void hilo_detencion(void* detenedor){
	pthread_t thread_detener_plani;
	pthread_create(&thread_detener_plani,NULL, detenedor,NULL);
	pthread_detach(thread_detener_plani);
}

void* _detener(){
		
		hilo_detencion(&detener_plani_nuevos);
		hilo_detencion(&detener_plani_finalizados);
		hilo_detencion(&detener_plani_corto);
		hilo_detencion(&detener_recibir_pcb);	

		return NULL;
	};
bool detener_planificacion(char** substrings){

	if(!planificacion_detenida){
		planificacion_detenida = true;
		pthread_t thread_detener_plani;
		pthread_create(&thread_detener_plani,NULL, &_detener,NULL);
		pthread_detach(thread_detener_plani);
	}
	return true;
}

bool multiprogramacion(char** substrings){

	loguear("El grado de multiprogramación anterior es: %d",config->GRADO_MULTIPROGRAMACION);
	if(string_array_size(substrings)>0){
		char* valor = substrings[1];
		if(is_numeric(valor)){
			char* endptr;
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

bool es_rr(){
	return config->ALGORITMO_PLANIFICACION.id == RR;
}
bool es_vrr(){
	return config->ALGORITMO_PLANIFICACION.id == VRR;
}

bool proceso_estado(){
	t_queue* estado_exec = queue_create();
	pthread_mutex_lock(&mx_pcb_exec);
	if(pcb_exec!=NULL)
		queue_push(estado_exec,pcb_exec);
	pthread_mutex_unlock(&mx_pcb_exec);
	imprimir_cola(estado_new, "Nuevo");
    imprimir_cola(estado_ready, "Listo");
	// imprimir_todas las colas de blocked
	imprimir_cola(estado_exec, "Ejecutando");	
	imprimir_cola(estado_exit, "Finalizado");
	imprimir_cola(estado_temp, "Temporal");
	if( es_vrr()) imprimir_cola(estado_ready_plus,"Listo VRR");

	queue_destroy(estado_exec);
	
	return true;
}


bool finalizar_consola(char** parametros){
	loguear("Consola finalizada.");	
	return false;
}


void iniciar_consola(){
	char *cadenaLeida;
	int comando = -1;
	 while (comando == -1 || comando != EXIT) {
		listar_comandos();
        cadenaLeida =  leer_texto_consola();
		comando = ejecutar_comando_consola(cadenaLeida);
		free(cadenaLeida);
    }
}

void ejecutar_planificacion(){
	config->ALGORITMO_PLANIFICACION.planificar();
}

void ejecutar_proceso(){

	loguear("Se debe enviar el pcb en exec a la cpu");
	pthread_mutex_lock(&mx_pcb_exec);
	enviar_pcb(pcb_exec,EJECUTAR_PROCESO,cpu_dispatch);
	pthread_mutex_unlock(&mx_pcb_exec);
	
}

void pcb_a_exec(t_pcb* pcb){
	if(pcb != NULL){
		pthread_mutex_lock(&mx_pcb_exec);
		pcb_exec = pcb;
		pthread_mutex_unlock(&mx_pcb_exec);
		ejecutar_proceso();
	}
}

t_pcb* ready_a_exec(){	
	sem_wait(&sem_bin_cpu_libre); // Verificamos que no haya nadie en CPU
	
	t_pcb* pcb = pop_estado_get_pcb(estado_ready,&mx_ready);
	pcb_a_exec(pcb);
	return pcb;
}

void planificacion_FIFO(){
	loguear("Planificando por FIFO");
	ready_a_exec();
}

void controlar_quantum (t_pcb* pcb_enviado){
	t_pcb pcb;
	memcpy(&pcb,pcb_enviado,sizeof(t_pcb));
	if(config->QUANTUM)
	{	usleep(config->QUANTUM*1000);		
		pthread_mutex_lock(&mx_pcb_exec);
		if(pcb_exec != NULL && pcb_exec->PID==pcb.PID){
			enviar_texto("FIN_QUANTUM",FIN_QUANTUM,cpu_interrupt);
			loguear("PID: <%d> - Desalojado por fin de Quantum",pcb.PID);
			
		}
		pthread_mutex_unlock(&mx_pcb_exec);
	}
}

void crear_hilo_quantum(t_pcb* pcb){
	pthread_t thread_quantum;

	pthread_create(&thread_quantum,NULL, (void*)controlar_quantum,pcb);
	
	pthread_detach(thread_quantum);
	if(es_vrr()){
		tiempo_inicial = time(NULL);
	}
	if (thread_quantum == -1)
		loguear_error("No se pudo iniciar el hilo de quantum para el PID: %d",pcb->PID);	
	
}



void interrumpir_por_fin_quantum(){
	loguear("FIN DE QUANTUM: Se debe pasar el pcb a ready y notificar a la cpu");
	t_pcb* pcb;
	pthread_mutex_lock(&mx_pcb_exec);
	pcb = pcb_exec;
	pcb_exec = NULL;	
	pthread_mutex_unlock(&mx_pcb_exec);
	push_proceso_a_estado(pcb,estado_ready,&mx_ready); // Thread safe
	sem_post(&sem_bin_ready); //Se le avisa a pcp que un nuevo proceso ingresó a esa lista
}


void planificacion_RR(){
	loguear("Planificando por Round Robbin");
	t_pcb* pcb = ready_a_exec();
	crear_hilo_quantum(pcb);
}

// Para VRR, en caso de que el proceso no ejecute todo su quantum se realiza la
// validacion correspondiente en kernel para saber a qué cola mandar el pcb
// una manera de distinguir entre IO y fin de proceso es el codigo de op
// que le manda cpu.

t_pcb*  ready_plus_a_exec(){
	t_pcb* pcb = pop_estado_get_pcb(estado_ready_plus,&mx_ready_plus);
	pcb_a_exec(pcb);
	return pcb;
}


void planificacion_VRR(){
	loguear("Planificando por Virtual Round Robbin");	
	t_pcb* pcb;
	if(!queue_is_empty(estado_ready_plus))
		pcb = ready_plus_a_exec();	
	else
		pcb = ready_a_exec();
	
	crear_hilo_quantum(pcb); // Agregar diferencia timestamp!!
}

////// MODIFICACIONES DE ESTADO

bool cambio_de_estado(t_queue* estado_origen, t_queue* estado_destino,pthread_mutex_t* sem_origen,pthread_mutex_t* sem_destino){	
	bool transicion = transicion_valida(estado_origen, estado_destino);
	if(transicion){
		pthread_mutex_lock(sem_origen);

 		t_pcb* pcb = queue_pop(estado_origen);
		push_proceso_a_estado(pcb,estado_destino,sem_destino);

		pthread_mutex_unlock(sem_origen);
	}
	return transicion;
}
// Convertir en matriz (Como en sintaxis con Roxy :) 
bool transicion_valida(t_queue* estado_origen,t_queue* estado_destino){
	if (estado_destino==estado_new || estado_origen==estado_exit){
		return false;
	}
	if(estado_destino==estado_ready && estado_origen==estado_exit){
		return false;
	}	
	return true;
}

void push_proceso_a_estado(t_pcb* pcb, t_queue* estado,pthread_mutex_t* mx_estado){
	pthread_mutex_lock(mx_estado);
	queue_push(estado,pcb);
	pthread_mutex_unlock(mx_estado);
}

t_pcb* pop_estado_get_pcb(t_queue* estado,pthread_mutex_t* mx_estado){
	pthread_mutex_lock(mx_estado);
	t_pcb* pcb = queue_pop(estado);
	pthread_mutex_unlock(mx_estado);
	return pcb;
}

// LIBERAR Y/O DESTRUIR ELEMENTOS Y PROCESOS

void config_destroy_kernel(t_config_kernel * config){
	config_destroy(config->config);
	string_array_destroy(config->INSTANCIAS_RECURSOS);
	string_array_destroy(config->RECURSOS);
	free(config);
}

void liberar_colas(){

	void liberar_pcb(void *pcb){
		pcb_destroy((t_pcb*)pcb);
	};

	void liberar_cola(t_queue* cola){
		if(cola!=NULL)
			queue_destroy_and_destroy_elements(cola,liberar_pcb);
	};

	liberar_cola(estado_exit);
	liberar_cola(estado_new);
	liberar_cola(estado_ready);
	if(es_vrr()){
		liberar_cola(estado_ready_plus);
	}
	liberar_cola(estado_temp);
	liberar_cola(io_stdin);
	liberar_cola(io_stdout);
	liberar_cola(io_generica);
	liberar_cola(io_dialfs);
	if(pcb_exec!=NULL)
		pcb_destroy(pcb_exec);
	
}

void liberar_semaforos(){
	sem_destroy(&sem_cont_grado_mp);
	sem_destroy(&sem_bin_new);
	sem_destroy(&sem_bin_ready);
	sem_destroy(&sem_bin_exit);
	sem_destroy(&sem_bin_cpu_libre);
	sem_destroy(&sem_bin_recibir_pcb);

	pthread_mutex_destroy(&mx_new);
	pthread_mutex_destroy(&mx_ready);
	if(es_vrr()) pthread_mutex_destroy(&mx_ready_plus);
	pthread_mutex_destroy(&mx_exit);
	pthread_mutex_destroy(&mx_pcb_exec);
	pthread_mutex_destroy(&mx_temp);
}

void liberar_comando(void* c){
	 t_comando_consola* comando = (t_comando_consola*)c;
	 if(comando){
		if(comando->parametros)
		free(comando->parametros);
		if(comando->nombre)
		free(comando->nombre);
		 free(comando);
		comando=NULL;
	 }
}
void liberar_comandos(){
	
	if(comandos_consola!=NULL) 
		dictionary_destroy_and_destroy_elements(comandos_consola,liberar_comando);
}


void finalizar_kernel(){
	
	if (conexion_memoria != -1) liberar_conexion(conexion_memoria);
	if (cpu_dispatch != -1) liberar_conexion(cpu_dispatch);
	if (cpu_interrupt != -1) liberar_conexion(cpu_interrupt);
	if(config!=NULL) config_destroy_kernel(config);
	if(logger!=NULL) log_destroy(logger);
	liberar_comandos();
	liberar_colas();
	liberar_semaforos();	
	liberar_diccionario_colas();
}





bool iniciar_servidor_kernel(){
    //Iniciamos el servidor con el puerto indicado en la config
	kernel_escucha = iniciar_servidor(config->PUERTO_ESCUCHA);
	if(kernel_escucha == -1){
		loguear_error("El servidor no pudo ser iniciado");
		return false;
	}
	loguear("El Servidor iniciado correctamente");
	return true;
}

bool eliminar_proceso(uint32_t pid){ // Al implementar en consola, hay q parsear el char* a uint32_t
	
	bool _eliminar_proceso_en_lista(t_queue* estado, pthread_mutex_t* mutex_estado){
		if (!queue_is_empty(estado)) return eliminar_proceso_en_lista(pid, estado, mutex_estado);
		return false;
	};

	if (es_vrr()){
		return (_eliminar_proceso_en_lista(estado_new, &mx_new) || 
	_eliminar_proceso_en_lista(estado_ready, &mx_ready) ||
	eliminar_proceso_en_blocked(pid) ||   
	_eliminar_proceso_en_lista(estado_ready_plus, &mx_ready_plus) ||
	eliminar_proceso_en_exec(pid));
	}
	return (_eliminar_proceso_en_lista(estado_new, &mx_new) || 
	_eliminar_proceso_en_lista(estado_ready, &mx_ready) || 
	eliminar_proceso_en_blocked(pid) ||
	eliminar_proceso_en_exec(pid));
}

bool eliminar_proceso_en_exec(uint32_t pid){
	pthread_mutex_lock(&mx_pcb_exec);
	if(pcb_exec->PID == pid){
		enviar_texto("FINALIZAR PROCESO",FINALIZAR_PROCESO_POR_CONSOLA,cpu_interrupt);
		pthread_mutex_unlock(&mx_pcb_exec);
		return true;
	}
	pthread_mutex_unlock(&mx_pcb_exec);
	return false;
}

bool eliminar_proceso_en_lista(uint32_t pid_buscado,t_queue* estado_buscado ,pthread_mutex_t* mutex_estado_buscado){
	t_pcb* pcb_buscado;
	/////// la función que evalua el condicional devuelve un pcb... en este caso, se toma como un true?
	if (encontrar_en_lista(pid_buscado,estado_buscado, mutex_estado_buscado)){
	/////
		pcb_buscado = encontrar_en_lista(pid_buscado,estado_buscado, mutex_estado_buscado);
		pthread_mutex_lock(mutex_estado_buscado);
		if (list_remove_element(estado_buscado->elements, pcb_buscado)) loguear("Se removio el PCB buscado");
		pthread_mutex_unlock(mutex_estado_buscado);
		pasar_a_exit(pcb_buscado);
		return true;
	}
	return false;
}
bool eliminar_proceso_en_blocked(uint32_t pid){
	bool _eliminar_proceso_en_lista(t_queue* estado, pthread_mutex_t* mutex_estado){
		if (!queue_is_empty(estado)) return eliminar_proceso_en_lista(pid, estado, mutex_estado);
		return false;
	};
	if(lista_interfaces_blocked == NULL || lista_interfaces_blocked->head == NULL){
		return false;
	}
	for(t_link_element *nodo_interfaz_actual = lista_interfaces_blocked->head; nodo_interfaz_actual != NULL; nodo_interfaz_actual = nodo_interfaz_actual->next){
			// Obtener el dato almacenado en el nodo actual
        	t_blocked_interfaz *interfaz_blocked = (t_blocked_interfaz*)nodo_interfaz_actual->data;
			if(_eliminar_proceso_en_lista(interfaz_blocked->estado_blocked,&interfaz_blocked->mx_blocked)) return true;
		}
	return false;
	
}


t_pcb* encontrar_en_lista(uint32_t pid_buscado,t_queue* estado_buscado ,pthread_mutex_t* mutex_estado_buscado){

	bool es_el_pcb (t_pcb* pcb){return pid_buscado == pcb->PID;};

	t_pcb* pcb_encontrado = NULL;
	pthread_mutex_lock(mutex_estado_buscado);
	pcb_encontrado = list_find(estado_buscado->elements, (void*) es_el_pcb);
	pthread_mutex_unlock(mutex_estado_buscado);
	return pcb_encontrado;
}

void pasar_a_exit(t_pcb* pcb){
	proceso_a_estado(pcb, estado_temp,&mx_temp); 
	sem_post(&sem_bin_exit);
}


bool iniciar_threads_io(){
	pthread_t thread_io_conexion;
	pthread_create(&thread_io_conexion,NULL, (void*) iniciar_conexion_io,NULL);
	pthread_detach(thread_io_conexion);

	return true;
}

void iniciar_conexion_io(){
	diccionario_nombre_conexion = dictionary_create();
	diccionario_nombre_qblocked = dictionary_create();
	diccionario_conexion_qblocked = dictionary_create();
	//
	lista_interfaces_blocked = list_create();
	//
	while (1){
		t_blocked_interfaz* blocked_interfaz = malloc(sizeof(t_blocked_interfaz));

		int mutex = pthread_mutex_init(&blocked_interfaz->mx_blocked, NULL);
		if(mutex == -1){
			loguear_warning("El mutex no se inicio correctamente.");
			//jump al principio?
		}
		blocked_interfaz -> estado_blocked = queue_create();
		pthread_t thread;
    	int *fd_conexion_ptr = malloc(sizeof(int));
    	*fd_conexion_ptr = esperar_cliente(kernel_escucha);
		if(*fd_conexion_ptr == -1){ 
			loguear_warning("No se puso establecer la conexion con el cliente(I/O).");
			// JUmp al principio?
		}
		char* nombre_interfaz = malloc(16);
		nombre_interfaz = recibir_nombre(*fd_conexion_ptr);
		//
		char* string_conexion = string_itoa(*fd_conexion_ptr);
		loguear("bienvenido %s",nombre_interfaz);
		dictionary_put(diccionario_nombre_conexion,nombre_interfaz,fd_conexion_ptr);
		dictionary_put(diccionario_nombre_qblocked,nombre_interfaz, blocked_interfaz);
		dictionary_put(diccionario_conexion_qblocked,string_conexion, blocked_interfaz);
		//
		list_add(lista_interfaces_blocked,blocked_interfaz);
		//
    	pthread_create(&thread,NULL, (void*) io_handler,(int*)(fd_conexion_ptr)); // VER SI VA PUNTERO ACA (Joaco :) )
    	//
		free(blocked_interfaz);
		free(string_conexion);
		free(nombre_interfaz);
		pthread_detach(thread);
		
	}

}

bool existe_interfaz(char* nombre_interfaz){
	return (!dictionary_is_empty(diccionario_nombre_conexion) && dictionary_has_key(diccionario_nombre_conexion,nombre_interfaz));
}

char *recibir_nombre(int conexion){
	char* nombre = malloc(15);
	if(recibir_operacion(conexion) == NUEVA_IO)
	nombre = recibir_mensaje(conexion);
	else loguear_error("RECIBÍ VACÍO");
	return nombre;
}



bool le_queda_quantum(t_pcb* pcb){
	return pcb->quantum != config->QUANTUM;
	//VERIFICAR DESPUES DE HACER VIRTUAL ROUND ROBIN
}

void a_ready(t_pcb* pcb){
	if(es_vrr()){
		if(le_queda_quantum(pcb))
			push_proceso_a_estado(pcb,estado_ready_plus,&mx_ready_plus);
		else
			push_proceso_a_estado(pcb,estado_ready,&mx_ready);
	}
	else
		push_proceso_a_estado(pcb,estado_ready,&mx_ready);
	sem_post(&sem_bin_ready);
}

void io_handler(int *ptr_conexion){
	while(1){
		int conexion = *ptr_conexion;
		int cod_operacion = recibir_operacion(conexion);
		loguear_warning("LLego el cod op %d", cod_operacion);
		char* mensaje = string_new();
		mensaje = recibir_mensaje(conexion);
		loguear_warning("Llego el mensaje %s", mensaje);
		t_pcb* pcb;
		char* string_conexion = string_itoa(conexion);
		loguear_warning("Antes del get del diccionario");
		t_blocked_interfaz* interfaz = dictionary_get(diccionario_conexion_qblocked,string_conexion);
		free(string_conexion);
		switch (cod_operacion){
			// SI LA RESPONSABILIDAD DE LEER/ESCRIBIR MEMORIA PARA LAS STD LE CABE A LAS INTERFACES,
			// NO SERÍA NECESARIO DIVIDIR POR CASES ENTRE GENERICA, STDIN Y STDOUT
			// TODAS HARÍAN LO MISMO
			case TERMINO_IO_GEN_SLEEP:
				//mensaje = recibir_mensaje(conexion);
				pthread_mutex_lock(&interfaz -> mx_blocked);
				pcb = queue_pop(interfaz -> estado_blocked);
				pthread_mutex_unlock(&interfaz -> mx_blocked);
				// pop_estado_get_pcb()
				loguear_warning("Ya se popeo el PCB con PID: %d", pcb->PID);

				// if(es_vrr()){
				// 	if(le_queda_quantum(pcb))
				// 		push_proceso_a_estado(pcb,estado_ready_plus,&mx_ready_plus);
				// }
				// 
				// 	push_proceso_a_estado(pcb,estado_ready,&mx_ready);
				a_ready(pcb);
				//sem_post(&sem_bin_ready);
				

				break;
			case TERMINO_STDIN:
				//mensaje = recibir_mensaje(conexion);
				//enviar mensaje recibido a memoria para que lo almacene // ¿ESTO LO HACE LA PROPIA INTERFAZ?
				// pthread_mutex_lock(&blocked_interfaz -> mx_blocked);
				// pcb = queue_pop(blocked_interfaz-> estado_blocked);
				// pthread_mutex_unlock(&blocked_interfaz -> mx_blocked);
				//a_ready(pcb);

				default:
					return;
		}
		free(mensaje);
	}
	// envia a la interfaz correspodiente la operación que debe ejecutar
}
