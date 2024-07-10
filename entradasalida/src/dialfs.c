#include "dialfs.h"

FILE *archivo_bitmap,*archivo_metadata,*archivo_bloques;
int tamanio_filesystem, tamanio_bitmap;
char* path_bitmap, *path_bloques, *dir_metadata;
void* bitmap;
void* data_bloques;
t_bitarray* bitarray_bitmap;
t_list* lista_archivos;

bool iniciar_archivos_dialfs(){
    if(DIALFS == config->TIPO_INTERFAZ.id)
    {
    int i;
    tamanio_bitmap= ceil(((config->BLOCK_COUNT)/8.0));
    lista_archivos = list_create();
    dir_metadata = path_resolve(config->PATH_BASE_DIALFS,DIR_METADATA);
    // DIR *dir = opendir(dir_metadata);
    // struct dirent *entry;
    // while ((entry = readdir(dir)) != NULL) {
    //     t_dialfs_metadata* metadata=obtener_metadata_txt(entry->d_name);
    //     list_add(lista_archivos,metadata);
    // }
    tamanio_filesystem = config->BLOCK_SIZE * config->BLOCK_COUNT;
    path_bitmap = path_resolve(config->PATH_BASE_DIALFS,PATH_BITMAP);
    path_bloques = path_resolve(config->PATH_BASE_DIALFS,PATH_BLOQUES);
    if(access(path_bloques,F_OK)!=0){
        archivo_bloques = fopen(path_bloques,"w");
        for(i=0;i<tamanio_filesystem;i++)
            fprintf(archivo_bloques,"@");
        fclose(archivo_bloques);
    }
    if(access(path_bitmap,F_OK)!=0){
        archivo_bitmap = fopen(path_bitmap,"w");
        void* bitmap_aux = malloc(tamanio_bitmap);
        t_bitarray* bitarray = bitarray_create_with_mode(bitmap_aux,tamanio_bitmap,LSB_FIRST);
        for(i=0;i<config->BLOCK_COUNT;i++)
            bitarray_clean_bit(bitarray,i);
        fwrite(bitarray->bitarray,config->BLOCK_COUNT,1,archivo_bitmap);
        fclose(archivo_bitmap);
    }
    int fd_bitmap = open(path_bitmap,O_RDWR);
    int fd_data = open(path_bloques,O_RDWR);
    bitmap = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    data_bloques = mmap(NULL, tamanio_filesystem, PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);
    bitarray_bitmap= bitarray_create_with_mode(bitmap,tamanio_bitmap,LSB_FIRST);
    }
    return true;

}

t_dialfs_metadata* obtener_metadata_txt(char* nombre_archivo){
    t_dialfs_metadata* metadata=malloc(sizeof(t_dialfs_metadata));



    return metadata;
}

bool io_fs_create(char* nombre_archivo){
    char* path_metadata =string_new();
    path_metadata = path_resolve(dir_metadata,nombre_archivo);
    
    t_dialfs_metadata* metadata = create_metadata(nombre_archivo);
    list_add(lista_archivos, metadata);
    // fwrite(&metadata,sizeof(t_dialfs_metadata),1,archivo_metadata);
    editar_archivo_metadata(path_metadata,metadata);

    free(path_metadata);
    return true;

}

bool io_fs_delete(char* nombre_archivo){
    
    bool buscar_archivo(void* elem){
        t_dialfs_metadata* metadata = (t_dialfs_metadata*) elem;
        if(!strcmp(nombre_archivo, metadata->nombre_archivo))
            return true;
    };

    char* path_metadata =string_new();
    path_metadata = path_resolve(config->PATH_BASE_DIALFS,DIR_METADATA);
    path_metadata = path_resolve(path_metadata,nombre_archivo);
    t_dialfs_metadata* metadata_delete = (t_dialfs_metadata*)list_find(lista_archivos,&buscar_archivo);
    truncar_bitmap(metadata_delete,0);
    list_remove_element(lista_archivos,metadata_delete);
    remove(path_metadata);
    // free(metadata_delete->nombre_archivo);
    free(metadata_delete);
    return true;
}

bool io_fs_truncate(char* nombre_archivo,uint32_t tamanio_final){

    bool buscar_archivo(void* elem){
        t_dialfs_metadata* metadata = (t_dialfs_metadata*) elem;
        if(!strcmp(nombre_archivo, metadata->nombre_archivo))
            return true;
        return false;
    };
    char* path_metadata =string_new();
    path_metadata = path_resolve(config->PATH_BASE_DIALFS,DIR_METADATA);
    path_metadata = path_resolve(path_metadata,nombre_archivo);
    t_dialfs_metadata* metadata_delete = (t_dialfs_metadata*)list_find(lista_archivos,&buscar_archivo);
    truncar_bitmap(metadata_delete,tamanio_final);
    metadata_delete->tamanio_archivo = tamanio_final;
    editar_archivo_metadata(path_metadata,metadata_delete);

    return true;

}


bool truncar_bitmap(t_dialfs_metadata* metadata, uint32_t tamanio_final){

    uint32_t tam_archivo = (metadata->tamanio_archivo == 0)? 0 : metadata->tamanio_archivo - 1;
    uint32_t cantidad_bloques = tam_archivo/ config->BLOCK_SIZE;
    uint32_t posicion_inicial=metadata->bloque_inicial + cantidad_bloques;
    uint32_t posicion_final= metadata->bloque_inicial + tamanio_final;
    for(uint32_t i=posicion_final;i <= posicion_inicial;i++)
        bitarray_clean_bit(bitarray_bitmap,i);
    
    return true;


}

bool editar_archivo_metadata(char* path_metadata,t_dialfs_metadata* metadata){
    archivo_metadata = fopen(path_metadata,"w");
    char* txt = malloc(50);
    sprintf(txt,"BLOQUE_INICIAL=%d TAMANIO_ARCHIVO=%d",metadata->bloque_inicial,metadata->tamanio_archivo);
    txt_write_in_file(archivo_metadata,txt);
    fclose(archivo_metadata);
    free(txt);
    return true;
}


t_dialfs_metadata* create_metadata(char* nombre_archivo){
    t_dialfs_metadata* metadata = malloc(sizeof(t_dialfs_metadata));
    metadata->bloque_inicial= (uint32_t)asignar_bloque_inicial();
    metadata->tamanio_archivo = 0;
    metadata->nombre_archivo=nombre_archivo;
    return metadata;

}


int asignar_bloque_inicial(){
   int i;
   bool flag_test_bit;
   for(i=0;i<config->BLOCK_COUNT;i++){
        flag_test_bit=bitarray_test_bit(bitarray_bitmap,i);
        if(!flag_test_bit){
            bitarray_set_bit(bitarray_bitmap,i);
            return i;
        }
   }
    return -1;

}

// t_bitarray* obtener_bitmap(){
//     archivo_bitmap = fopen(path_bitmap,"r");
//     char* bitmap = malloc(config->BLOCK_COUNT);
//     fread(bitmap,config->BLOCK_COUNT,1, archivo_bitmap);
//     fclose(archivo_bitmap);
//     return bitarray_create_with_mode(bitmap,config->BLOCK_COUNT,LSB_FIRST);
// }



void loguear_bitmap(t_bitarray* bitmap){
    loguear("|    N  | v |");
    loguear("-------------");
    for(int i=0;i<config->BLOCK_COUNT;i++)
        loguear("| %4d  | %1d |",i,bitarray_test_bit(bitmap,i));
}