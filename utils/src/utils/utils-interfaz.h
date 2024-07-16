
#ifndef utils_interfaz_h
#define utils_interfaz_h

#include<utils/utils-server.h>
#include<utils/utils-client.h>
#include <utils/utils-commons.h>
#include </cpu.h>

void escribir_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros, char* registro_dato,int conexion);
t_buffer* leer_memoria_completa(t_direcciones_proceso* direcciones_fisicas_registros,int conexion);
t_operacion_fs* obtener_op_fs(char*,t_direcciones_proceso* ,uint32_t, op_code);
void enviar_operacion_fs(t_operacion_fs*,op_code ,int);
void* serializar_operacion_fs(t_operacion_fs*, int);
t_operacion_fs* recibir_op_fs(t_paquete* paquete);

#endif /* utils_interfaz_h */
