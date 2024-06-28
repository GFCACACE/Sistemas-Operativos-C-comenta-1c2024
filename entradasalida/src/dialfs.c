#include "dialfs.h"
FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
int tamanio_filesystem;
t_config_io* config;

bool iniciar_archivos_base(){
    int i;
    tamanio_filesystem = config->BLOCK_SIZE * config->BLOCK_COUNT;
    archivo_bloques = fopen(PATH_BLOQUES,"w");
    archivo_bitmap = fopen(PATH_BITMAP,"w");
    for(i=0;i<tamanio_filesystem;i++) fprintf(archivo_bloques,'\0');
    archivo_bitmap = fopen(PATH_BITMAP,"w");
    for (i=0;i < config->BLOCK_COUNT;i++) fprintf(archivo_bitmap,"0");
    fclose(archivo_bitmap);
    fclose(archivo_bloques);
    return true;

}