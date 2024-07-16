#include "utils/utils-interfaz.h"


 void escribir_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion_a_memoria){
	int operacion_ok;
	t_acceso_espacio_usuario* acceso_espacio_usuario;
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_pid_valor pid_size_total = direcciones_fisicas_registros->pid_size_total;
	uint32_t size_leido=0;
	uint32_t size_registro_pagina_actual;
    uint32_t registro_int = atoi(registro_dato);
    void* registro_puntero = &registro_int;
	int registro_reconstr;
    void* registro_puntero_recons = &registro_reconstr;
 

		void _enviar_direcciones_memoria(void* element){
		
			t_direccion_registro* direccion_registro = (t_direccion_registro*) element;
			size_registro_pagina_actual = direccion_registro->size_registro_pagina;
			
			void* dato_parcial = malloc(size_registro_pagina_actual);

			memcpy(dato_parcial, registro_puntero + size_leido,size_registro_pagina_actual);
			
			memcpy(registro_puntero_recons + size_leido, dato_parcial ,size_registro_pagina_actual);

			acceso_espacio_usuario =  acceso_espacio_usuario_create(
			pid_size_total.PID,
			direccion_registro->direccion_fisica,
			direccion_registro-> size_registro_pagina,
			dato_parcial);
			
			
			enviar_acceso_espacio_usuario(acceso_espacio_usuario,ESCRITURA_MEMORIA,conexion_a_memoria);
			size_leido += size_registro_pagina_actual;	

			operacion_ok = recibir_operacion(conexion_a_memoria);
			
			if(operacion_ok==MOV_OUT_OK){
				char* valor_memoria =recibir_mensaje(conexion_a_memoria);
				free(valor_memoria);
			}
			
			//loguear("PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>",
			//pid_size_total.PID,direccion_registro->direccion_fisica,registro_dato + size_leido);
			free(acceso_espacio_usuario);
			free(dato_parcial);
			
			
		};
		
	list_iterate(direcciones_registros, &_enviar_direcciones_memoria);

	loguear("Valor escrito: <%d>",registro_reconstr);	
}

t_buffer* leer_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros,int conexion){
	
	int response;
	t_acceso_espacio_usuario* acceso_espacio_usuario;
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_pid_valor pid_size_total = direcciones_fisicas_registros->pid_size_total;
	uint32_t size_leido=0;
	uint32_t size_registro_pagina_actual;
    t_buffer* dato_final_puntero = crear_buffer(pid_size_total.valor);
	
	/////BORRAR
	int registro_reconstr;
    void* registro_puntero_recons = &registro_reconstr;
	/////BORRAR


	void _enviar_direcciones_memoria(void* element){	
			t_direccion_registro* direccion_registro = (t_direccion_registro*) element;
			size_registro_pagina_actual = direccion_registro->size_registro_pagina;
			
			
			acceso_espacio_usuario =  acceso_espacio_usuario_create(
			pid_size_total.PID,
			direccion_registro->direccion_fisica,
			direccion_registro->size_registro_pagina,
			NULL);		
			enviar_acceso_espacio_usuario(acceso_espacio_usuario,LECTURA_MEMORIA,conexion);
			
			free(acceso_espacio_usuario);
			response = recibir_operacion(conexion);
				
		//	if(response == VALOR_LECTURA_MEMORIA){
				
				void* dato_recibido = recibir_buffer(&size_registro_pagina_actual,conexion);		

				memcpy(dato_final_puntero->stream + size_leido,dato_recibido, size_registro_pagina_actual);
				
				/////BORRAR
				memcpy(registro_puntero_recons + size_leido, dato_recibido ,size_registro_pagina_actual);
				/////BORRAR
				
				size_leido += size_registro_pagina_actual;
	
				
			//	loguear("PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>",
		   // pid_size_total.PID,direccion_registro->direccion_fisica,dato_recibido);
			free(dato_recibido);
			//}
			
			
		}
		list_iterate(direcciones_registros, &_enviar_direcciones_memoria);
		/////BORRAR
		//loguear("Valor leido: <%d>",registro_reconstr);
		/////BORRAR
	return dato_final_puntero;
};


