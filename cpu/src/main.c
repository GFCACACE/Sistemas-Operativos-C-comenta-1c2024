#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <unistd.h>
#include "cpu.h"
#include "test.h"

pthread_t tid;

void* gestionar_interrupcion(){
    int estado_guardado;
    while(1){
        estado_guardado=recibir_operacion(kernel_interrupt);
        pthread_mutex_lock(&mutex_interrupt);
        cod_op_kernel_interrupt=estado_guardado;
        pthread_mutex_unlock(&mutex_interrupt);
    }

}
int main_cpu(int argc, char** argv) {
    int err;

    err = pthread_create(&(tid), NULL, gestionar_interrupcion, NULL);
    bool cpu_iniciada = iniciar_cpu(argv[1]);
    if(!cpu_iniciada){ 
        finalizar_cpu();
        return EXIT_FAILURE;
        }

    ejecutar_proceso_cpu();

    
    finalizar_cpu();
    
    pthread_join(tid, NULL);
    return EXIT_SUCCESS;

}

int main(int argc, char** argv) {
        if(argc > 1 && strcmp(argv[1],"-test")==0)
        run_tests();
    else 
        main_cpu(argc,argv);
}

