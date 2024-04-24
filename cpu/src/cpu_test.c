#include "cpu_test.h"

void test_instrucciones(t_pcb* pcb){
    bool flag_fetch=fetch(pcb);
    bool flag_decode = decode();
    if(flag_decode == false){ loguear_error("%s",registros_cpu->IR); return EXIT_FAILURE;}
    loguear("ID:%s P1:%i P2:%i",
    registros_cpu->INSTID,
    *(uint32_t*)registros_cpu->PARAM1,
    *(uint32_t*)registros_cpu->PARAM2);
    bool flag_exe = execute();
    if(flag_exe==false){loguear_error("No se pudo ejecutar"); return EXIT_FAILURE;}
    loguear("AX: %d", registros_cpu->AX);
}