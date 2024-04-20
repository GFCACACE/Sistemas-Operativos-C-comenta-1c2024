#include "cpu_test.h"

void test_instrucciones(){
    registros_cpu->BX=1;
    registros_cpu->CX=2;
    exe_set(&registros_cpu->AX,registros_cpu->CX);
    loguear("SET AX CX -> AX: %d CX:%d",registros_cpu->AX,registros_cpu->CX);
    exe_sum(&registros_cpu->AX,registros_cpu->BX);
    loguear("SUM AX BX -> AX: %d",registros_cpu->AX);
    exe_sub(&registros_cpu->AX,registros_cpu->BX);
    loguear("SUB AX BX -> AX: %d",registros_cpu->AX);
    exe_jnz(&registros_cpu->AX,(uint32_t)15);
    loguear("JNZ AX 15 -> PC: %d",registros_cpu->PC);
    registros_cpu->IR="SET AX 45";
    bool flag_decode = decode();
    if(flag_decode == false){ loguear_error("%s",registros_cpu->IR); return EXIT_FAILURE;}
    loguear("ID:%s P1:%d P2:%d",
    registros_cpu->INSTID,
    (uint32_t**)registros_cpu->PARAM1,
    (uint32_t)registros_cpu->PARAM2);
    bool flag_exe = execute();
    if(flag_exe==false){loguear_error("No se pudo ejecutar"); return EXIT_FAILURE;}
    loguear("AX: %d", registros_cpu->AX);
}