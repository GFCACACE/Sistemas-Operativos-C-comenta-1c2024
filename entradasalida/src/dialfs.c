#include "dialfs.h"

FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
int tamanio_filesystem;
char* path_bitmap, *path_bloques;
char* bitmap;
t_list* lista_archivos;

bool iniciar_archivos_dialfs(){
    if(DIALFS == config->TIPO_INTERFAZ.id)
    {
    int i;
    lista_archivos = list_create();
    tamanio_filesystem = config->BLOCK_SIZE * config->BLOCK_COUNT;
    path_bitmap = path_resolve(config->PATH_BASE_DIALFS,PATH_BITMAP);
    path_bloques = path_resolve(config->PATH_BASE_DIALFS,PATH_BLOQUES);
    archivo_bloques = fopen(path_bloques,"w");
    archivo_bitmap = fopen(path_bitmap,"w");
    for(i=0;i<tamanio_filesystem;i++) fprintf(archivo_bloques,"@");
    char* bitmap = malloc(config->BLOCK_COUNT);
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
    
    t_dialfs_metadata* metadata = create_metadata();
    list_add(lista_archivos, metadata);
    // fwrite(&metadata,sizeof(t_dialfs_metadata),1,archivo_metadata);
    editar_archivo_metadata(path_metadata,metadata);

    free(path_metadata);
    return true;

}

bool io_fs_delete(char* nombre_archivo){
    
    t_dialfs_metadata* buscar_archivo(void* elem){
        t_dialfs_metadata* metadata = (t_dialfs_metadata*) elem;
        if(!strcmp(nombre_archivo, metadata->nombre_archivo))
            return metadata;
    };

    char* path_metadata =string_new();
    path_metadata = path_resolve(config->PATH_BASE_DIALFS,DIR_METADATA);
    path_metadata = path_resolve(path_metadata,nombre_archivo);
    t_dialfs_metadata* metadata_delete = list_iterate(lista_archivos,&buscar_archivo);
    truncar_bitmap(metadata_delete,0);
    list_remove_element(lista_archivos,metadata_delete);
    remove(path_metadata);
    free(metadata_delete->nombre_archivo);
    free(metadata_delete);
    return true;
}

bool io_fs_truncate(char* nombre_archivo,uint32_t tamanio_final){

    t_dialfs_metadata* buscar_archivo(void* elem){
        t_dialfs_metadata* metadata = (t_dialfs_metadata*) elem;
        if(!strcmp(nombre_archivo, metadata->nombre_archivo))
            return metadata;
    };
    char* path_metadata =string_new();
    path_metadata = path_resolve(config->PATH_BASE_DIALFS,DIR_METADATA);
    path_metadata = path_resolve(path_metadata,nombre_archivo);
    t_dialfs_metadata* metadata_delete = list_iterate(lista_archivos,&buscar_archivo);
    truncar_bitmap(metadata_delete,tamanio_final);
    metadata_delete->cantidad_bloques = tamanio_final;
    editar_archivo_metadata(path_metadata,metadata_delete);



}


bool truncar_bitmap(t_dialfs_metadata* metadata, uint32_t tamanio_final){

    t_bitarray* bitmap = obtener_bitmap();
    uint32_t posicion_inicial=metadata->bloque_inicial + metadata->cantidad_bloques;
    uint32_t posicion_final= metadata->bloque_inicial + tamanio_final;
    for(uint32_t i=posicion_final;i <= posicion_inicial;i++)
        bitarray_clean_bit(bitmap,i);
    actualizar_bitmap(bitmap);
    return true;


}

bool editar_archivo_metadata(char* path_metadata,t_dialfs_metadata* metadata){
    archivo_metadata = fopen(path_metadata,"w");
    char* txt = string_new();
    sprintf(txt,"BLOQUE_INICIAL=%d TAMANIO_BLOQUE=%d",metadata->bloque_inicial,metadata->cantidad_bloques);
    txt_write_in_file(archivo_metadata,txt);
    fclose(archivo_metadata);
    free(txt);
    return true;
}


t_dialfs_metadata* create_metadata(){
    t_dialfs_metadata* metadata = malloc(sizeof(t_dialfs_metadata));
    metadata->bloque_inicial= (uint32_t)asignar_bloque_inicial();
    metadata->cantidad_bloques = 1;
    return metadata;

}


int asignar_bloque_inicial(){
   t_bitarray* bitmap=obtener_bitmap();
   int i;
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
    fclose(archivo_bitmap);
    return bitarray_create(bitmap,config->BLOCK_COUNT);
}

bool actualizar_bitmap(t_bitarray* bitmap){
    archivo_bitmap = fopen(path_bitmap,"w");
    fwrite((bitmap->bitarray),bitmap->size,1,archivo_bitmap);
    fclose(archivo_bitmap);
    return true;
}