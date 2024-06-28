#ifndef entradasalida_h
#define entradasalida_h
#include "entradasalida.h"
#include <stdlib.h>
#define PATH_BITMAP "${workspaceFolder}/files_dialFS/bitmap.dat"
#define PATH_BLOQUES "${workspaceFolder}/files_dialFS/bloques.dat"
#define DIR_METADATA "${workspaceFolder}/files_dialFS/metadata/"

bool iniciar_archivos_base();//Control de bloques de datos
bool compactacion();
bool io_fs_create():
bool io_fs_delete();
bool io_fs_write();
bool io_fs_read();

extern FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
extern int tamanio_filesystem;
#endif