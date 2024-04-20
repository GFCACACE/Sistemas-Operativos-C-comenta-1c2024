#include "test.h"

// Función que suma dos números enteros
int sum(int a, int b) {
    return a + b;
}
t_dictionary * comandos_consola_test;

bool moke_ejecutar_script(){
    printf("Simulo ejecutar script");
    return true;
}

bool moke_iniciar_proceso(){
    printf("Simulo iniciar_proceso");
    return true;
}

void agregar_comando_test(op_code_kernel code,char* nombre,char* params,void* funcion){
    
    t_comando_consola* comando = malloc(sizeof(t_comando_consola));
    comando->comando = code;
    comando->parametros = params;
    comando->funcion = funcion;

    void _agregar_comando_(char* texto){
        dictionary_put(comandos_consola_test,texto,comando);
    }

    _agregar_comando_(string_itoa(code));
    _agregar_comando_(nombre);

}


bool existe_comando_test(char* comando){
   return (dictionary_has_key(comandos_consola_test,comando));
}

bool iniciar_comandos_test(){
        comandos_consola_test =  dictionary_create();
    agregar_comando(EJECUTAR_SCRIPT,"EJECUTAR_SCRIPT","[PATH]",&moke_ejecutar_script);
    agregar_comando(INICIAR_PROCESO,"INICIAR_PROCESO","[PATH] [PRIORIDAD]",&moke_iniciar_proceso);

   
    char* ingreso=string_new();
    string_append(&ingreso,"iniciar_processo");
    string_to_upper(ingreso);
    bool existe = existe_comando_test(ingreso);
    printf("existe %i", existe);
    if(existe){
        t_comando_consola* comando = dictionary_get(comandos_consola_test,ingreso);
        comando->funcion(NULL);
    }

    return 0;
}

bool iniciar_kernel_prueba(){
 return   iniciar_logger_config("./kernel.config")&&
	    inicializar_comandos();    
}

// Función de inicialización de las pruebas
int init_suite(void) {
     
    system("clear");
   
    if(  iniciar_kernel_prueba())
     consola();
    
    return 0;
}

// Función de limpieza de las pruebas
int clean_suite(void) {

    return 0; // Todo está bien
}

// Test unitario para la función sum
void test_sum() {
    CU_ASSERT_EQUAL(sum(1, 2), 3);
    CU_ASSERT_EQUAL(sum(-1, 1), 0);
    CU_ASSERT_EQUAL(sum(0, 0), 0);
    CU_ASSERT_EQUAL(sum(10, -5), 5);
}


int run_tests() {
    // Ejecutar el test
    
   // Inicializar el registro de pruebas
    CU_initialize_registry();

    // Añadir la suite de pruebas al registro
    CU_pSuite suite = CU_add_suite("Suite de Pruebas", init_suite, clean_suite);

    // Añadir las pruebas a la suite
    CU_add_test(suite, "test_sum", test_sum);   

    resumen_tests();


    return 0;
}