#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <unistd.h>
#include "cpu.h"
#include "cpu_test.h"



int main(int argc, char** argv) {
    
    bool flag_iniciar_cpu = iniciar_cpu(argv[1]);
    if(flag_iniciar_cpu == false){ 
        finalizar_cpu();
        return EXIT_FAILURE;
        }

    return EXIT_SUCCESS;

}
