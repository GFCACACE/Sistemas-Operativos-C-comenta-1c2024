#ifndef dialfs_h
#define dialfs_h

#include "entradasalida.h"
#define PATH_BITMAP "bitmap.dat"
#define PATH_BLOQUES "bloques.dat"
#define DIR_METADATA "metadata/"

bool iniciar_archivos_dialfs();//Control de bloques de datos
// bool compactacion();
// bool io_fs_create():
// bool io_fs_delete();
// bool io_fs_write();
// bool io_fs_read();

extern FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
extern int tamanio_filesystem;
#endif