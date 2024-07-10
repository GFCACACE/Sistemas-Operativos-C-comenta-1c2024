#ifndef dialfs_h
#define dialfs_h

#include "entradasalida.h"
#define PATH_BITMAP "bitmap.dat"
#define PATH_BLOQUES "bloques.dat"
#define DIR_METADATA "metadata/"


typedef struct 
{   
    char* nombre_archivo;
    uint32_t bloque_inicial;
    uint32_t cantidad_bloques;
    

} t_dialfs_metadata;



bool iniciar_archivos_dialfs();//Control de bloques de datos
// bool compactacion();
bool io_fs_create(char* nombre_archivo);
bool io_fs_delete(char* nombre_archivo);
bool io_fs_truncate(char* nombre_archivo, uint32_t tamanio_final);
// bool io_fs_write();
// bool io_fs_read();

bool actualizar_bitmap(t_bitarray* bitmap);
bool editar_archivo_metadata(char* path_metadata,t_dialfs_metadata* metadata);

t_dialfs_metadata* create_metadata(char* nombre_archivo);
t_bitarray* obtener_bitmap();
int asignar_bloque_inicial();
bool truncar_bitmap(t_dialfs_metadata* metadata, uint32_t tamanio_final);


extern FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
extern int tamanio_filesystem;
extern char* bitmap;
extern t_list* lista_archivos;
#endif