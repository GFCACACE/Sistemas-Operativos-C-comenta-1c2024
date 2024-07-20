#include "stdin.h"


char* leer_texto_consola(){
    return readline(">");
}


void io_stdin_read(t_direcciones_proceso* direcciones_proceso, int conexion){
    
    t_pid_valor pid_valor = direcciones_proceso->pid_size_total;
    
    if(list_size(direcciones_proceso->direcciones) <= 0)
        return;
        
    char* texto = malloc(100);
    char* texto_a_enviar = malloc(pid_valor.valor);
    texto = leer_texto_consola();
    while(strlen(texto)  < pid_valor.valor){
        loguear_error("La cadena escrita es de %d bytes, debe ser de %d bytes.",strlen(texto),pid_valor.valor);
        texto = leer_texto_consola();
    }
    loguear("TEXTO INGRESADO: %s",texto);
    memcpy(texto_a_enviar,texto,pid_valor.valor);
    loguear("TEXTO A ENVIAR: %s",texto_a_enviar);
    escribir_memoria_completa_io(direcciones_proceso, texto_a_enviar, conexion,ESCRITURA_MEMORIA);
    free(texto);
    free(texto_a_enviar);
}