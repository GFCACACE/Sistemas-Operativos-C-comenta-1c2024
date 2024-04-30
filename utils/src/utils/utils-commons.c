#include "utils-commons.h"
#
int ultimo_pid=0;

t_pcb* pcb_create(char* path_programa){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->PID = ultimo_pid++;
	pcb->archivos_abiertos = list_create();
	pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
	pcb->registros_cpu->AX=pcb->registros_cpu->BX=pcb->registros_cpu->CX=pcb->registros_cpu->DX=0;
	pcb->registros_cpu->EAX=pcb->registros_cpu->EBX=pcb->registros_cpu->ECX=pcb->registros_cpu->EDX=pcb->registros_cpu->SI=pcb->registros_cpu->DI=0;
	pcb->program_counter = 0;    
	pcb->quantum = 0; // En caso de necesitarlo, el planificador de corto plazo lo inicializará con el valor adecuado 
    pcb->path = string_duplicate(path_programa);
   	return pcb;
}

t_pcb* pcb_create_quantum(char* path_programa,int quantum){
	t_pcb* pcb = pcb_create(path_programa);
	pcb->quantum=quantum;
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
	loguear("Quantum: %d", pcb->quantum);
	loguear("Reg AX: %d",pcb->registros_cpu->AX);
	loguear("Reg BX: %d",pcb->registros_cpu->BX);
	loguear("Reg CX: %d",pcb->registros_cpu->CX);
	loguear("Reg DX: %d",pcb->registros_cpu->DX);
	loguear("Reg EAX: %d",pcb->registros_cpu->EAX);
	loguear("Reg EBX: %d",pcb->registros_cpu->EBX);
	loguear("Reg ECX: %d",pcb->registros_cpu->ECX);
	loguear("Reg EDX: %d",pcb->registros_cpu->EDX);
	loguear("Reg SI: %d",pcb->registros_cpu->SI);
	loguear("Reg DI: %d",pcb->registros_cpu->DI);
	loguear("Cant. de archivos abiertos: %d",list_size( pcb->archivos_abiertos));
    loguear("PATH: %s",pcb->path);
	printf("==========================----- \n");

	printf("FIN LOGUEO PCB\n");
}


bool is_numeric(const char* str) {
    if (str == NULL || *str == '\0') 
        return false;
    for (int i = 0; str[i] != '\0'; i++) 
        if (!isdigit(str[i])) 
            // If any character s not a digit, return false.
            return false;
    return true;
}


char* path_resolve(char* directorio, char* nombre_archivo)
{
   char* path = string_duplicate(directorio);
	int path_size = strlen(path);;
	if(path[path_size - 1] != '/')
		string_append(&path, "/");
	string_append(&path, nombre_archivo);
	return path;
}

char * uint_a_string(uint num){
    char* string = malloc(sizeof(char) * 20);
	sprintf(string,"%u", num);
    return string;
}

void quitar_salto_linea(char* linea){
	if(linea!=NULL){
		int len = strlen(linea);
		if(linea[len - 1] == '\n')
		{  //Eliminamos el salto de línea
			linea[len - 1] = '\0';
			len--;
		}
		if(linea[len - 1] == '\r')
			//Eliminamos el retorno de carro.
		linea[len - 1] = '\0';
	}
}

FILE* abrir_archivo(char* directorio,char* nombre_archivo){
char* path = path_resolve(directorio,nombre_archivo);
	
	FILE *archivo;

	archivo = fopen(path, "r");
	free(path);

	if (archivo == NULL)
	{
		perror("Error al abrir el archivo");
		loguear_error("No se pudo abrir el archivo %s \n", path);
		return NULL;
	}

	return archivo;
}


t_list* get_instrucciones(char* directorio,char *nombre_archivo)
{
	t_list *lista_instrucciones = list_create();
	FILE *archivo = abrir_archivo(directorio,nombre_archivo);
	
	if (archivo == NULL)		
		return NULL;
	
	size_t line_size = 0;

	while (!feof(archivo))
	{

		char *linea = NULL;
		
		getline(&linea, &line_size, archivo);
		quitar_salto_linea(linea);
		if (linea != NULL)					
			list_add(lista_instrucciones, linea);
	}
	
	
	fclose(archivo);
	return lista_instrucciones;
}

char *leer_linea_i(FILE *archivo, int posicion) {
    char *linea = NULL;
    size_t longitud = 0;
    ssize_t caracteres_leidos;
    int num_linea = 0;

    // Leer líneas hasta llegar a la posición deseada
    while (num_linea <= posicion && (caracteres_leidos = getline(&linea, &longitud, archivo)) != -1) 
        num_linea++;
    

    // Si la posición especificada excede el número de líneas en el archivo
    if (num_linea < posicion) {
        free(linea);
        return NULL;
    }

    // Si la línea se leyó correctamente, devolverla
    return linea;
}

char* get_linea_archivo(char* directorio,char* nombre_archivo,int posicion){

	FILE *archivo = abrir_archivo(directorio,nombre_archivo);
	if (archivo == NULL)		
		return NULL;
	char *linea = leer_linea_i(archivo, posicion);
	quitar_salto_linea(linea);			
	fclose(archivo);

	return linea;

}