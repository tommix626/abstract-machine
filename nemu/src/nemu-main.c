/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include "./monitor/sdb/sdb.h" //include for unit test on EXPR

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
static void test_expr();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
  /*Unit test*/
  test_expr();
  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}


/// @brief test th expr evaluation function
void test_expr()
{
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  FILE *file = fopen("./src/testexpr", "r");
  if (!file) {
      perror("Unable to open the file \"input\"");
      return;
  }

  while ((read = getline(&line, &len, file)) != -1) {
      // printf("Read line: %s", line);
      
      /* extract the first token as the number */
      char *result_str = strtok(line, " ");
      if (result_str == NULL) { assert(0); continue; }
      uint32_t result = strtol(result_str, NULL, 0);
      // printf("Result= %u\n", result);
      /* treat the remaining string as the expression*/
      // char *expr = result_str + strlen(result_str) + 1; //skip the command and get the args.
      char *test_expr = strtok(NULL, "\n");
      // printf("EXPR= %s\n", test_expr);
       
      bool sflag = true;
      uint32_t parse_result = expr(test_expr, &sflag);
      // printf("Result= %u; \t\t Parse get %u\n", result,parse_result);
      assert(result==parse_result);
  }

  free(line);
  fclose(file); 

  printf("expr10000 test pass\n");
  // assert(0);
}