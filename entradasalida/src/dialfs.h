#ifndef dialfs_h
#define dialfs_h

#include "entradasalida.h"
#define PATH_BITMAP "bitmap.dat"
#define PATH_BLOQUES "bloques.dat"
#define DIR_METADATA "metadata/"


typedef struct 
{
    uint32_t bloque_inicial;
    uint32_t cantidad_bloques;

} t_dialfs_metadata;


bool iniciar_archivos_dialfs();//Control de bloques de datos
// bool compactacion();
bool io_fs_create();
// bool io_fs_delete();
// bool io_fs_write();
// bool io_fs_read();

t_dialfs_metadata* create_metadata();

extern FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
extern int tamanio_filesystem;

#endif