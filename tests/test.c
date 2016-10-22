#include <stdlib.h>
#include <stdio.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

void test_protocole_fiable(void)
{

}


int main(int argc, char const *argv[])
{

    CU_pSuite pSuite = NULL;

    if(CUE_SUCCESS != CU_initialize_registry())
       return CU_get_error();

    pSuite = CU_add_suite("Suite de tests sur le protocole", NULL, NULL);
    if(NULL == pSuite) {
       CU_cleanup_registry();
       return CU_get_error();
    }


    if(NULL == CU_add_test(pSuite, "test du protocole sans pertes, corruptions et délais", test_protocole_fiable)) {
       CU_cleanup_registry();
       return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_SILENT);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
