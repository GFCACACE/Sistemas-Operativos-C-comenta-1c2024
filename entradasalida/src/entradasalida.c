#include "entradasalida.h"


int conexion_memoria, conexion_kernel;
int cod_op_kernel,cod_op_memoria;
t_config_io* config;
t_queue* cola_peticiones_io;
pthread_mutex_t mx_peticion = PTHREAD_MUTEX_INITIALIZER; 
sem_t sem_bin_cola_peticiones; 

t_config_io* iniciar_config_io(char* path_config,char* nombre){
	t_config* _config = config_create(path_config);
	if(_config ==NULL)
		return NULL;
	t_config_io* config_io = malloc(sizeof(t_config_io));	
	// Los siguientes valores le son comunes a todos los tipos de interfaces, por lo cual los inicializamos siempre
	config_io->TIPO_INTERFAZ = config_get_string_value(_config,"TIPO_INTERFAZ");
	config_io->IP_KERNEL = config_get_string_value(_config,"IP_KERNEL");
	config_io->PUERTO_KERNEL = config_get_int_value(_config,"PUERTO_KERNEL");

	// Los siguientes casos se pueden modularizar en funciones. No lo hice asi xq no sabia bien como :/
	// CASO GENERICA
	if (!strcmp("GENERICA",config_io->TIPO_INTERFAZ)){
		config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
	}
	//CASO STDIN
	if (!strcmp("STDIN",config_io->TIPO_INTERFAZ)){
		config_io->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
		config_io->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
	}
	//CASO STDOUT
	if (!strcmp("STDOUT",config_io->TIPO_INTERFAZ)){
		config_io->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
		config_io->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
		config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
	}
	//CASO DIALFS
	if (!strcmp("DIALFS",config_io->TIPO_INTERFAZ)){
		config_io->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
		config_io->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
		config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
		config_io->PATH_BASE_DIALFS = config_get_string_value(_config,"PATH_BASE_DIALFS");
		config_io->BLOCK_SIZE = config_get_int_value(_config,"BLOCK_SIZE");
		config_io->BLOCK_COUNT = config_get_int_value(_config,"BLOCK_COUNT");
	}
	config_io->NOMBRE = string_new();
	config_io->NOMBRE = string_duplicate(nombre);
	config_io->config = _config;

	return config_io;
}


void loguear_config(){

	loguear("NOMBRE INTERFAZ: %s", config->NOMBRE);
	loguear("TIPO_INTERFAZ: %s",config->TIPO_INTERFAZ);
	loguear("IP_KERNEL: %s",config->IP_KERNEL);
    loguear("PUERTO_KERNEL: %d",config->PUERTO_KERNEL);
	if (!strcmp("GENERICA",config->TIPO_INTERFAZ)) loguear_config_generica();
	if (!strcmp("STDIN",config->TIPO_INTERFAZ)) loguear_config_stdin();
	if (!strcmp("STDOUT",config->TIPO_INTERFAZ)) loguear_config_stdout();
	if (!strcmp("DIALFS",config->TIPO_INTERFAZ)) loguear_config_dialfs();

}

