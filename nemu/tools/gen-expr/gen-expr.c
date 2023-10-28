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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[655360] = {};
static char code_buf[655360 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = ((unsigned) 0) + %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

// Generate a random number between 0 and n-1
uint32_t choose(uint32_t n) {
    // Ensure n is greater than 0 to avoid division by zero
    if (n == 0) {
        fprintf(stderr, "Error: choose() called with n=0\n");
        exit(1);
    }

    int random_num = rand();
    uint32_t result = (uint32_t)((double)random_num / RAND_MAX * n);
    assert(result<n && result>=0);
    return result;
}

void gen(char c, char* buf) {
  int len = strlen(buf);
  strncat(buf, &c,1);
}

//add number to buf
static char * gen_num(char* buf){
  uint32_t number = choose(0xFFFFFFFF);
  char number_buf[100]; 
  sprintf(number_buf, "%u", number);
  strcat(buf,number_buf);
}

//add op to buf
static char * gen_rand_op(){
  char operators[] = "+-*/";
  char random_operator = operators[choose(strlen(operators))];
  assert(random_operator);
  gen(random_operator,buf); 
}


static void gen_rand_expr(char* buf, int cnt) {
  
  strcat(buf, "(unsigned)" );
  //  strcpy(buf,  gen_rand_expr_inner(100));
  if(cnt < 0){
    gen_num(buf);
    return;
  }
  switch (choose(3)) {
    case 0: gen_num(buf); break;
    case 1: 
      gen('(',buf); 
      gen_rand_expr(buf,cnt-1); 
      gen(')', buf); 
      break;
    default: 
      gen_rand_expr(buf,cnt/2);
      gen_rand_op(buf); 
      gen_rand_expr(buf,cnt/2); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(0);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  // loop = 300;
  for (i = 0; i < loop; i ++) {
    memset(buf, '\0', sizeof(buf));
    gen_rand_expr(buf,10000);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("./tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc ./tmp/.code.c -o ./tmp/.expr");
    if (ret != 0) continue;

    fp = popen("./tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
