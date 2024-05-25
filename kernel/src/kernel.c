#include "kernel.h"
#include "semaphore.h"

sem_t sem_cont_grado_mp;
sem_t sem_bin_new; //Sincroniza que plp (hilo new) no actúe hasta que haya un nuevo elemento en new
sem_t sem_bin_ready; //Sincroniza que pcp no actúe hasta que haya un nuevo elemento en ready
sem_t sem_bin_exit; //Sincroniza que plp (hilo exit) no actúe hasta que haya un nuevo elemento en exit
sem_t sem_bin_cpu_libre; // Sincroniza que no haya ningun PCB ejecutando en CPU

pthread_mutex_t mx_new = PTHREAD_MUTEX_INITIALIZER; // Garantiza mutua exclusion en estado_new. Podrían querer acceder consola y plp al mismo tiempo
pthread_mutex_t mx_ready = PTHREAD_MUTEX_INITIALIZER; //Garantiza mutua exclusion en estado_ready. Podrían querer acceder plp y pcp al mismo tiempo
pthread_mutex_t mx_exit = PTHREAD_MUTEX_INITIALIZER; // Garantiza mutua exclusion en estado_exit. Podrían querer acceder consola, plp y pcp al mismo tiempo
pthread_mutex_t mx_pcb_exec = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_deleted = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mx_blocked = PTHREAD_MUTEX_INITIALIZER;

int conexion_memoria, cpu_dispatch,cpu_interrupt, kernel_escucha, conexion_io;
int cod_op_dispatch,cod_op_interrupt,cod_op_memoria;
t_config_kernel* config;
t_dictionary * comandos_consola;
t_queue* estado_new, *estado_ready, *estado_blocked, *estado_exit, *estado_ready_plus, *estado_deleted,
		*io_stdin, *io_stdout, *io_generica, *io_dialfs;
t_pcb* pcb_exec; 


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
    agregar_comando(INICIAR_PROCESO,"INICIAR_PROCESO","[PATH]",&iniciar_proceso);
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
	io_stdin = queue_create();
	io_stdout = queue_create();
	io_generica = queue_create();
	io_dialfs = queue_create();
	return true;
}

bool iniciar_estados_planificacion(){

	estado_new = queue_create();
	estado_ready = queue_create();
	estado_blocked = queue_create();
	estado_exit = queue_create();	
	estado_deleted = queue_create();	
	if(es_vrr())
		estado_ready_plus = queue_create();
	
	
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
	iniciar_colas_entrada_salida()&&
	iniciar_semaforos();
	//&&iniciar_conexion_io();
}
bool iniciar_semaforos(){
	sem_init(&sem_cont_grado_mp,0,config->GRADO_MULTIPROGRAMACION);
	sem_init(&sem_bin_new,0,0);
	sem_init(&sem_bin_ready,0,0);
	sem_init(&sem_bin_exit,0,0);
	sem_init(&sem_bin_cpu_libre,0,1);

	// mx_new = PTHREAD_MUTEX_INITIALIZER; 
	// mx_exit = PTHREAD_MUTEX_INITIALIZER; 
	// mx_ready = PTHREAD_MUTEX_INITIALIZER; 
	// mx_pcb_exec = PTHREAD_MUTEX_INITIALIZER;    
	return true;
}

// bool iniciar_conexion_io(){
// 	pthread_t thread_io;

// 	pthread_create(&thread_io,NULL, (void*)io_handler,NULL);

// 	pthread_detach(thread_io);
// 	if(thread_io == -1){
// 		loguear_error("No se pudo iniciar la I/O.");
// 		return false;
// 	}
// 	return true;
// }

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

// Falta pasar un pcb de un lugar a exit. Se debe validar si el pcb se encuentra en una lista o no
void liberar_proceso(t_pcb* pcb){
	// Pasar de una cola o exec a exit
	t_queue* queue_origen =  get_cola_pcb(pcb);
	if (queue_origen!=NULL){
		//cambio_lista(queue_origen,*estado_exit,pcb)
	}
	
}




//Este método se llama cuando se inicia un proceso
void plp_procesos_nuevos(){
	while(1){
		sem_wait(&sem_bin_new); //Bloquea plp hasta que aparezca un proceso
		sem_wait(&sem_cont_grado_mp); //Se bloquea en caso de que el gradodemultiprogramación esté lleno
		bool proceso_new_a_ready = cambio_de_estado(estado_new, estado_ready,&mx_new,&mx_ready);
		if(proceso_new_a_ready){
			sem_post(&sem_bin_ready);
			loguear("El proceso ingresó correctamente a la lista de ready");
		}
		else {
			loguear("Se fue todo al carajo en plp_procesos_nuevos");
		}
	}
}


void plp_procesos_finalizados(){
	while(1){
		sem_wait(&sem_bin_exit);
		loguear_warning("Se va a sacar un PCB de exit");
		t_pcb* pcb = pop_estado_get_pcb(estado_exit,&mx_exit);
		loguear_warning("Se sacó el PCB: %d de cola Exit para eliminar.", pcb->PID );
		eliminar_proceso_en_memoria(pcb);
		loguear_warning("Se eliminó el proceso: %d de memoria.", pcb->PID );
		push_proceso_a_estado(pcb,estado_deleted,&mx_deleted);
		sem_post(&sem_cont_grado_mp);
		}
}


