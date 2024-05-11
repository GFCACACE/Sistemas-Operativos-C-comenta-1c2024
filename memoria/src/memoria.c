#include "memoria.h"

int memoria_escucha, conexion_cpu, conexion_kernel;
t_config_memoria *config_memoria;
t_dictionary *procesos;

t_config_memoria *iniciar_config_memoria(char *config_path)
{

	t_config *_config = config_create(config_path);
	if (_config == NULL)
		return NULL;
	t_config_memoria *config_memoria = malloc(sizeof(t_config_memoria));

	config_memoria->PUERTO_ESCUCHA = config_get_int_value(_config, "PUERTO_ESCUCHA");
	config_memoria->TAM_MEMORIA = config_get_int_value(_config, "TAM_MEMORIA");
	config_memoria->TAM_PAGINA = config_get_int_value(_config, "TAM_PAGINA");
	config_memoria->PATH_INSTRUCCIONES = config_get_string_value(_config, "PATH_INSTRUCCIONES");
	config_memoria->RETARDO_RESPUESTA = config_get_int_value(_config, "RETARDO_RESPUESTA");

	config_memoria->config = _config;

	return config_memoria;
}

void config_memoria_destroy()
{
	// Para asegurarnos que liberamos toda la memoria de una estructura
	// primero borramos lo de adentro y luego lo de afuera

	config_destroy(config_memoria->config); // Primero borramos la config que está adentro
	free(config_memoria);					// Finalmente la estructura que lo contenía
}

void loguear_config_memoria()
{

	loguear("PUERTO_ESCUCHA: %d", config_memoria->PUERTO_ESCUCHA);
	loguear("TAM_MEMORIA: %d", config_memoria->TAM_MEMORIA);
	loguear("TAM_PAGINA: %d", config_memoria->TAM_PAGINA);

	loguear("PATH_INSTRUCCIONES: %s", config_memoria->PATH_INSTRUCCIONES);
	loguear("RETARDO_RESPUESTA: %d", config_memoria->RETARDO_RESPUESTA);
}


bool iniciar_logger_config(char* path_config){
	// en el "memoria.h" se hizo un "#define" con el nombre del MODULO
	decir_hola(MODULO);

	// Iniciamos el logger que es una variable global en las utils
	/*Recuerden que al ser un módulo que se compila separado de los otros módulos,
	 la variable global solo tiene alcance para cada módulo en particular.
	 Por eso nos beneficia tenerlo declarado en un lugar general*/
	logger = iniciar_logger(MODULO); //
	if (logger == NULL)
	{
		printf("EL LOGGER NO PUDO SER INICIADO.\n");
		// Retornamos 'false' indicando que no se inició correctamente
		return false;
	}

	config_memoria = iniciar_config_memoria(path_config);

	if (config_memoria == NULL)
	{
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}

	// Registramos en el log todos los parámetros de la config de memoria
	loguear_config_memoria();
	return true;

}

bool iniciar_servidor_memoria(){

		//Iniciamos el servidor con el puerto indicado en la config
		memoria_escucha= iniciar_servidor(config_memoria->PUERTO_ESCUCHA);
		if(memoria_escucha == -1){
			loguear_error("El servidor no pudo ser iniciado");
			return false;
		}
		loguear("El Servidor iniciado correctamente");
		return true;
}

bool iniciar_conexion_cpu(){

		//Vamos a guardar el socket del cliente que se conecte en esta variable de abajo
		conexion_cpu = esperar_cliente(memoria_escucha);
		if(conexion_cpu == -1){
			loguear_error("Falló la conexión con cpu");
			return false;
		}

		return true;
}

bool iniciar_conexion_kernel(){

		//Vamos a guardar el socket del cliente que se conecte en esta variable de abajo
		conexion_kernel = esperar_cliente(memoria_escucha);
		if(conexion_kernel == -1){
			loguear_error("Falló la conexión con Kernel");
			return false;
		}

		return true;
}

bool iniciar_memoria_instrucciones(){
	pthread_t thread_memoria_procesos;
	pthread_t thread_memoria_instrucciones;//Inicializo el thread

	pthread_create(&thread_memoria_procesos,NULL, (void*)recibir_procesos,NULL);
	pthread_create(&thread_memoria_instrucciones,NULL,(void*)buscar_instrucciones,NULL);
	
	pthread_detach(thread_memoria_procesos);
	if (thread_memoria_procesos == -1){
		loguear_error("No se pudo iniciar la memoria de procesos.");
		return false;
	}
	pthread_join(thread_memoria_instrucciones,NULL);
	if (thread_memoria_instrucciones == -1){
		loguear_error("No se pudo iniciar la memoria de instrucciones.");
		return false;
	}
	return true;
}

