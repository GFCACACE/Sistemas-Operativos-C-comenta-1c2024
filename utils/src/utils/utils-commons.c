#include "utils-commons.h"
int ultimo_pid=0;

t_pcb* pcb_create(char* path_programa){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->PID = ultimo_pid++;
	pcb->archivos_abiertos = list_create();
	pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
	pcb->registros_cpu->AX=pcb->registros_cpu->BX=pcb->registros_cpu->CX=pcb->registros_cpu->DX=0;
	pcb->program_counter = 0;    
    pcb->path = string_duplicate(path_programa);
   	return pcb;
}

void pcb_destroy(t_pcb* pcb){
	list_destroy(pcb->archivos_abiertos);
	free(pcb->registros_cpu);
    free(pcb->path);
	free(pcb);
}

void loguear_pcb(t_pcb* pcb){
	printf("LOGUEO PCB\n");
	loguear("PID: %d",pcb->PID);
	loguear("program_counter: %d",pcb->program_counter);
	loguear("Prioridad: %d",pcb->prioridad);
	loguear("Reg AX: %d",pcb->registros_cpu->AX);
	loguear("Reg BX: %d",pcb->registros_cpu->BX);
	loguear("Reg CX: %d",pcb->registros_cpu->CX);
	loguear("Reg DX: %d",pcb->registros_cpu->DX);
	loguear("Cant. de archivos abiertos: %d",list_size( pcb->archivos_abiertos));
    loguear("PATH: %s",pcb->path);
	printf("==========================----- \n");

	printf("FIN LOGUEO PCB\n");
}


bool is_numeric(const char* str) {
    if (str == NULL || *str == '\0') {
        // Handle empty strings or NULL pointers.
        return false;
    }

    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            // If any character is not a digit, return false.
            return false;
        }
    }

    return true;
}


void path_resolve(char* dest, const char* path, const char* filename)
{
    if(path == NULL && filename == NULL) {
        strcpy(dest, "");;
    }
    else if(filename == NULL || strlen(filename) == 0) {
        strcpy(dest, path);
    }
    else if(path == NULL || strlen(path) == 0) {
        strcpy(dest, filename);
    } 
    else {
        char directory_separator[] = "/";
#ifdef WIN32
        directory_separator[0] = '\\';
#endif
        const char *last_char = path;
        while(*last_char != '\0')
            last_char++;        
        int append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(dest, path);
        if(append_directory_separator)
            strcat(dest, directory_separator);
        strcat(dest, filename);
    }
}

char * uint_a_string(uint num){
    char* string = malloc(sizeof(char) * 20);
	sprintf(string,"%u", num);
    return string;
}


t_list* get_instrucciones(char* path_inicial,char *nombre_archivo)
{
	t_list *lista_instrucciones = list_create();
	char* path = string_duplicate(path_inicial);
	int path_size = strlen(path);;
	if(path[path_size - 1] != '/')
		string_append(&path, "/");
	string_append(&path, nombre_archivo);
	printf("sizeof(nombre_archivo) %ld",sizeof(nombre_archivo));
	
	FILE *archivo;

	archivo = fopen(path, "r");
	free(path);

	if (archivo == NULL)
	{
		loguear_error("No se pudo abrir el archivo %s \n", path);
		return NULL;
	}

	
	size_t line_size = 0;

	while (!feof(archivo))
	{

		char *linea = NULL;
		
		getline(&linea, &line_size, archivo);
		if (linea != NULL)
		{	int len = strlen(linea);
			if(linea[len - 1] == '\n')
      		{  //Eliminamos el salto de línea
        		linea[len - 1] = '\0';
				len--;
			}
			if(linea[len - 1] == '\r')
      		  //Eliminamos el retorno de carro.
        	linea[len - 1] = '\0';
			
			list_add(lista_instrucciones, linea);
			free(linea);

		}

	//	loguear("la linea es:%s\n", linea);

	}
	
	
	fclose(archivo);
	return lista_instrucciones;
}