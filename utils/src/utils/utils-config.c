#include "utils-config.h"
t_log* logger;
t_config* iniciar_config(char* nombre_archivo)
{
     char* path = construir_path_config(nombre_archivo);

    if(path==NULL)	{		
        loguear( "No se pudo crear el config! No se especificó un nombre de archivo");
        exit(EXIT_FAILURE);
    };

   
	t_config* nuevo_config;
	char *current_dir = getcwd(NULL, 0);
    printf("El directorio actual es %s\n", current_dir);
    free(current_dir);
	nuevo_config = config_create(path);
    free(path);

    if(nuevo_config==NULL)	{		
        loguear( "No se pudo crear el config! %s",nombre_archivo);
        exit(EXIT_FAILURE);
    };


	return nuevo_config;
}

t_log* iniciar_logger(char* nombre_archivo)
{
    char *titulo = string_duplicate(nombre_archivo);
    string_to_upper(titulo);
    char* path = construir_path_log(nombre_archivo);
	t_log* nuevo_logger =log_create(path,titulo,1,LOG_LEVEL_INFO);
    free(path);
    free(titulo);

	return nuevo_logger;
}

char* construir_path_log(char* nombre_archivo){
    return construir_path(nombre_archivo,".log");
}

char* construir_path_config(char* nombre_archivo){
    if(nombre_archivo!=NULL)
    return construir_path(nombre_archivo,".config");
    else
        return NULL;
}

char* construir_path(char* nombre_archivo,char* extension){
    char* path_char =  "";    
    char* path =malloc(strlen(path_char)+strlen(nombre_archivo)+strlen(extension) +1);
        if (path == NULL) {
        fprintf(stderr, "Memory allocation failed in construir_path.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(path, path_char);
    strcat(path,nombre_archivo);
    strcat(path,extension);
    return path;
}


pthread_mutex_t mx_log = PTHREAD_MUTEX_INITIALIZER;

void loguear(const char* message_template, ...) {
	
	char*message;
	va_list arguments;
   	va_start(arguments, message_template);			
	va_end(arguments);		
	message = string_from_vformat(message_template, arguments);

	pthread_mutex_lock(&mx_log);
	log_info(logger,message,"");
	pthread_mutex_unlock(&mx_log);

	free(message);

}

void loguear_error(const char* message_template, ...) {
	
	char*message;
	va_list arguments;
   	va_start(arguments, message_template);			
	va_end(arguments);		
	message = string_from_vformat(message_template, arguments);

	pthread_mutex_lock(&mx_log);
	log_error(logger,message,"");
	pthread_mutex_unlock(&mx_log);

	free(message);

}