t_operacion_fs* obtener_op_fs(uint32_t pid, char* nmb_archivo,t_direcciones_proceso* direcciones, uint32_t puntero_archivo, uint32_t tamanio_truncate,op_code cod_op){
	t_operacion_fs* operacion_fs = malloc(sizeof(t_operacion_fs));
	operacion_fs->nombre_archivo = nmb_archivo;
	operacion_fs->cod_op = cod_op;
	operacion_fs->registro_puntero_archivo = puntero_archivo;
	operacion_fs->direcciones_proceso = direcciones;
	operacion_fs->tamanio_truncate = tamanio_truncate;
	operacion_fs->pid = pid;

	return operacion_fs;
}
void enviar_operacion_fs(t_operacion_fs* operacion,op_code op,int socket){
	int size;
	void* stream = serializar_operacion_fs(operacion,&size);									
					 
	enviar_stream(stream,size,socket,op);
	free(stream);
}


/*
typedef struct t_operacion_fs{
	op_code cod_op;
	uint32_t pid;
	uint32_t tamanio_registro;
	uint32_t registro_puntero;//FSEEK
	uint32_t tamanio_truncate;
	char* nombre_archivo;
	t_list* direcciones;
}t_operacion_fs;

typedef struct t_direccion_tamanio{
	uint32_t direccion_fisica;
	uint32_t tamanio_bytes;
}t_direccion_tamanio

*/
void* serializar_operacion_fs(t_operacion_fs* operacion_fs,int* size){
	
	//t_list* lista = operacion_fs->direcciones_proceso->direcciones;
	t_list* lista = operacion_fs->direcciones;
	char* nombre = malloc(strlen(operacion_fs->nombre_archivo)+1);
	uint32_t cant_direcciones = list_size(lista);
	*size = sizeof(op_code) + sizeof(uint32_t) * 4 + sizeof(uint32_t) + strlen(nombre) + sizeof(uint32_t) + cant_direcciones * (2*sizeof(uint32_t));
													// TAMAÑO char*                      TAMAÑO direcciones_proceso
	t_buffer* buffer = crear_buffer(*size);
	
	agregar_a_buffer(buffer, &operacion_fs->cod_op, sizeof(op_code));
	agregar_a_buffer(buffer, &operacion_fs->pid, sizeof(uint32_t));
	agregar_a_buffer(buffer, &operacion_fs->tamanio_registro, sizeof(uint32_t));
	agregar_a_buffer(buffer, &operacion_fs->registro_puntero, sizeof(uint32_t));
	agregar_a_buffer(buffer, &operacion_fs->tamanio_truncate, sizeof(uint32_t));
	agregar_a_buffer(buffer, strlen(nombre), sizeof(uint32_t)); // TAMAÑO CHAR*
	agregar_a_buffer(buffer, &nombre, strlen(nombre));
	agregar_a_buffer(buffer, &cant_direcciones, sizeof(uint32_t)); // TAMAÑO/CANTIDAD DIRECCIONES
	
	for(int i=0;i<cant_direcciones;i++)
	{	
		t_direccion_tamanio* direc = (t_direccion_tamanio*)list_get(lista,i);  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		agregar_a_buffer(buffer, &direc->direccion_fisica, sizeof(uint32_t)); // REVISAR
		agregar_a_buffer(buffer, &direc->tamanio_bytes, sizeof(uint32_t)); // 
															// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	void * stream = buffer->stream;
	free(buffer);
	free(nombre);
	return stream;
}


// POR HACER...
t_operacion_fs* recibir_op_fs(t_paquete* paquete)
{
	int tam_nombre;
	int cant_direcciones;
	t_operacion_fs* op_fs = malloc(sizeof(t_operacion_fs));
	
	t_buffer* buffer = paquete->buffer;
	buffer->desplazamiento = sizeof(uint32_t);
	void _recibir(void* lugar_destino,size_t tam){
		recibir_de_buffer(lugar_destino,buffer,tam);
	}

	agregar_a_buffer(buffer, &cant_direcciones, sizeof(uint32_t));

	_recibir(&op_fs->cod_op,sizeof(op_code));
	_recibir(&op_fs->pid,sizeof(uint32_t));
	_recibir(&op_fs->tamanio_registro,sizeof(uint32_t));
	_recibir(&op_fs->registro_puntero,sizeof(uint32_t));
	_recibir(&op_fs->tamanio_truncate,sizeof(uint32_t));
	_recibir(&tam_nombre,sizeof(uint32_t));
	_recibir(&op_fs->nombre_archivo,tam_nombre);
	_recibir(&cant_direcciones,sizeof(uint32_t));

	for(int i=0;i<cant_direcciones;i++)
	{	
		t_direccion_tamanio* direc = malloc(sizeof(t_direccion_tamanio));
		_recibir(&direc->direccion_fisica,sizeof(uint32_t));
		_recibir(&direc->tamanio_bytes,sizeof(uint32_t));
		list_add(direcciones_proceso->direcciones,direc);
	}
	return op_fs;
}





t_direcciones_proceso* recibir_direcciones_proceso(t_paquete* paquete)
{
	t_buffer* buffer = paquete->buffer;
	buffer->desplazamiento = sizeof(uint32_t);
	void _recibir(void* lugar_destino,size_t tam){
		recibir_de_buffer(lugar_destino,buffer,tam);
	}
	int cant_direcciones;
	t_direcciones_proceso* direcciones_proceso = direcciones_proceso_create(0,0);
	
	_recibir(&direcciones_proceso->pid_size_total.PID,sizeof(uint32_t));
	_recibir(&direcciones_proceso->pid_size_total.valor,sizeof(uint32_t));
	_recibir(&cant_direcciones,sizeof(uint32_t));
	for(int i=0;i<cant_direcciones;i++)
	{	t_id_valor* id_valor = malloc(sizeof(t_id_valor));
		_recibir(&id_valor->id,sizeof(uint32_t));
		_recibir(&id_valor->valor,sizeof(uint32_t));
		list_add(direcciones_proceso->direcciones,id_valor);
	}
	

	return direcciones_proceso;
}


t_acceso_espacio_usuario* acceso_espacio_usuario_create(uint32_t PID, uint32_t direccion, uint32_t size_registro,void* valor){
	t_acceso_espacio_usuario* acceso_espacio_usuario = malloc(sizeof(t_acceso_espacio_usuario));
	acceso_espacio_usuario->PID = PID;
	acceso_espacio_usuario->direccion_fisica = direccion;
	//acceso_espacio_usuario->bytes_restantes_en_frame = bytes_restantes;
	acceso_espacio_usuario->size_registro = size_registro;
	//acceso_espacio_usuario->size_registro = (valor!=NULL) ? (uint32_t)(strlen(valor)+1) : (uint32_t)0;
	acceso_espacio_usuario->registro_dato = valor;
	return acceso_espacio_usuario;
}


	
void* serializar_acceso_espacio_usuario(t_acceso_espacio_usuario* acceso_espacio_usuario,int* size){
	// uint32_t tamanio_dato = ((uint32_t)strlen(acceso_espacio_usuario->registro_dato)+(uint32_t)1);
	
	*size = sizeof(uint32_t) * 3 ;
	if (acceso_espacio_usuario->registro_dato!=NULL){
	*size = *size +  (acceso_espacio_usuario->size_registro); 
	}
	t_buffer* buffer = crear_buffer(*size);
	agregar_a_buffer(buffer, &acceso_espacio_usuario->PID, sizeof(uint32_t));
	agregar_a_buffer(buffer, &acceso_espacio_usuario->direccion_fisica, sizeof(uint32_t));
	//agregar_a_buffer(buffer, &acceso_espacio_usuario->bytes_restantes_en_frame, sizeof(uint32_t));
	agregar_a_buffer(buffer, &acceso_espacio_usuario->size_registro, sizeof(uint32_t));
	if( acceso_espacio_usuario->registro_dato!=NULL){
	agregar_a_buffer(buffer, acceso_espacio_usuario->registro_dato, acceso_espacio_usuario->size_registro);
	}
	void * stream = buffer->stream;
	free(buffer);
	
	return stream;

}

