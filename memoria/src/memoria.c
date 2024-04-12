#include "memoria.h"

int memoria_escucha,conexion_cpu, conexion_kernel;
t_config_memoria* config_memoria;

t_config_memoria* iniciar_config_memoria(char* config_path){

	t_config* _config = config_create(config_path);
	if(_config ==NULL)
		return NULL;
	t_config_memoria* config_memoria = malloc(sizeof(t_config_memoria));	

	config_memoria->PUERTO_ESCUCHA = config_get_int_value(_config,"PUERTO_ESCUCHA");
	config_memoria->TAM_MEMORIA = config_get_int_value(_config,"TAM_MEMORIA");
	config_memoria->TAM_PAGINA = config_get_int_value(_config,"TAM_PAGINA");
	config_memoria->PATH_INSTRUCCIONES = config_get_string_value(_config,"PATH_INSTRUCCIONES");
	config_memoria->RETARDO_RESPUESTA = config_get_int_value(_config,"RETARDO_RESPUESTA");

	config_memoria->config=_config;

	return config_memoria;
}



void config_memoria_destroy(){
	//Para asegurarnos que liberamos toda la memoria de una estructura 
	//primero borramos lo de adentro y luego lo de afuera

	config_destroy(config_memoria->config);//Primero borramos la config que está adentro
	free(config_memoria); // Finalmente la estructura que lo contenía
}

void loguear_config_memoria(){

	loguear("PUERTO_ESCUCHA: %d",config_memoria->PUERTO_ESCUCHA);
	loguear("TAM_MEMORIA: %d",config_memoria->TAM_MEMORIA);
	loguear("TAM_PAGINA: %d",config_memoria->TAM_PAGINA);

	loguear("PATH_INSTRUCCIONES: %s",config_memoria->PATH_INSTRUCCIONES);
	loguear("RETARDO_RESPUESTA: %d",config_memoria->RETARDO_RESPUESTA);
}

bool iniciar_memoria(char* path_config/*acá va la ruta en dónde se hallan las configs*/){
	
	// en el "memoria.h" se hizo un "#define" con el nombre del MODULO
    decir_hola(MODULO);

    //Iniciamos el logger que es una variable global en las utils
	/*Recuerden que al ser un módulo que se compila separado de los otros módulos,
	 la variable global solo tiene alcance para cada módulo en particular.
	 Por eso nos beneficia tenerlo declarado en un lugar general*/
	logger= iniciar_logger(MODULO);//
	if(logger ==NULL ){
		printf("EL LOGGER NO PUDO SER INICIADO.\n");
		//Retornamos 'false' indicando que no se inició correctamente
		return false;
	} 

	config_memoria = iniciar_config_memoria(path_config);
	
	if(config_memoria == NULL){ 
		loguear_error("No se encuentra el archivo de las config");
		return false;
	}

	//Registramos en el log todos los parámetros de la config de memoria
	loguear_config_memoria();   

	//Iniciamos el servidor con el puerto indicado en la config
    memoria_escucha= iniciar_servidor(config_memoria->PUERTO_ESCUCHA);
	if(memoria_escucha == -1){
		loguear_error("El servidor no pudo ser iniciado");
		return false;
	}
    loguear("El Servidor iniciado correctamente");

	//Vamos a guardar el socket del cliente que se conecte en esta variable de abajo
    conexion_cpu = esperar_cliente(memoria_escucha);
	if(conexion_cpu == -1){
		loguear_error("Falló la conexión con cpu");
		return false;
	}
    conexion_kernel = esperar_cliente(memoria_escucha);
	if(conexion_kernel == -1){
		loguear_error("Falló la conexión con kernel");
		return false;
	}
	return true;
}

void finalizar_memoria(){
	if(config_memoria != NULL) config_memoria_destroy();
	if(logger != NULL) log_destroy(logger);
}

t_dictionary* crear_procesos_prueba(){

	t_dictionary * procesosCargados  = dictionary_create();

	t_list * comandos_01 = list_create();
	list_add(comandos_01,"SUM AX BX");
	list_add(comandos_01,"MOV CX AX");
	list_add(comandos_01,"SET CX 3");
	list_add(comandos_01,"EXIT");

	t_list * comandos_02 = list_create();
	list_add(comandos_02,"SUM AX BX");
	list_add(comandos_02,"MOV CX AX");
	list_add(comandos_02,"SET CX 3");
	list_add(comandos_02,"EXIT");		

	t_list * comandos_03 = list_create();
	list_add(comandos_03,"SUM AX BX");
	list_add(comandos_03,"MOV CX AX");
	list_add(comandos_03,"SET CX 3");	
	list_add(comandos_03,"EXIT");


	dictionary_put(procesosCargados,"1",comandos_01);
	dictionary_put(procesosCargados,"2",comandos_02);
	dictionary_put(procesosCargados,"3",comandos_03);

	return procesosCargados;
}

char* proxima_instruccion_de(t_pcb* pcb){
	char* proxima_instruccion = "";
	char pid[20];       
    sprintf(pid, "%d", pcb->PID);
	t_dictionary* procesos = crear_procesos_prueba();
	void list_iterate_loguear(void *element) {
		char *mensaje = (char *)element;
		loguear("%s\n", mensaje);
	}
	if(dictionary_has_key(procesos,pid)){
		t_list* lista = (t_list*) dictionary_get(procesos, pid);
		list_iterate(lista,list_iterate_loguear);
		proxima_instruccion = list_get(lista,pcb->program_counter);
		loguear("Próxima instruccción: %s",proxima_instruccion);
		
	}
	else{
		loguear("No hay comandos para el PID:%s, %d",pid,pcb->PID);
	
	}

	return proxima_instruccion;
}
