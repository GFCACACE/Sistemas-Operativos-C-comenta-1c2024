#include "utils-client.h"


void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;  // MAGIC?
}

int crear_conexion(char *ip, int puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	char* puerto_char = (char*)malloc(sizeof(char) * 20); 
	sprintf(puerto_char, "%d", puerto);
	getaddrinfo(ip, puerto_char, &hints, &server_info);
	free(puerto_char);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	int connect_res=connect(socket_cliente,server_info->ai_addr,server_info->ai_addrlen);
	printf("%d\n",connect_res);
	freeaddrinfo(server_info);
	if(connect_res<0)
		socket_cliente = connect_res;

	return socket_cliente;
}


void enviar_stream(void*stream,int size,int socket,op_code codigo_operacion){
	//loguear("enviar_mensaje");
	t_paquete* paquete = crear_paquete(codigo_operacion);	
	agregar_a_paquete(paquete,stream,size);	
	enviar_paquete(paquete,socket);
	paquete_destroy(paquete);
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	// loguear("Se va a enviar el mensaje %s.", mensaje);
	// loguear("Length:%ld",strlen(mensaje));
	// loguear("Length:%ld",strlen("hola"));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	paquete_destroy(paquete);
}


// void crear_buffer(t_paquete* paquete)
// {
// 	paquete->buffer = malloc(sizeof(t_buffer));
// 	paquete->buffer->size = 0;
// 	paquete->buffer->stream = NULL;
// }
t_buffer* crear_buffer(size_t size)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = size;
	buffer->stream = malloc(buffer->size);
	buffer->desplazamiento = 0;

	return buffer;
}


t_paquete* crear_paquete(op_code codigo_operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	paquete->buffer = crear_buffer(0);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	 close(socket_cliente);
}

void agregar_a_buffer(t_buffer* buffer,const void* lugar_origen,size_t tam){
	memcpy(buffer->stream+buffer->desplazamiento,lugar_origen,tam);
	buffer->desplazamiento+=tam;
}

void* serializar_pcb(t_pcb* pcb,int* size)
{	
	
	uint32_t path_size = strlen(pcb->path) + 1;
	//		(PID,pc,quantum,size del path) + registros + prioridad + path_size
	*size = sizeof(uint32_t) *4 + 4*sizeof(t_registro) + sizeof(uint8_t) + path_size ;
	t_buffer* buffer = crear_buffer(*size);

	//loguear("Size PCB:%d",*size);

	agregar_a_buffer(buffer, &pcb->PID, sizeof(uint32_t));
	agregar_a_buffer(buffer, &pcb->prioridad, sizeof(uint8_t));
	agregar_a_buffer(buffer, &pcb->program_counter, sizeof(uint32_t));
	agregar_a_buffer(buffer, &pcb->quantum, sizeof(uint32_t));
	agregar_a_buffer(buffer, &pcb->registros_cpu->AX, sizeof(t_registro));
	agregar_a_buffer(buffer, &pcb->registros_cpu->BX, sizeof(t_registro));
	agregar_a_buffer(buffer, &pcb->registros_cpu->CX, sizeof(t_registro));
	agregar_a_buffer(buffer, &pcb->registros_cpu->DX, sizeof(t_registro));
	agregar_a_buffer(buffer, &path_size, sizeof(uint32_t));
	agregar_a_buffer(buffer, pcb->path, path_size);

	
//	loguear("Path: %s",pcb->path);
//	loguear("desplazamiento:%d",buffer->desplazamiento);	
	void * stream = buffer->stream;
	free(buffer);

	return stream;
}

void enviar_pcb(t_pcb* pcb,op_code operacion,int socket){
	int size;
	void* stream = serializar_pcb(pcb,&size);
	//	loguear_stream_pcb(stream,size);
	loguear_pcb(pcb);
	enviar_stream(stream,size,socket,operacion);
	free(stream);
}

void enviar_texto(char* texto,op_code operacion,int socket){
	int size = strlen(texto) + 1;
	enviar_stream(texto,size,socket,operacion);

}