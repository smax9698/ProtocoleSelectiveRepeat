#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

void test_protocole_fiable(void)
{
  printf("TEST 1 : lien fiable\n");
  system("./test_fiable.sh");
  int ex_status = system("diff test_out_fiable.txt test_in.txt");

  CU_ASSERT_EQUAL(WEXITSTATUS(ex_status),0);
}

void test_protocole_lost(void)
{
  printf("TEST 2 : pertes\n");
  system("./test_lost.sh");
  int ex_status = system("diff test_out_lost.txt test_in.txt");

  CU_ASSERT_EQUAL(WEXITSTATUS(ex_status),0);
}

void test_protocole_delay(void)
{
  printf("TEST 3 : délais\n");
  system("./test_delay.sh");
  int ex_status = system("diff test_out_delay.txt test_in.txt");

  CU_ASSERT_EQUAL(WEXITSTATUS(ex_status),0);
}

void test_protocole_corrupt(void)
{
  printf("TEST 4 : corruptions de données\n");
  system("./test_corrupt.sh");
  int ex_status = system("diff test_out_corrupt.txt test_in.txt");

  CU_ASSERT_EQUAL(WEXITSTATUS(ex_status),0);
}

void test_protocole_mix(void)
{
  printf("TEST 5 : toutes les imperfections possibles\n");
  system("./test_mix.sh");
  int ex_status = system("diff test_out_mix.txt test_in.txt");

  CU_ASSERT_EQUAL(WEXITSTATUS(ex_status),0);
}

int main(int argc, char const *argv[])
{
    printf("Lancement des tests\nTemps estimé : 50s");
    CU_pSuite pSuite = NULL;

    if(CUE_SUCCESS != CU_initialize_registry())
       return CU_get_error();

    pSuite = CU_add_suite("Suite de tests sur le protocole", NULL, NULL);
    if(NULL == pSuite) {
       CU_cleanup_registry();
       return CU_get_error();
    }


    if(NULL == CU_add_test(pSuite, "test du protocole sans pertes, corruptions et délais", test_protocole_fiable) ||
      NULL == CU_add_test(pSuite, "test du protocole avec perte", test_protocole_lost) ||
      NULL == CU_add_test(pSuite, "test du protocole avec délais", test_protocole_delay) ||
      NULL == CU_add_test(pSuite, "test du protocole avec corruption de données", test_protocole_corrupt) ||
      NULL == CU_add_test(pSuite, "test du protocole avec toutes les imperfections possibles", test_protocole_mix)) {
       CU_cleanup_registry();
       return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_SILENT);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