void planificador_corto(){
	while(1){
		sem_wait(&sem_bin_ready); //Hay que ver si tiene que estar acá. En este caso se considera que cada replanificación pasa por aca
		ejecutar_planificacion();
		// Esperar la vuelta del PCB
		recibir_pcb_de_cpu();
		// verificar a que lista debe ir
		// enviar a ready/blocked/exit (signal a esa cola)
		/* Cuando recibe un pcb con centexto finalizado, lo agrega a la cola de exit y hace un sem_post(&sem_bin_exit) */
	}

}
void liberar_pcb_exec(){
	pthread_mutex_lock(&mx_pcb_exec);
	pcb_exec = NULL;
	pthread_mutex_unlock(&mx_pcb_exec);
}
void recibir_pcb_de_cpu(){
	loguear_warning("Intento recibir de CPU!");
	t_paquete *paquete = recibir_paquete(cpu_dispatch);
	int cod_op = paquete->codigo_operacion;
	loguear("Cod op CPU: %d", cod_op);
	t_pcb* pcb_recibido = recibir_pcb(paquete);  
	liberar_pcb_exec();
	paquete_destroy(paquete);
	switch (cod_op)
	{
		case CPU_EXIT:
			proceso_a_estado(pcb_recibido, estado_exit,&mx_exit); 
			proceso_estado();
			sem_post(&sem_bin_exit);
			break;
		case FIN_QUANTUM:
			proceso_a_estado(pcb_recibido, estado_ready,&mx_ready); 
			sem_post(&sem_bin_ready);
			break;
		case CPU_INTERRUPT:
			// QUE HACEMOS???
			/* Puede pasar a ready( sem_post(&sem_bin_ready); ) , blocked (o en VRR a Ready+) */
			//return false;
			break;
		case IO_GEN_SLEEP:
            t_peticion_generica* peticion_generica = crear_peticion_generica(pcb_recibido,paquete->buffer);//CREAR FUNCION!!!
            proceso_a_estado(pcb_recibido,estado_blocked,&mx_blocked);//Crear semáforo blocked
            enviar_peticion(peticion_generica); //Acá tiene la interfaz, se puede averiguar a qué conexión se envía mediante el diccionario. CREAR FUNCION!!!
			break;
		default:
			break;
	}
	sem_post(&sem_bin_cpu_libre);
	
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
	int numero_comando = -1;
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
	}

	loguear("iniciando proceso...");
	imprimir_valores_leidos(parametros);	

	if(!parametros_iniciar_proceso_validos(parametros))
	return false;
	
	char *path = string_duplicate(parametros[1]);
	loguear("PATH: %s",path);
	// Crear if de planificacion para ver si se usa pcb_create o pcb_create_quantum?????????
	t_pcb* pcb = pcb_create(path);   // Se crea el PCB
	
	bool proceso_creado = crear_proceso_en_memoria(pcb);
	if(proceso_creado){
		push_proceso_a_estado(pcb,estado_new,&mx_new); //Pasa el PCB a New
		sem_post(&sem_bin_new);
	}
	else pcb_destroy(pcb);

	free(path);
		
	return true;
}