void loguear_config_generica(){
	loguear("TIEMPO_UNIDAD_TRABAJO: %d",config->TIEMPO_UNIDAD_TRABAJO);
}
void loguear_config_stdin(){
	loguear("IP_MEMORIA: %s",config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d",config->PUERTO_MEMORIA);
}

void loguear_config_stdout(){
	loguear("TIEMPO_UNIDAD_TRABAJO: %d",config->TIEMPO_UNIDAD_TRABAJO);
	loguear("IP_MEMORIA: %s",config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d",config->PUERTO_MEMORIA);
}
void loguear_config_dialfs(){
	loguear("TIEMPO_UNIDAD_TRABAJO: %d",config->TIEMPO_UNIDAD_TRABAJO);
	loguear("IP_MEMORIA: %s",config->IP_MEMORIA);
	loguear("PUERTO_MEMORIA: %d",config->PUERTO_MEMORIA);
	loguear("PATH_BASE_DIALFS: %s",config->PATH_BASE_DIALFS);
	loguear("BLOCK_SIZE: %d",config->BLOCK_SIZE);
    loguear("BLOCK_COUNT: %d",config->BLOCK_COUNT);
}

bool iniciar_log_config(char* path_config, char* nombre){
    logger = iniciar_logger(MODULO);

	if(logger == NULL) printf("EL LOGGER NO PUDO SER INICIADO.\n");
	
    config = iniciar_config_io(path_config, nombre);
	
    if(config == NULL) {
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}
	loguear_config();	    
   
	return true;
}


bool iniciar_conexion_kernel(){
    conexion_kernel = crear_conexion(config->IP_KERNEL, config->PUERTO_KERNEL);
	if(conexion_kernel ==-1){
		
		loguear_error("No se pudo conectar kernel");
		return false;
	}
	enviar_texto(config->NOMBRE,NUEVA_IO,conexion_kernel);
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
bool iniciar_semaforo_y_cola(){
	sem_init(&sem_bin_cola_peticiones, 0, 0);
	cola_peticiones_io = queue_create();
	return true;
}

bool iniciar_hilo_ejecutar_io(){
	pthread_t thread_io;
	pthread_create(&thread_io,NULL, (void*) ejecutar_op_io, NULL);							
	pthread_detach(thread_io);

	if (thread_io == -1){
		loguear_error("No se pudo iniciar el hilo de ejecucion de la interfaz.");
		return false;
	}
	return true;
}

bool iniciar_io(char* path_config, char* nombre){
    return iniciar_log_config(path_config, nombre)
    // && iniciar_conexion_memoria()
	&& iniciar_hilo_ejecutar_io()
	&& iniciar_semaforo_y_cola()
    && iniciar_conexion_kernel();
}


void config_io_destroy(t_config_io* config){

	config_destroy(config->config);
	free(config);
}


void finalizar_io(){
	if(config != NULL) config_io_destroy(config);
	if(logger != NULL) log_destroy(logger);
	sem_destroy(&sem_bin_cola_peticiones);
	pthread_mutex_destroy(&mx_peticion);
}

void recibir_io(){
	loguear("IO conectada: Esperando ordenes");
	
	while(1){
		t_peticion_io* peticion_io = malloc(sizeof(t_peticion_io));
		int cod_op_io = recibir_operacion(conexion_kernel);
		peticion_io->cod_op = cod_op_io;
		char* _peticion;
		_peticion = recibir_mensaje(conexion_kernel);
		peticion_io->peticion = strdup(_peticion);
		loguear_warning("Aca se ve la PETICION %s", peticion_io->peticion);
		pthread_mutex_lock(&mx_peticion);
		queue_push(cola_peticiones_io, peticion_io);
		pthread_mutex_unlock(&mx_peticion);
		sem_post(&sem_bin_cola_peticiones);


		free(_peticion);
		
	}

}
int ejecutar_op_io()
{

	loguear("Ejecuta operacion de entrada salida");
	while (1)
	{
		t_peticion_io* peticion_io = malloc(sizeof(t_peticion_io));
		sem_wait(&sem_bin_cola_peticiones);
		pthread_mutex_lock(&mx_peticion);
		peticion_io = queue_pop(cola_peticiones_io);
		pthread_mutex_unlock(&mx_peticion);
		int cod_op = peticion_io->cod_op;
		char* _peticion;
		_peticion = peticion_io->peticion;
		char** splitter = string_array_new();
		splitter = string_split(_peticion," ");
		loguear("Cod op: %d", cod_op);
		// ESTAMOS BIEN
		char mensaje[70];

        switch (cod_op) {
            case IO_GEN_SLEEP:
				
				sprintf(mensaje,"PID: <%s> - Operacion: <IO_GEN_SLEEP> - Unidades de trabajo: %s",splitter[0],splitter[1]);
				//loguear(mensaje);
				loguear("PID: <%s> - Operacion: <IO_GEN_SLEEP> - Unidades de trabajo: %s",splitter[0],splitter[1]);
				io_gen_sleep(atoi(splitter[1]));
				//loguear_warning("Ya termino de dormir zzzzz");
				enviar_texto(_peticion,TERMINO_IO,conexion_kernel);
				loguear_warning("Termino el io_gen_sleep");
                break;			
			case IO_STDIN_READ:
				
				sprintf(mensaje,"PID: <%s> - Operacion: <IO_STDIN_READ> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
				//loguear(mensaje);
				loguear("PID: <%s> - Operacion: <IO_STDIN_READ> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
				io_stdin_read((uint32_t)atoi(splitter[0]), (uint32_t)atoi(splitter[1]), (uint32_t) atoi(splitter[2]));
				enviar_texto(_peticion,TERMINO_IO,conexion_kernel);
				loguear_warning("Termino el io_stdin_read");
                break;
			case IO_STDOUT_WRITE:
				
				sprintf(mensaje,"PID: <%s> - Operacion: <IO_STDOUT_WRITE> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
				//loguear(mensaje);
				loguear("PID: <%s> - Operacion: <IO_STDOUT_WRITE> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
				io_stdout_write((uint32_t)atoi(splitter[0]),(uint32_t)atoi(splitter[1]), (uint32_t) atoi(splitter[2]));
				enviar_texto(_peticion,TERMINO_IO,conexion_kernel);
				loguear_warning("Termino el io_stdin_read");
                break;	
            case -1:
				loguear_error("Problemas en la comunicacion con el servidor. Cerrando conexion...");
				return EXIT_FAILURE;
			default:
				log_warning(logger, "Operacion desconocida. No quieras meter la pata");
				return EXIT_FAILURE;
		}
		free(peticion_io);
	}
}