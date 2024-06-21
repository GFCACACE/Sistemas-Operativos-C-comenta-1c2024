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
	imprimir_uso_frames();
	
	
	




	loguear("Espacio memoria total: %d",config_memoria->TAM_MEMORIA);
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
		list_destroy_and_destroy_elements(proceso->tabla_paginas,free);
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
	free(memoriaPrincipal);
	
	list_destroy_and_destroy_elements(frames,free);
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
}
void efectuar_retardo(){
	usleep(config_memoria->RETARDO_RESPUESTA*1000);
}

int buscar_instrucciones(){
	 while (1) {
         t_paquete *paquete = recibir_paquete(conexion_cpu);
         int cod_op =paquete->codigo_operacion;
		 //int cod_op_a_devolver;


		loguear("Cod op: %d", cod_op);
		efectuar_retardo();
        switch (cod_op) {
            case PROXIMA_INSTRUCCION:
                t_pcb *pcb = recibir_pcb(paquete); 
                enviar_proxima_instruccion(pcb);
				paquete_destroy(paquete);
                break;
			case RESIZE:
				
				t_pid_valor* tamanio_proceso =  recibir_pid_value(paquete);
				ejecutar_resize(tamanio_proceso);
				imprimir_uso_frames();			
				
				imprimir_tabla_paginas_proceso(tamanio_proceso->PID);
				paquete_destroy(paquete);
				
				break;
			case ESCRITURA_MEMORIA:
	
				t_acceso_espacio_usuario* acceso_espacio_usuario_escritura = recibir_acceso_espacio_usuario(paquete);		
				acceder_a_espacio_usuario(ESCRITURA_MEMORIA,acceso_espacio_usuario_escritura);
				paquete_destroy(paquete);
				
				break;
			case ACCESO_TABLA_PAGINAS:
				loguear("TABLA PAGINAS \n");
				
				t_pid_valor* pid_pagina =  recibir_pid_value(paquete);	
				acceder_tabla_de_paginas(pid_pagina);
				paquete_destroy(paquete);
				free(pid_pagina);
				
				break;
				
			case LECTURA_MEMORIA:
				
				t_acceso_espacio_usuario* acceso_espacio_usuario_lectura = recibir_acceso_espacio_usuario(paquete);		
				acceder_a_espacio_usuario(LECTURA_MEMORIA,acceso_espacio_usuario_lectura);
				paquete_destroy(paquete);
							
				break;

            case FIN_PROGRAMA:
			    loguear("Fin programa");
				imprimir_uso_frames();	
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

// valor = 1050
// cantFRames= 128
// result= 8.2
// 8*cantFRames = 1024
void acceder_tabla_de_paginas(t_pid_valor* pid_pagina){
		t_proceso* proceso_en_memoria = dictionary_get(procesos,string_itoa(pid_pagina->PID));
		proceso_en_memoria = dictionary_get(procesos,string_itoa(pid_pagina->PID));
		t_list* tabla_de_paginas_proceso = proceso_en_memoria->tabla_paginas;
		int nro_pagina = pid_pagina->valor;
		int frame = obtener_frame(tabla_de_paginas_proceso, nro_pagina);
		char* frame_str =  string_itoa(frame);
		loguear("PID: %d - Pagina: %d - Marco: %d",pid_pagina->PID,nro_pagina,frame);
		enviar_texto(frame_str,RESPUESTA_NRO_FRAME,conexion_cpu);
		free(frame_str);
}

void ejecutar_resize(t_pid_valor* tamanio_proceso){
	int cod_op_a_devolver = RESIZE_OK;
	t_proceso* proceso_en_memoria = dictionary_get(procesos,string_itoa(tamanio_proceso->PID));
	t_list* tabla_de_paginas_proceso = proceso_en_memoria->tabla_paginas;
	int pag_solictadas_respecto_actual = diferencia_tamaño_nuevo_y_actual(tabla_de_paginas_proceso,tamanio_proceso->valor);   // Devuelve la diferencia entre la cantidad de paginas solicitadas y las que actualmente tiene el proceso
	
	if(pag_solictadas_respecto_actual>0){ //AMPLIACION_PROCESO
		loguear("PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d",tamanio_proceso->PID, list_size(tabla_de_paginas_proceso)*tamanio_pagina,tamanio_proceso->valor);			//En bytes se loguea no?
		if(validar_ampliacion_proceso(pag_solictadas_respecto_actual)){
				ampliar_proceso(tabla_de_paginas_proceso,pag_solictadas_respecto_actual);

		}else{
				loguear_error("No hay suficiente espacio en memoria");		
				cod_op_a_devolver = OUT_OF_MEMORY;
			}
		}
	if(pag_solictadas_respecto_actual<0){ //REDUCCION_PROCESO
		loguear("PID: %d - Tamaño Actual: %d - Tamaño a Reducir: %d",tamanio_proceso->PID, list_size(tabla_de_paginas_proceso),tamanio_proceso->valor);			
		int paginas_a_reducir = abs(pag_solictadas_respecto_actual);
		reducir_proceso(tabla_de_paginas_proceso,paginas_a_reducir);
		}
	
	enviar_texto("Resize efectuado",cod_op_a_devolver,conexion_cpu); 
}

void acceder_a_espacio_usuario(op_code tipo_acceso,t_acceso_espacio_usuario* acceso_espacio_usuario){
	void* direccion_real = &memoriaPrincipal + acceso_espacio_usuario->direccion_fisica;
	uint32_t bytes_restantes_en_frame = acceso_espacio_usuario->bytes_restantes_en_frame;
	if (tipo_acceso == LECTURA_MEMORIA){
		loguear("PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño: %d", acceso_espacio_usuario->PID,acceso_espacio_usuario->direccion_fisica,acceso_espacio_usuario->size_registro);
		char* dato_consultado = malloc((int)bytes_restantes_en_frame);		
		memcpy(&dato_consultado,direccion_real,(uint32_t)bytes_restantes_en_frame);
		enviar_texto(dato_consultado,VALOR_LECTURA_MEMORIA,conexion_cpu);
		
		}
	if (tipo_acceso==ESCRITURA_MEMORIA){
		loguear("PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño: %d", acceso_espacio_usuario->PID,acceso_espacio_usuario->direccion_fisica,acceso_espacio_usuario->size_registro);

		memcpy(direccion_real,&acceso_espacio_usuario->registro_dato,(uint32_t)acceso_espacio_usuario->size_registro);
		enviar_texto("OK",MOV_OUT_OK,conexion_cpu);
		loguear_warning("Se escribió: <%s>",acceso_espacio_usuario->registro_dato);
	}
	
}
	
bool tiene_exit(t_list* instrucciones){
	return list_any_satisfy(instrucciones,es_exit);
}

t_validacion* crear_proceso( t_pcb *pcb ){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->pcb=pcb;
	proceso->instrucciones = get_instrucciones_memoria(pcb->path);
	proceso->tabla_paginas = list_create();
	loguear("PID: %d - Tamaño: %d", pcb->PID,list_size(proceso->tabla_paginas));
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
		liberar_proceso_de_memoria(pcb->PID);
		
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
	printf("|   Indice   |    Uso    |");
	printf("                                           \n");
	for (int i=0;i<list_size(frames);i++){
	bool uso =  *(bool*) list_get(frames,i);
	
	if(i>=10){
		if(i>=100){
			printf("|     %d   |     %d    |",i,uso);
		}
		else{
			printf("|     %d     |     %d     |",i,uso);
		}
	
	}else{
	printf("|     %d      |     %d     |",i,uso);
	}
	printf("                                           \n");
	}
}

void imprimir_tabla_paginas_proceso(int PID){
	t_proceso* proceso_en_memoria = dictionary_get(procesos,string_itoa(PID));
	t_list* tabla_paginas =  proceso_en_memoria->tabla_paginas;
	printf("Tabla de paginas del proceso %d\n", PID);
	printf("| nro_pagina |   frame   |");
	printf("                                           \n");
	for (int i=0;i<list_size(tabla_paginas);i++){
	int uso =  *(int*) list_get(tabla_paginas,i);
	
	if(i>=10){
		if(i>=100){
			printf("|     %d    |     %d     |",i,uso);
		}
		else{
			printf("|      %d    |     %d     |",i,uso);
		}
	
	}else{
	printf("|     %d      |     %d     |",i,uso);
	}
	printf("                                           \n");
	}
}




//Compara el tamaño nuevo con el acutal en numero de paginas.
//  -> 2. Quiere decir que es ampliación y se están pidiendo dos paginas mas de las que ya tiene
//  -> -4. Quiere decir que es reducción y deben sacar las últimas 4 páginas del proceso
//  -> 0 (No se si se va a dar este caso, lo contemplo. No hay resize. Mismo tamanio)
int diferencia_tamaño_nuevo_y_actual(t_list* tabla_paginas, int tamanio_proceso){

int cantidad_paginas_solicitadas = convertir_bytes_a_paginas(tamanio_proceso);
	loguear("Cantidad paginas solicitadas: %d",cantidad_paginas_solicitadas);
	if (list_is_empty(tabla_paginas)){
		return cantidad_paginas_solicitadas;
	}
	else{
		int cantidad_paginas_actual = list_size(tabla_paginas);
		loguear("Paginas actual:%d",cantidad_paginas_actual);
		return cantidad_paginas_solicitadas - cantidad_paginas_actual;
	}
}

bool esIgualA0(void* elemento){
	bool* valor = (bool*) elemento;
	return !*valor;
}

bool validar_ampliacion_proceso(int cantidad_frames_a_agregar){
imprimir_uso_frames();
int cant_paginas_disponibles = list_size(list_filter(frames,esIgualA0));

return cant_paginas_disponibles > cantidad_frames_a_agregar;
}
int asignar_frame(){
	
	int frame_asignado = list_find_index(frames,&is_false);
	//loguear("frame asignado %d \n",frame_asignado);
	bool* frame = (bool*) list_get(frames,frame_asignado);
	*frame = true;
	return frame_asignado;

}
void ampliar_proceso(t_list* tabla_paginas,int cantidad_paginas_ampliar){

for (int i = 0;i < cantidad_paginas_ampliar;i++){
	
	int frame_asignado = asignar_frame();
	int* frame_asignado_a_pagina = malloc(sizeof(int));
	*frame_asignado_a_pagina = frame_asignado;	
	list_add(tabla_paginas, frame_asignado_a_pagina);
	
}


}

void reducir_proceso(t_list* tabla_paginas,int cantidad_paginas_reducir){
int ultimo_indice_actual = list_size(tabla_paginas)-1;
int ultimo_indice_nuevo = ultimo_indice_actual - cantidad_paginas_reducir;

for (int i = ultimo_indice_actual;i > ultimo_indice_nuevo;i--){
	int indice =  obtener_frame(tabla_paginas,i);
	remover_proceso_del_frame(indice); //Cambia el bit de uso a false
	list_remove(tabla_paginas,i);
}

}

int convertir_bytes_a_paginas(int tamanio_bytes){
	loguear("Tamanio en bytes %d",tamanio_bytes);
	loguear("Tamanio en en pag %d",config_memoria->TAM_PAGINA);
	//return ceil((double) (tamanio_bytes / config_memoria->TAM_PAGINA)); //Debería siempre devolver un entero no? Son múltiplos del tamanio memoria
	return (int)ceil((double) tamanio_bytes / config_memoria->TAM_PAGINA);
}

int obtener_frame(t_list* tabla_de_paginas,int nro_pagina){
	return *(int*) list_get(tabla_de_paginas,nro_pagina);
	
}
void quitar_paginas_de_frame(uint32_t PID){
		
	t_proceso* proceso_en_memoria = dictionary_get(procesos,string_itoa(PID));
	t_list* tabla_de_paginas_proceso = proceso_en_memoria->tabla_paginas;
	
	for (int i=0;i < list_size(tabla_de_paginas_proceso) ;i++){
	int nro_frame = obtener_frame(tabla_de_paginas_proceso,i);
	remover_proceso_del_frame(nro_frame);
	}
	loguear("PID: %d - Tamaño: %d", PID,list_size(tabla_de_paginas_proceso));
}

void liberar_proceso_de_memoria(uint32_t PID){
	quitar_paginas_de_frame(PID);
	dictionary_remove_and_destroy(procesos,string_itoa(PID),proceso_destroy);
}


void liberar_frame(int nro_frame){
	list_add_in_index(frames,nro_frame,false); 

}


void remover_proceso_del_frame(int frame){
	bool* dir_frame = list_get(frames,frame);
	*dir_frame = false;
}

void crear_frames_memoria_principal(int cantidadFrames){

	frames = list_create();
	
	for (int i = 0;i<cantidadFrames;i++){
		bool * uso = malloc(sizeof(bool));
		*uso = false;
		list_add(frames,uso);
	} 
}

