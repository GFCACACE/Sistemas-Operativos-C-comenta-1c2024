#include "dialfs.h"

FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
int tamanio_filesystem;
t_bitarray dialfs_bitmap;

bool iniciar_archivos_dialfs(){
    if(!strcmp("DIALFS",config->TIPO_INTERFAZ))
    {
    int i;
    
    tamanio_filesystem = config->BLOCK_SIZE * config->BLOCK_COUNT;
    char* path_bitmap = path_resolve(config->PATH_BASE_DIALFS,PATH_BITMAP);
    dialfs_bitmap = bitarray_create_with_mode(/*byte 0 de archivo bitmap.dat*/, config->BLOCK_COUNT,LSB_FIRST);
    char* path_bloques = path_resolve(config->PATH_BASE_DIALFS,PATH_BLOQUES);
    archivo_bloques = fopen(path_bloques,"w");
    archivo_bitmap = fopen(path_bitmap,"w");
    for(i=0;i<tamanio_filesystem;i++) fprintf(archivo_bloques,"@");
    for (i=0;i < config->BLOCK_COUNT;i++) fprintf(archivo_bitmap,"0");
    fclose(archivo_bloques);
    fclose(archivo_bitmap);
    
    }
    return true;

}

bool io_fs_create(char* nombre_archivo){
    char* path_metadata = path_resolve(DIR_METADATA,nombre_archivo);
    archivo_metadata = fopen(path_metadata,"w");

}

t_dialfs_metadata* create_metadata(){
    t_dialfs_metadata* metadata = malloc(sizeof(t_dialfs_metadata));
    metadata->bloque_inicial= asignar_bloque();

}