void proceso_a_estado(t_pcb* pcb, t_queue* estado,pthread_mutex_t* mx_estado){
	pthread_mutex_lock(mx_estado);
	queue_push(estado,pcb);
	pthread_mutex_unlock(mx_estado);
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
    imprimir_cola(estado_blocked, "Bloqueado");
	imprimir_cola(estado_exec, "Ejecutando");	
	imprimir_cola(estado_exit, "Finalizado");
	imprimir_cola(estado_deleted, "Eliminado");
	if( es_vrr())
	imprimir_cola(estado_ready_plus,"Listo VRR");

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

void planificacion_FIFO(){
	loguear("Planificando por FIFO");
	
	/* ¿Esto convendria dividirlo en funciones, que cada una se encargue de un estado?
	por ej, todo lo de abajo estaria en la funcion ready_a_running

	SE UTILIZÓ de_ready_a_exec (que tiene hecho el mutex de ready)*/

	ready_a_exec();
	
	/*SUGERENCIA
	*/
	// de exec a blocked
	// y de blocked a ready
	
	// faltaria tener en cuenta el resto de los estados, por ej
	// running_a_ready
	// running_a_blocked
	// funcion blocked_a_ready
	// ready_a_exit???? Esto no es largo plazo??

	// RR podría usar lo mismo, solamente habria que agregar el validador del quantum

	
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
	if (thread_quantum == -1)
		loguear_error("No se pudo iniciar el hilo de quantum para el PID: %d",pcb->PID);	
	
}

void ejecutar_proceso(){

	loguear("Se debe enviar el pcb en exec a la cpu");
	pthread_mutex_lock(&mx_pcb_exec);
	loguear_pcb(pcb_exec);
	enviar_pcb(pcb_exec,EJECUTAR_PROCESO,cpu_dispatch);
	pthread_mutex_unlock(&mx_pcb_exec);
	// Caso RR/VRR: Crear hilo con quantum
	
}

void interrumpir_por_fin_quantum(){
	loguear("FIN DE QUANTUM: Se debe pasar el pcb a ready y notificar a la cpu");
	t_pcb* pcb;
	pthread_mutex_lock(&mx_pcb_exec);
	pcb = pcb_exec;
	pcb_exec = NULL;
	pthread_mutex_unlock(&mx_pcb_exec);
	push_proceso_a_estado(pcb,estado_ready,&mx_ready); // Thread safe
	// Se reemplaza esta función por la función push_proceso_a_estado, que tiene mutex
	//queue_push(estado_ready,pcb);

	sem_post(&sem_bin_ready); //Se le avisa a pcp que un nuevo proceso ingresó a esa lista

	loguear_pcb(pcb);

}

void planificacion_RR(){
	loguear("Planificando por Round Robbin");
	//kernel chequea el quantum que le manda cpu luego de cada instrucción
	//t_pcb* pcb = pop_estado_get_pcb(estado_ready,&mx_ready);
		// interrumpir_por_fin_quantum();	
	ready_a_exec();
	crear_hilo_quantum(pcb_exec);			
}

// Para VRR, en caso de que el proceso no ejecute todo su quantum se realiza la
// validacion correspondiente en kernel para saber a qué cola mandar el pcb
// una manera de distinguir entre IO y fin de proceso es el codigo de op
// que le manda cpu.

void planificacion_VRR(){
	loguear("Planificando por Virtual Round Robbin");

	// TO DO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

}




////// MODIFICACIONES DE ESTADO

bool modificacion_estado(t_queue* estado_origen,t_queue* estado_destino){
	if (estado_destino==estado_new){
		return false;
	}
	if(estado_origen==estado_exit){
		return false;
	}
	if(estado_destino==estado_ready && estado_origen==estado_exit){
		return false;
	}

	if(estado_destino==estado_blocked && estado_origen!=estado_new){
		return false;
	}	

	return true;
}

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
	if(estado_destino==estado_blocked && estado_origen==estado_new){
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


void ready_a_exec(){
	sem_wait(&sem_bin_cpu_libre); // Verificamos que no haya nadie en CPU
	//////// IMPORTANTE HACER EL SEM_POST CUANDO CUELVA UN PCB DE CPU
	t_pcb* pcb = pop_estado_get_pcb(estado_ready,&mx_ready);
	if(pcb != NULL){
		pthread_mutex_lock(&mx_pcb_exec);
		pcb_exec = pcb;
		pthread_mutex_unlock(&mx_pcb_exec);
		ejecutar_proceso();
	}
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
	}

	void liberar_cola(t_queue* cola){
		if(cola!=NULL)
			queue_destroy_and_destroy_elements(cola,liberar_pcb);
	};

	liberar_cola(estado_blocked);
	liberar_cola(estado_exit);
	liberar_cola(estado_new);
	liberar_cola(estado_ready);
	liberar_cola(estado_ready_plus);
	liberar_cola(estado_deleted);
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

	pthread_mutex_destroy(&mx_new);
	pthread_mutex_destroy(&mx_ready);
	pthread_mutex_destroy(&mx_exit);
	pthread_mutex_destroy(&mx_pcb_exec);
	pthread_mutex_destroy(&mx_deleted);
}

void finalizar_kernel(){
	
	if (conexion_memoria != -1) liberar_conexion(conexion_memoria);
	if (cpu_dispatch != -1) liberar_conexion(cpu_dispatch);
	if (cpu_interrupt != -1) liberar_conexion(cpu_interrupt);
	if(config!=NULL) config_destroy_kernel(config);
	if(logger!=NULL) log_destroy(logger);
	if(comandos_consola!=NULL) dictionary_destroy(comandos_consola);
	liberar_colas();
	liberar_semaforos();	
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

// BRAND NEW
//cambiar el tipo y ver como engancharlo.
// bool iniciar_conexion_io(){
// 	while (1){
// 		pthread_t thread;
//     	int *fd_conexion_ptr = malloc(sizeof(int));
//     	*fd_conexion_ptr = accept(kernel_escucha, NULL, NULL);
// 		//revisar
// 		char* nombre_interfaz = recibir_mensaje(kernel_escucha);
//     	pthread_create(&thread,NULL, (void*) io_handler(nombre_interfaz),fd_conexion_ptr);
//     	//
// 		pthread_detach(thread);
		
// 	}
// }
// BRAND NEW

// // BRAND NEW
// void io_handler(char* nombre){
// 	// envia a la interfaz correspodiente la operación que debe ejecutar
// }
// // BRAND NEW

// // BRAND NEW
// void recibir_peticion_io_de_cpu(){
// 	// while(1)?
// 	// recibe una petición de la CPU
// 	// verifica que exista la interfaz 
// 	// verifica que esté conectada
// 	// verifica que la interfaz admita dicha operación
// 	// envia el proceso a BLOCKED
// 	//
// }
// // BRAND NEW
