#include "utils-serializacion.h"

void buffer_destroy(t_buffer* buffer){
	if(buffer!=NULL){
		if(buffer->stream!=NULL)
			free(buffer->stream);

		free(buffer);
	}

}
void paquete_destroy(t_paquete* paquete){
	buffer_destroy(paquete->buffer);
	free(paquete);
}
