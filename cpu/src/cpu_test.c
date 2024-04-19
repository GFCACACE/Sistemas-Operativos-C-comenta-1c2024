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
    loguear("ID:%s P1:%s P2:%s P3:%s",registros_cpu->INSTID,registros_cpu->PARAM1,registros_cpu->PARAM2,registros_cpu->PARAM3);

}