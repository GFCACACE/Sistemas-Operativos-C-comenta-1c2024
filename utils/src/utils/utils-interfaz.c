#include "utils-interfaz.h"


 void escribir_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion_a_memoria){
	int operacion_ok;
	t_acceso_espacio_usuario* acceso_espacio_usuario;
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_pid_valor pid_size_total = direcciones_fisicas_registros->pid_size_total;
	uint32_t size_leido=0;
	uint32_t size_registro_pagina_actual;
    int registro_int = atoi(registro_dato);
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
	
	// 
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
			int size_registro_pagina_actual = direccion_registro->size_registro_pagina;
			
			
			t_acceso_espacio_usuario* acceso_espacio_usuario =  acceso_espacio_usuario_create(
			pid_size_total.PID,
			direccion_registro->direccion_fisica,
			direccion_registro->size_registro_pagina,
			NULL);		
			enviar_acceso_espacio_usuario(acceso_espacio_usuario,LECTURA_MEMORIA,conexion);
			
			free(acceso_espacio_usuario);
			int response = recibir_operacion(conexion);
				
		//	if(response == VALOR_LECTURA_MEMORIA){
				
				void* dato_recibido = recibir_buffer(&size_registro_pagina_actual,conexion);		

				memcpy(dato_final_puntero->stream + size_leido,dato_recibido, size_registro_pagina_actual);
				
				/////BORRAR
				//memcpy(registro_puntero_recons + size_leido, dato_recibido ,size_registro_pagina_actual);
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
}


 void escribir_memoria_completa_io(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion_a_memoria){
	int operacion_ok;
	t_acceso_espacio_usuario* acceso_espacio_usuario;
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_pid_valor pid_size_total = direcciones_fisicas_registros->pid_size_total;
	uint32_t size_leido=0;
	uint32_t size_registro_pagina_actual;
    
	//BORRAR
	//char* registro_dato = malloc(5);
	//registro_dato = "hola";
	//BORRAR
    void* registro_puntero =(void*) registro_dato; 
	void* dato_parcial_prueba = malloc(pid_size_total.valor);
		void _enviar_direcciones_memoria(void* element){
		
			t_direccion_registro* direccion_registro = (t_direccion_registro*) element;
			size_registro_pagina_actual = direccion_registro->size_registro_pagina;
			
			
			void* dato_parcial = malloc(size_registro_pagina_actual);

			//BORRAR
			//void* dato_parcial_prueba = malloc(size_registro_pagina_actual+1);
			//BORRAR
			memcpy(dato_parcial, registro_puntero + size_leido,size_registro_pagina_actual);
			
			//BORRAR
			memcpy(dato_parcial_prueba + size_leido,dato_parcial,size_registro_pagina_actual);
			//BORRAR
			
			
			acceso_espacio_usuario =  acceso_espacio_usuario_create(
			pid_size_total.PID,
			direccion_registro->direccion_fisica,
			direccion_registro-> size_registro_pagina,
			dato_parcial);
			
			
			enviar_acceso_espacio_usuario(acceso_espacio_usuario,PEDIDO_STDIN,conexion_a_memoria);
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
	((char*)dato_parcial_prueba)[pid_size_total.valor] = '\0';
	loguear("Dato prueba <%s>",dato_parcial_prueba);
//	loguear("Valor escrito: <%d>",registro_reconstr);
	free(dato_parcial_prueba);

//free(registro_dato);		
}


t_buffer* leer_memoria_completa_io(t_direcciones_proceso* direcciones_fisicas_registros,int conexion){

	// 
	int response;
	t_acceso_espacio_usuario* acceso_espacio_usuario;
	t_list* direcciones_registros =  direcciones_fisicas_registros->direcciones;
	t_pid_valor pid_size_total = direcciones_fisicas_registros->pid_size_total;
	uint32_t size_leido=0;
	uint32_t size_registro_pagina_actual;
    t_buffer* dato_final_puntero = crear_buffer(pid_size_total.valor);



	void _enviar_direcciones_memoria(void* element){	
			t_direccion_registro* direccion_registro = (t_direccion_registro*) element;
			uint16_t size_registro_pagina_actual = direccion_registro->size_registro_pagina;

			loguear("direccion_registro:<%d>",  direccion_registro->size_registro_pagina);
			loguear("direccion_registro->direccion_fisica:<%d>", direccion_registro->direccion_fisica);
			t_acceso_espacio_usuario* acceso_espacio_usuario =  acceso_espacio_usuario_create(
			pid_size_total.PID,
			direccion_registro->direccion_fisica,
			direccion_registro->size_registro_pagina,
			NULL);		
			enviar_acceso_espacio_usuario(acceso_espacio_usuario,PEDIDO_STDOUT,conexion);

			free(acceso_espacio_usuario);
			int response = recibir_operacion(conexion);

		//	if(response == VALOR_LECTURA_MEMORIA){

				void* dato_recibido = recibir_buffer(&size_registro_pagina_actual,conexion);		

				memcpy(dato_final_puntero->stream + size_leido,dato_recibido, size_registro_pagina_actual);
				char* val_parcial= malloc(size_registro_pagina_actual+1);
				memcpy(val_parcial + size_leido,dato_final_puntero->stream ,size_registro_pagina_actual);
				((char*)val_parcial)[size_registro_pagina_actual] = '\0';
				loguear("size_registro_pagina_actual <%d>",size_registro_pagina_actual);
				loguear("Valor parcial: <%s>",val_parcial);
				free(val_parcial);
				/////BORRAR
				//memcpy(registro_puntero_recons + size_leido, dato_recibido ,size_registro_pagina_actual);
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
}