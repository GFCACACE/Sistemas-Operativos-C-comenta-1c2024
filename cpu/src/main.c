#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <unistd.h>
#include "cpu.h"
#include "test.h"


int main_cpu(int argc, char** argv) {
    
    bool cpu_iniciada = iniciar_cpu(argv[1]);
    if(!cpu_iniciada){ 
        finalizar_cpu();
        return EXIT_FAILURE;
        }

    t_pcb* pcb_prueba = pcb_create("programa5.txt");
    loguear_pcb(pcb_prueba);
    //ejecutar_programa(pcb_prueba);
    //pcb_destroy(pcb_prueba);
    ciclo_de_instruccion(pcb_prueba);
    pcb_destroy(pcb_prueba);
    finalizar_cpu();
    

    return EXIT_SUCCESS;

}

int main(int argc, char** argv) {
        if(argc > 1 && strcmp(argv[1],"-test")==0)
        run_tests();
    else 
        main_cpu(argc,argv);
}