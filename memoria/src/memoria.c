#include "memoria.h"

/* Cuando tengamos que recorrer la memoria, o hacer algo,
nos paramos en memoriaPrincipal + nroMarco * tam_marco + offset 
*/

int memoria_escucha, conexion_cpu, conexion_kernel;
t_config_memoria *config_memoria;
t_dictionary *procesos;

void* memoriaPrincipal;
t_list* frames; //Lista que sabe que numero de frame está en uso y cual no en toda la memoria
int cantidadFrames;
int tamanio_pagina;


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
		char* tamanio_pagina = string_new();
		sprintf(tamanio_pagina,"%d",config_memoria->TAM_PAGINA);
		enviar_texto(tamanio_pagina,TAMANIO_PAGINA,conexion_cpu);
		free(tamanio_pagina);
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
bool iniciar_paginacion(){
	
	memoriaPrincipal = malloc(config_memoria->TAM_MEMORIA);
	
	tamanio_pagina = config_memoria-> TAM_PAGINA;
	if(memoriaPrincipal == NULL){
		loguear_error("No se reservó la memoria correctamente");
	}
	cantidadFrames = config_memoria->TAM_MEMORIA/tamanio_pagina; 
	
	
	crear_frames_memoria_principal(cantidadFrames);
	//imprimir_uso_frames();
	
	loguear("Espacio memoria total: %d",config_memoria->TAM_MEMORIA);
	loguear("El tamanio de pagina es: %d",tamanio_pagina);
	
	loguear("Cantidad frames %d",cantidadFrames);
	return true;
}
bool iniciar_memoria(char *path_config /*acá va la ruta en dónde se hallan las configs*/)
{
	return
		iniciar_logger_config(path_config)&&
		iniciar_servidor_memoria() &&
		inicializar_memoria()&&
		iniciar_paginacion()&&
		iniciar_conexion_cpu()&&
		iniciar_conexion_kernel()&&
		iniciar_memoria_instrucciones();
}

void proceso_destroy(void* elemento){

	t_proceso* proceso = (t_proceso*)elemento;
	if(proceso){
		list_destroy(proceso->instrucciones);
		pcb_destroy(proceso->pcb);
		free(proceso);
	}

}

