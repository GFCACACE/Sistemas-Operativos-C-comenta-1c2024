#include "entradasalida.h"

/////BORRAR
int registro_reconstr;
int registro_reconstr_leer;
void* registro_puntero_recons = &registro_reconstr;
void* registro_puntero_recons_leer = &registro_reconstr_leer;
uint32_t  size_leido=0;
uint32_t size_leido_leer=0;
/////BORRAR

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
	config_io->TIPO_INTERFAZ = get_tipo_io(config_get_string_value(_config,"TIPO_INTERFAZ"));
	config_io->IP_KERNEL = config_get_string_value(_config,"IP_KERNEL");
	config_io->PUERTO_KERNEL = config_get_int_value(_config,"PUERTO_KERNEL");

	// Los siguientes casos se pueden modularizar en funciones. No lo hice asi xq no sabia bien como :/
	// CASO GENERICA
	if (GENERICA==config_io->TIPO_INTERFAZ.id){
		config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
	}
	//CASO STDIN
	if (STDIN==config_io->TIPO_INTERFAZ.id){
		config_io->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
		config_io->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
	}
	//CASO STDOUT
	if (STDOUT==config_io->TIPO_INTERFAZ.id){
		config_io->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
		config_io->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
		config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
	}
	//CASO DIALFS
	if (DIALFS==config_io->TIPO_INTERFAZ.id){
		config_io->IP_MEMORIA = config_get_string_value(_config,"IP_MEMORIA");
		config_io->PUERTO_MEMORIA = config_get_int_value(_config,"PUERTO_MEMORIA");
		config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(_config,"TIEMPO_UNIDAD_TRABAJO");
		config_io->PATH_BASE_DIALFS = config_get_string_value(_config,"PATH_BASE_DIALFS");
		config_io->BLOCK_SIZE = config_get_int_value(_config,"BLOCK_SIZE");
		config_io->BLOCK_COUNT = config_get_int_value(_config,"BLOCK_COUNT");
		config_io->RETRASO_COMPACTACION = config_get_int_value(_config,"RETRASO_COMPACTACION");
	}
	config_io->NOMBRE = string_new();
	config_io->NOMBRE = string_duplicate(nombre);
	config_io->config = _config;
	if(config_io->TIPO_INTERFAZ.seleccionar_io ==NULL)
	{	
		free(config_io->NOMBRE);
		config_io_destroy(config_io);
		return NULL;
	}


	return config_io;
}

char* devuelve_tipo_en_char(t_interfaz tipo_interfaz){
	if(tipo_interfaz == GENERICA)
		return "GENERICA";
	else if(tipo_interfaz == STDIN)
		return "STDIN";
	else if(tipo_interfaz == STDOUT)
		return "STDOUT";
	else if(tipo_interfaz == DIALFS)
		return "DIALFS";
}


void loguear_config(){

	loguear("NOMBRE INTERFAZ: %s", config->NOMBRE);
	loguear("TIPO_INTERFAZ: %s",devuelve_tipo_en_char(config->TIPO_INTERFAZ.id));
	loguear("IP_KERNEL: %s",config->IP_KERNEL);
    loguear("PUERTO_KERNEL: %d",config->PUERTO_KERNEL);
	if (GENERICA==config->TIPO_INTERFAZ.id) loguear_config_generica();
	if (STDIN==config->TIPO_INTERFAZ.id) loguear_config_stdin();
	if (STDOUT==config->TIPO_INTERFAZ.id) loguear_config_stdout();
	if (DIALFS==config->TIPO_INTERFAZ.id) loguear_config_dialfs();
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
	loguear("RETRASO_COMPACTACION: %d",config->RETRASO_COMPACTACION);
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
	char* texto = string_new();

	if(conexion_kernel ==-1){
		
		loguear_error("No se pudo conectar kernel");
		return false;
	}
	sprintf(texto,"%s %s",config->NOMBRE,devuelve_tipo_en_char(config->TIPO_INTERFAZ.id));
	enviar_texto(texto,NUEVA_IO,conexion_kernel);
    return true;
}


