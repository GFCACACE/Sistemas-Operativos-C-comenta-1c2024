#include "dialfs.h"

FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
int tamanio_filesystem;
char* path_bitmap, *path_bloques;
char* bitmap;


bool iniciar_archivos_dialfs(){
    if(!strcmp("DIALFS",config->TIPO_INTERFAZ))
    {
    int i;
    
    tamanio_filesystem = config->BLOCK_SIZE * config->BLOCK_COUNT;
    
    path_bitmap = path_resolve(config->PATH_BASE_DIALFS,PATH_BITMAP);
    path_bloques = path_resolve(config->PATH_BASE_DIALFS,PATH_BLOQUES);
    archivo_bloques = fopen(path_bloques,"w");
    archivo_bitmap = fopen(path_bitmap,"w");
    for(i=0;i<tamanio_filesystem;i++) fprintf(archivo_bloques,"@");
    char* bitmap[config->BLOCK_COUNT];
    t_bitarray* bitmap_array =bitarray_create(bitmap,config->BLOCK_COUNT); 
    fwrite(&bitmap,config->BLOCK_COUNT,1,archivo_bitmap);
    fclose(archivo_bloques);
    fclose(archivo_bitmap);
    bitarray_destroy(bitmap_array);
   
    }
    return true;

}

bool io_fs_create(char* nombre_archivo){
    char* path_metadata =string_new();
    path_metadata = path_resolve(config->PATH_BASE_DIALFS,DIR_METADATA);
    path_metadata = path_resolve(path_metadata,nombre_archivo);
    archivo_metadata = fopen(path_metadata,"w");
    t_dialfs_metadata* metadata = create_metadata();
    // fwrite(&metadata,sizeof(t_dialfs_metadata),1,archivo_metadata);
    char* txt = string_new();
    sprintf(txt,"BLOQUE_INICIAL=%d TAMANIO_BLOQUE=%d",metadata->bloque_inicial,metadata->cantidad_bloques);
    txt_write_in_file(archivo_metadata,txt);
    fclose(archivo_metadata);
    free(txt);
    free(path_metadata);
    return true;

}

t_dialfs_metadata* create_metadata(){
    t_dialfs_metadata* metadata = malloc(sizeof(t_dialfs_metadata));
    metadata->bloque_inicial= asignar_bloque();
    metadata->cantidad_bloques = 0;
    return metadata;

}


uint32_t asignar_bloque(){
   t_bitarray* bitmap=obtener_bitmap();
   uint32_t i;
   bool flag_test_bit;
   for(i=0;i<config->BLOCK_COUNT;i++){
    flag_test_bit=bitarray_test_bit(bitmap,i);
    if(!flag_test_bit){
        bitarray_set_bit(bitmap,i);
        actualizar_bitmap(bitmap);
        bitarray_destroy(bitmap);
        return i;
    }
    
    
   }


    return -1;

}

t_bitarray* obtener_bitmap(){
    archivo_bitmap = fopen(path_bitmap,"r");
    char* bitmap = malloc(config->BLOCK_COUNT);
    fread(bitmap,config->BLOCK_COUNT,1, archivo_bitmap);
    return bitarray_create(bitmap,config->BLOCK_COUNT);
}

bool actualizar_bitmap(t_bitarray* bitmap){
    archivo_bitmap = fopen(path_bitmap,"w");
    fwrite((bitmap->bitarray),bitmap->size,1,archivo_bitmap);
    fclose(archivo_bitmap);
    return true;
}