void finalizar_memoria()
{
	if (config_memoria)
		config_memoria_destroy();
	if (logger)
		log_destroy(logger);
	if(procesos)
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
			case RESIZE:
				t_tamanio_proceso* tamanio_proceso =  recibir_tamanio_proceso(paquete);
				t_proceso* proceso_en_memoria = dictionary_get(procesos,string_itoa(tamanio_proceso->PID));
				t_list* tabla_de_paginas_proceso = proceso_en_memoria->tabla_paginas;
				int pag_solictadas_respecto_actual = diferencia_tamaño_nuevo_y_actual(tabla_de_paginas_proceso,tamanio_proceso);   // Devuelve la diferencia entre la cantidad de paginas solicitadas y las que actualmente tiene el proceso
				
					if(pag_solictadas_respecto_actual>0){ //AMPLIACION_PROCESO
						if(validar_ampliacion_proceso(pag_solictadas_respecto_actual)){
							ampliar_proceso(tabla_de_paginas_proceso,pag_solictadas_respecto_actual);
						}else{
							return OUT_OF_MEMORY;
						}
					}
					if(pag_solictadas_respecto_actual<0){ //REDUCCION_PROCESO
						int paginas_a_reducir = abs(pag_solictadas_respecto_actual);
						reducir_proceso(tabla_de_paginas_proceso,paginas_a_reducir);
					}
				
				paquete_destroy(paquete);
			case ACCESO_TABLA_PAGINAS:
				tamanio_proceso =  recibir_tamanio_proceso(paquete);
				proceso_en_memoria = dictionary_get(procesos,string_itoa(tamanio_proceso->PID));
				tabla_de_paginas_proceso = proceso_en_memoria->tabla_paginas;
				int nro_pagina = tamanio_proceso->tamanio;

				return obtener_frame(tabla_de_paginas_proceso, nro_pagina);


			case LECTURA_MEMORIA:
				//int direccion_fisica = atoi(recibir_mensaje(paquete));
				// nos paramos en memoriaPrincipal + nroMarco * tam_marco + offset 
				//int dato_consultado;	
				
				//memcpy(&dato_consultado,direccion_fisica,sizeof(direccion_fisica));
				
				//enviar_texto(dato_consultado,VALOR_CONSULTA_CPU,conexion_cpu);
				
	
				
	
	
				//nos paramos en memoriaPrincipal + nroMarco * tam_marco + offset 
				break;

			case ESCRITURA_MEMORIA:
			//TODO


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

t_validacion* crear_proceso( t_pcb *pcb ){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->pcb=pcb;
	proceso->instrucciones = get_instrucciones_memoria(pcb->path);

	t_validacion* validacion = validacion_new();
	
	if(proceso->instrucciones==NULL){
		validacion->descripcion = "El programa asociado no existe: %s";
		loguear_error(validacion->descripcion,pcb->path);
		return validacion;
	}

	if(tiene_exit(proceso->instrucciones))
	{	dictionary_put(procesos,string_itoa(pcb->PID),proceso);
		validacion->descripcion = "Programa cargado en memoria";
		validacion->resultado = true;
	}
	else{
		validacion->descripcion = "El programa asociado no tiene %s";
		loguear_error(validacion->descripcion,EXIT_PROGRAM);
	}
	
	return validacion;
}

t_validacion* eliminar_proceso( t_pcb *pcb ){
	t_proceso* proceso = (t_proceso*)dictionary_get(procesos, string_itoa(pcb->PID));
	t_validacion* validacion = validacion_new();
	if(proceso){
		dictionary_remove_and_destroy(procesos,string_itoa(pcb->PID),proceso_destroy);
		validacion->descripcion = "Programa removido de memoria";
		validacion->resultado = true;
	}
	else
	{	
		validacion->descripcion = "El proceso %d no existe en memoria";
		loguear_error(validacion->descripcion,pcb->PID,EXIT_PROGRAM);
	}

	return validacion;
}

void avisar_a_kernel(op_code codigo_operacion,char*texto){
	enviar_texto(texto,codigo_operacion,conexion_kernel);
}

void notificar_proceso_am(t_validacion* validacion,t_paquete* paquete,t_pcb* pcb,op_code codigo_ok,op_code codigo_error){
	void avisar(op_code codigo_operacion){
		avisar_a_kernel(codigo_operacion,validacion->descripcion);
	}

	if(validacion->resultado)	
		avisar(codigo_ok);
	else	
	{	
		avisar(codigo_error);
		pcb_destroy(pcb);
		paquete_destroy(paquete);
	}	

	free(validacion);
}

void notificar_proceso_creado(t_validacion* validacion,t_paquete* paquete,t_pcb* pcb){

	notificar_proceso_am(validacion,paquete,pcb,CREACION_PROCESO,CREACION_PROCESO_FALLIDO);
}


void notificar_proceso_eliminado(t_validacion* validacion,t_paquete* paquete,t_pcb* pcb){

	notificar_proceso_am(validacion,paquete,pcb,ELIMINACION_PROCESO,ELIMINACION_PROCESO_FALLIDO);
}

void recibir_pcb_y_aplicar(t_paquete *paquete,t_validacion* (*accion)(t_pcb*),void (*notificador)(t_validacion*,t_paquete*,t_pcb*) ){
	 t_pcb *pcb = recibir_pcb(paquete); 
	t_validacion* validacion = accion(pcb);
	notificador(validacion,paquete,pcb);	
}


int recibir_procesos(){
	 while (1) {
		t_paquete *paquete = recibir_paquete(conexion_kernel);
		int cod_op =paquete->codigo_operacion;
		loguear("Cod op: %d", cod_op);
        switch (cod_op) {
			case CREACION_PROCESO:
			 	recibir_pcb_y_aplicar(paquete,crear_proceso,notificar_proceso_creado); 
			break;
			case ELIMINACION_PROCESO:
				recibir_pcb_y_aplicar(paquete,eliminar_proceso,notificar_proceso_eliminado); 
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

void imprimir_uso_frames(){
	printf("Uso de frames\n");
	printf("|  Indice  |   Uso   |");
	printf("                                           \n");
	for (int i=0;i<list_size(frames);i++){
	int uso = list_get(frames,i);
	
	if(i>=10){
		if(i>=100){
			printf("|    %d   |    %d    |",i,uso);
		}
		else{
			printf("|    %d    |    %d    |",i,uso);
		}
	
	}else{
	printf("|    %d     |    %d    |",i,uso);
	}
	printf("                                           \n");
	}
}


//Compara el tamaño nuevo con el acutal en numero de paginas.
//  -> 2. Quiere decir que es ampliación y se están pidiendo dos paginas mas de las que ya tiene
//  -> -4. Quiere decir que es reducción y deben sacar las últimas 4 páginas del proceso
//  -> 0 (No se si se va a dar este caso, lo contemplo. No hay resize. Mismo tamanio)
int diferencia_tamaño_nuevo_y_actual(t_list* tabla_paginas,t_tamanio_proceso* tamanio_proceso){

int cantidad_paginas_solicitadas = convertir_bytes_a_paginas(tamanio_proceso->tamanio);

if (list_is_empty(tabla_paginas)){
	return cantidad_paginas_solicitadas;
}else{
	int cantidad_paginas_actual = list_size(tabla_paginas);
	return cantidad_paginas_solicitadas - cantidad_paginas_actual;
}
}

bool esIgualA0(void* elemento){
	int* valor = (int*) elemento;
	return *valor ==false;
}

bool validar_ampliacion_proceso(int cantidad_frames_a_agregar){

int cant_paginas_disponibles = list_size(list_filter(frames,esIgualA0));

return cant_paginas_disponibles > cantidad_frames_a_agregar;
}

void ampliar_proceso(t_list* tabla_paginas,int cantidad_paginas_ampliar){

for (int i = 0;i < cantidad_paginas_ampliar;i++){
	int frame_asignado = asignar_frame();
	list_add(tabla_paginas,frame_asignado);
}

}

void reducir_proceso(t_list* tabla_paginas,int cantidad_paginas_reducir){
int ultimo_indice_actual = list_size(tabla_paginas)-1;
int ultimo_indice_nuevo = ultimo_indice_actual - cantidad_paginas_reducir;

for (int i = ultimo_indice_actual;i > ultimo_indice_nuevo;i--){
	remover_proceso_del_frame(list_get(tabla_paginas,i)); //Cambia el bit de uso a false
	list_remove(tabla_paginas,i);
}

}

int convertir_bytes_a_paginas(int tamanio_bytes){
	return tamanio_bytes / config_memoria->TAM_PAGINA; //Debería siempre devolver un entero no? Son múltiplos del tamanio memoria
}

int obtener_frame(t_list* tabla_de_paginas,int nro_pagina){
	return list_get(tabla_de_paginas,nro_pagina); //Tengo que convertir en int?
}


int asignar_frame(){
	int indice = 0;
	bool uso = list_get(frames,indice);
	while(uso == true){
	uso = list_get(frames,indice);
	indice++;
	}
	list_add_in_index(frames,indice,true);
	return indice;
}
void remover_proceso_del_frame(int frame){
	list_add_in_index(frames,frame,false);
}

void crear_frames_memoria_principal(int cantidadFrames){
	frames = list_create();
	
	for (int i = 0;i<cantidadFrames;i++){
		list_add(frames,false);
	} 
}