bool iniciar_conexion_memoria(){
	if(devuelve_tipo_en_char(config->TIPO_INTERFAZ.id) != "GENERICA"){
		conexion_memoria = crear_conexion(config->IP_MEMORIA,config->PUERTO_MEMORIA);
		char* texto = string_new();
		
		if(conexion_memoria ==-1){
			loguear_error("No se pudo conectar memoria");
			return false;
		} 
		sprintf(texto,"%s %s",config->NOMBRE,devuelve_tipo_en_char(config->TIPO_INTERFAZ.id));
		enviar_texto(texto,NUEVA_IO,conexion_memoria);
		return true;
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
	pthread_create(&thread_io,NULL, (void*) ejecutar_selector_io, NULL);							
	pthread_detach(thread_io);

	if (thread_io == -1){
		loguear_error("No se pudo iniciar el hilo de ejecucion de la interfaz.");
		return false;
	}
	return true;
}

bool iniciar_io(char* path_config, char* nombre){
    return iniciar_log_config(path_config, nombre)
	&& iniciar_archivos_dialfs()
    && iniciar_conexion_memoria()
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

// void recibir_io(){
// 	loguear("IO conectada: Esperando ordenes");
	
// 	while(1){
// 		t_peticion_io* peticion_io = malloc(sizeof(t_peticion_io));
// 		int cod_op_io = recibir_operacion(conexion_kernel);		
// 		if(cod_op_io==-1){
// 			loguear_warning("Kernel se desconectó.");
// 			free(peticion_io);
// 			return;
// 		}
// 		peticion_io->cod_op = cod_op_io;
// 		char* _peticion;
// 		_peticion = recibir_mensaje(conexion_kernel);
// 		peticion_io->peticion = strdup(_peticion);
// 		loguear_warning("Aca se ve la PETICION %s", peticion_io->peticion);
// 		pthread_mutex_lock(&mx_peticion);
// 		queue_push(cola_peticiones_io, peticion_io);
// 		pthread_mutex_unlock(&mx_peticion);
// 		sem_post(&sem_bin_cola_peticiones);
// 		free(_peticion);	
// 	}

// }

void recibir_io(){
	loguear("IO conectada: Esperando ordenes");
	
	while(1){	
		if(config->TIPO_INTERFAZ.id == GENERICA){
			t_peticion_io* peticion_io = malloc(sizeof(t_peticion_io));
			int cod_op_io = recibir_operacion(conexion_kernel);		
			if(cod_op_io==-1){
				loguear_warning("Kernel se desconectó.");
				free(peticion_io);
				return;
			}
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

		else if(config->TIPO_INTERFAZ.id == STDIN || config->TIPO_INTERFAZ.id == STDOUT ){
			t_paquete* paquete = recibir_paquete(conexion_kernel);
			int cod_op_io = paquete->codigo_operacion;
			if(cod_op_io==-1){
				loguear_warning("Kernel se desconectó.");
				paquete_destroy(paquete);
				return;
			}
			t_direcciones_proceso* direcciones_proceso = recibir_direcciones_proceso(paquete);
			pthread_mutex_lock(&mx_peticion);
			queue_push(cola_peticiones_io, direcciones_proceso);
			pthread_mutex_unlock(&mx_peticion);
			sem_post(&sem_bin_cola_peticiones);
			paquete_destroy(paquete);	
		}
	}
}

void ejecutar_selector_io(){
	config->TIPO_INTERFAZ.seleccionar_io();
}

t_selector_io get_tipo_io(char* nombre){


	 // Crear el diccionario de algoritmos
    t_dictionary *tipo_ios = dictionary_create();
	t_selector_io selector_io;

    // Función para agregar un algoritmo al diccionario

    void _agregar(char* _nombre, t_interfaz tipo,void(*funcion)(void)){
		t_selector_io* select = malloc(sizeof(t_selector_io));
		select->id = tipo;
		select->seleccionar_io = funcion; 	
        dictionary_put(tipo_ios, _nombre, select);
    };

	_agregar("STDIN",STDIN,&ejecutar_op_io_stdin);
	_agregar("STDOUT",STDOUT,&ejecutar_op_io_stdout);
	_agregar("GENERICA",GENERICA,&ejecutar_op_io_generica);
	_agregar("DIALFS",DIALFS,&ejecutar_op_io_dialfs);

	t_selector_io* selector_io_ptr= (t_selector_io*)dictionary_get(tipo_ios, nombre);
	if(selector_io_ptr==NULL)	
	{	
		perror("Tipo de funcion IO no encontrada");

		exit(EXIT_FAILURE);
	}
	else selector_io=*selector_io_ptr;
	
	dictionary_destroy_and_destroy_elements(tipo_ios,free);
	return selector_io;

}

bool es_selector(t_interfaz tipo_interfaz){
	return config->TIPO_INTERFAZ.id == tipo_interfaz;
}

bool es_stdin(){
	return es_selector(STDIN);
}
bool es_stdout(){
	return es_selector(STDOUT);
}
bool es_dialfs(){
	return es_selector(DIALFS);
}
bool es_generica(){
	return es_selector(GENERICA);
}

int ejecutar_op_io_stdin(){
	while(1){
		
		sem_wait(&sem_bin_cola_peticiones);
		pthread_mutex_lock(&mx_peticion);
		t_direcciones_proceso* direcciones_proceso = queue_pop(cola_peticiones_io);
		pthread_mutex_unlock(&mx_peticion);
		char* pid = string_itoa(direcciones_proceso->pid_size_total.PID);
		io_stdin_read(direcciones_proceso, conexion_memoria);

		enviar_texto(pid,TERMINO_IO,conexion_kernel);
		loguear_warning("Termino el IO_STDIN_READ.");
		free(pid);
		direcciones_proceso_destroy(direcciones_proceso);
	}
}
int ejecutar_op_io_stdout(){
	while(1){
		
		sem_wait(&sem_bin_cola_peticiones);
		pthread_mutex_lock(&mx_peticion);
		t_direcciones_proceso* direcciones_proceso = queue_pop(cola_peticiones_io);
		pthread_mutex_unlock(&mx_peticion);
		char* pid = string_itoa(direcciones_proceso->pid_size_total.PID);
		io_stdout_write(direcciones_proceso,conexion_memoria);
		enviar_texto(pid,TERMINO_IO,conexion_kernel);
		loguear_warning("Termino el IO_STDOUT_WRITE.");
		free(pid);
		direcciones_proceso_destroy(direcciones_proceso);
	}
}

int ejecutar_op_io_dialfs(){

}

int ejecutar_op_io_generica(){
	while(1){
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

		sprintf(mensaje,"PID: <%s> - Operacion: <IO_GEN_SLEEP> - Unidades de trabajo: %s",splitter[0],splitter[1]);
		//loguear(mensaje);
		loguear("PID: <%s> - Operacion: <IO_GEN_SLEEP> - Unidades de trabajo: %s",splitter[0],splitter[1]); // LOG MÍNIMO Y OBLIGATORIO
		io_gen_sleep(atoi(splitter[1]));
		//loguear_warning("Ya termino de dormir zzzzz");
		enviar_texto(splitter[0],TERMINO_IO,conexion_kernel);
		loguear_warning("Termino el IO_GEN_SLEEP.");

		string_array_destroy(splitter);
		free(peticion_io);
	}

}


// int ejecutar_op_io()
// {

// 	loguear("Ejecuta operacion de entrada salida");
// 	while (1)
// 	{
// 		t_peticion_io* peticion_io = malloc(sizeof(t_peticion_io));
// 		sem_wait(&sem_bin_cola_peticiones);
// 		pthread_mutex_lock(&mx_peticion);
// 		peticion_io = queue_pop(cola_peticiones_io);
// 		pthread_mutex_unlock(&mx_peticion);
// 		int cod_op = peticion_io->cod_op;
// 		char* _peticion;
// 		_peticion = peticion_io->peticion;
// 		char** splitter = string_array_new();
// 		splitter = string_split(_peticion," ");
// 		loguear("Cod op: %d", cod_op);
// 		ESTAMOS BIEN
// 		char mensaje[70];

//         switch (cod_op) {
//             case IO_GEN_SLEEP:
				
// 				sprintf(mensaje,"PID: <%s> - Operacion: <IO_GEN_SLEEP> - Unidades de trabajo: %s",splitter[0],splitter[1]);
// 				loguear(mensaje);
// 				loguear("PID: <%s> - Operacion: <IO_GEN_SLEEP> - Unidades de trabajo: %s",splitter[0],splitter[1]);
// 				io_gen_sleep(atoi(splitter[1]));
// 				loguear_warning("Ya termino de dormir zzzzz");
// 				enviar_texto(_peticion,TERMINO_IO,conexion_kernel);
// 				loguear_warning("Termino el IO_GEN_SLEEP.");
//                 break;			
// 			case IO_STDIN_READ:
				
// 				sprintf(mensaje,"PID: <%s> - Operacion: <IO_STDIN_READ> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
// 				loguear(mensaje);
// 				loguear("PID: <%s> - Operacion: <IO_STDIN_READ> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
// 				io_stdin_read((uint32_t)atoi(splitter[0]), (uint32_t)atoi(splitter[1]), (uint32_t) atoi(splitter[2]));
// 				enviar_texto(_peticion,TERMINO_IO,conexion_kernel);
// 				loguear_warning("Termino el IO_STDIN_READ.");
//                 break;
// 			case IO_STDOUT_WRITE:
				
// 				sprintf(mensaje,"PID: <%s> - Operacion: <IO_STDOUT_WRITE> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
// 				loguear(mensaje);
// 				loguear("PID: <%s> - Operacion: <IO_STDOUT_WRITE> - Direccion: %s Tamanio: %s",splitter[0],splitter[1], splitter[2]);
// 				io_stdout_write((uint32_t)atoi(splitter[0]),(uint32_t)atoi(splitter[1]), (uint32_t) atoi(splitter[2]));
// 				enviar_texto(_peticion,TERMINO_IO,conexion_kernel);
// 				loguear_warning("Termino el IO_STDOUT_WRITE.");
//                 break;	
//             case -1:
// 				loguear_error("Problemas en la comunicacion con el servidor. Cerrando conexion...");
// 				free(peticion_io);
// 				return EXIT_FAILURE;
// 			default:
// 				log_warning(logger, "Operacion desconocida. No quieras meter la pata");
// 				free(peticion_io);
// 				return EXIT_FAILURE;
// 		}
// 		free(peticion_io);
// 	}
// }