bool inicializar_memoria(){
	procesos = dictionary_create();
	return true;
}

bool iniciar_memoria(char *path_config /*acá va la ruta en dónde se hallan las configs*/)
{
	return
		iniciar_logger_config(path_config)&&
		iniciar_servidor_memoria() &&
		inicializar_memoria()&&
		iniciar_conexion_cpu()&&
		iniciar_conexion_kernel()&&
		iniciar_memoria_instrucciones();
}

void proceso_destroy(void* elemento){

	t_proceso* proceso = (t_proceso*)elemento;
	if(proceso!=NULL){
		list_destroy(proceso->instrucciones);
		pcb_destroy(proceso->pcb);
		free(proceso);
	}

}

void finalizar_memoria()
{
	if (config_memoria != NULL)
		config_memoria_destroy();
	if (logger != NULL)
		log_destroy(logger);
	if(procesos!=NULL)
		dictionary_destroy_and_destroy_elements(procesos,proceso_destroy);
}


void list_iterate_loguear(void *element) {
    char *mensaje = (char *)element;
    loguear("%s\n", mensaje);
}

t_list* get_instrucciones_memoria(char* archivo){
	return get_instrucciones(config_memoria->PATH_INSTRUCCIONES,archivo);
}

char *proxima_instruccion_de(t_pcb *pcb){
	t_proceso* proceso = dictionary_get(procesos,string_itoa(pcb->PID));
	return list_get(proceso->instrucciones,pcb->program_counter);

}


void enviar_proxima_instruccion (t_pcb* pcb){
	char* instruccion = proxima_instruccion_de(pcb); 
	enviar_mensaje(instruccion,conexion_cpu);
	pcb_destroy(pcb);	
	usleep(config_memoria->RETARDO_RESPUESTA);
}

int buscar_instrucciones(){
	 while (1) {
         t_paquete *paquete = recibir_paquete(conexion_cpu);
         int cod_op =paquete->codigo_operacion;


		loguear("Cod op: %d", cod_op);
        switch (cod_op) {
            case PROXIMA_INSTRUCCION:
                t_pcb *pcb = recibir_pcb(paquete); 
                enviar_proxima_instruccion(pcb);
				paquete_destroy(paquete);
                break;
            case FIN_PROGRAMA:
			    loguear("Fin programa");
				paquete_destroy(paquete);				
			break;
            case -1:
			loguear_error("el cliente se desconectó. Terminando servidor");
			paquete_destroy(paquete);
			return EXIT_FAILURE;
		    default:
			log_warning(logger,"Operación desconocida. No quieras meter la pata");
			paquete_destroy(paquete);
			return EXIT_FAILURE;
        }

    }
}

bool tiene_exit(t_list* instrucciones){
	return list_any_satisfy(instrucciones,es_exit);
}

bool crear_proceso( t_pcb *pcb ){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->pcb=pcb;
	proceso->instrucciones = get_instrucciones_memoria(pcb->path);

	if(proceso->instrucciones==NULL){
		loguear_error("El programa asociado no existe.");
		return false;
	}

	if(tiene_exit(proceso->instrucciones))
		dictionary_put(procesos,string_itoa(pcb->PID),proceso);
	else{
		loguear_error("El programa asociado no tiene %s",EXIT_PROGRAM);
		return false;
	}
	return true;
}

int recibir_procesos(){
	 while (1) {
		  t_paquete *paquete = recibir_paquete(conexion_kernel);
		 int cod_op =paquete->codigo_operacion;
		loguear("Cod op: %d", cod_op);
        switch (cod_op) {
			case CREACION_PROCESO:
			  t_pcb *pcb = recibir_pcb(paquete); 
			  bool proceso_creado = crear_proceso(pcb);
			  if(proceso_creado)	
			  	enviar_texto("Proceso creado",CREACION_PROCESO,conexion_kernel);
				else	
			{	
				enviar_texto("No se pudo crear el proceso",CREACION_PROCESO_FALLIDO,conexion_kernel);
				pcb_destroy(pcb);
				paquete_destroy(paquete);
			}	  
			 
			break;
			case ELIMINACION_PROCESO:
			break;
			case -1:
			loguear_error("el cliente se desconectó. Terminando servidor");
			paquete_destroy(paquete);
			return EXIT_FAILURE;
		    default:
			log_warning(logger,"Operación desconocida. No quieras meter la pata");
			paquete_destroy(paquete);
			return EXIT_FAILURE;

		}
	 }
}