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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include "sdb.h"
#define MAX_TOKEN_NUM 32000
enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NUM,       // For numbers
  TK_HEXNUM,    // For hex numbers
  TK_PLUS,      // For '+'
  TK_MINUS,     // For '-'
  TK_MULTIPLY,  // For '*'
  TK_DIVIDE,    // For '/'
  TK_LEFT_PAREN, // For '('
  TK_RIGHT_PAREN, // For ')'
  TK_UNSIGNED,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\(unsigned\\)", TK_UNSIGNED}, // matching unsigned, top priority, so placed at index 0
  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},     // plus
  {"==", TK_EQ},        // equal
  {"[0-9]+", TK_NUM},   // numbers (decimal)
  {"0[xX][0-9a-fA-F]+", TK_HEXNUM}, //numbers (hex)
  {"-", TK_MINUS},      // minus
  {"\\*", TK_MULTIPLY}, // multiply
  {"/", TK_DIVIDE},     // divide
  {"\\(", TK_LEFT_PAREN},  // left parenthesis
  {"\\)", TK_RIGHT_PAREN}, // right parenthesis
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[MAX_TOKEN_NUM] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

/*DEBUG Function*/
static void print_token(int start, int end){
  for (int i = start; i <= end; i++)
  {
    printf("%s ", tokens[i].str);
  }
  printf("\n\n");
  
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
        printf("position=%d\n",position);
        /* TODO(DONE): Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(substr_len>=32){
          panic("token buffer overflow");
        }
        if (nr_token >= MAX_TOKEN_NUM) {
          panic("Too many tokens\n");
        }
        switch (rules[i].token_type) {
          case TK_NOTYPE: case TK_UNSIGNED: break;
          default: 
            Token new_token = { .type = rules[i].token_type };
            tokens[nr_token] = new_token; //nr_token is the top counter
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0'; // Important, debugged for so long...
            nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


static word_t parseHexStringToInt(const char* hexString) {
    if (hexString == NULL || hexString[0] == '\0') {
        panic("Invalid input: NULL or empty string\n");
        return false; 
    }

    int offset = 0;
    if (strncmp(hexString, "0x", 2) == 0) {
        offset = 2;
    }
    else {panic("hex number doesn't start with \"0x\"");}

    // Use strtol to convert the hex string to an integer
    char* endptr;
    long int result = strtol(hexString + offset, &endptr, 16);

    // Check for conversion errors
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid input: Not a valid hexadecimal number\n");
        return false;
    }
    return (word_t)result; // Cast the long integer result to int
}

int find_main_operator(int p, int q, bool *success) {
  // Find the operator with the lowest precedence
  int parentheses_count = 0;
  int main_operator = -1;
  int lowest_precedence = 999;

  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_LEFT_PAREN) {
      parentheses_count++;
    } else if (tokens[i].type == TK_RIGHT_PAREN) {
      parentheses_count--;
    } else if (parentheses_count == 0) {
      // Only consider operators outside of parentheses
      if (tokens[i].type == TK_PLUS || tokens[i].type == TK_MINUS) {
        int precedence = 1;
        if (precedence <= lowest_precedence) {
          lowest_precedence = precedence;
          main_operator = i;
        }
      } else if (tokens[i].type == TK_MULTIPLY || tokens[i].type == TK_DIVIDE) {
        int precedence = 2;
        if (precedence <= lowest_precedence) {
          lowest_precedence = precedence;
          main_operator = i;
        }
      } else if (tokens[i].type == TK_EQ) {
        int precedence = 0;
        if (precedence <= lowest_precedence) {
          lowest_precedence = precedence;
          main_operator = i;
        }
      }
    }
  }

  if (parentheses_count != 0) {
    // Unmatched parentheses
    *success = false;
    return -1;
  }

  return main_operator;
}

bool check_parentheses(int p, int q) {
  if (tokens[p].type != TK_LEFT_PAREN || tokens[q].type != TK_RIGHT_PAREN) {
    /* Mismatched parentheses */
    return false;
  }

  int balance = 0;
  for (int i = p; i < q; i++) {
    if (tokens[i].type == TK_LEFT_PAREN) {
      balance++;
    } else if (tokens[i].type == TK_RIGHT_PAREN) {
      balance--;
      if (balance <= 0) {
        /* Unmatched closing parenthesis */
        return false;
      }
    }
  }

  return (balance == 1);
}

word_t evaluate_expression(int p, int q, bool *success) {
  printf("eval from %d to %d\n", p,q); //debug
  print_token(p, q); //DEBUG
  if (*success == false) {
    return 0;
  }
  
  if (p > q) {
    *success = false;
    return 0;
  }
  
  if (p == q) {
    // Single token in the expression
    if (tokens[p].type == TK_NUM) {
      return strtol(tokens[p].str, NULL, 0);
    }
    else if (tokens[p].type == TK_HEXNUM) {
      return parseHexStringToInt(tokens[p].str);
    }
    else {
      *success = false;
      return 0;
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return evaluate_expression(p + 1, q - 1, success);
  }
  
  int op = find_main_operator(p, q, success);
  
  if (!*success) {
    return 0;
  }
  
  // Evaluate the left and right sub-expressions
  word_t val1 = evaluate_expression(p, op - 1, success);
  word_t val2 = evaluate_expression(op + 1, q, success);
  
  if (!*success) {
    return 0;
  }
  word_t combVal;
  // printf("This is a test!");
  // Perform the operation based on the main operator
  switch (tokens[op].type) {
    case TK_PLUS:
      combVal = val1 + val2;
      break;
    case TK_MINUS:
      combVal = val1 - val2;
      break;
    case TK_MULTIPLY:
      combVal = val1 * val2;
      break;
    case TK_DIVIDE:
      if (val2 == 0) { // div by 0
        *success = false;
        combVal = 0;
      }
      combVal = val1 / val2;
      break;
    case TK_EQ:
      combVal = val1 == val2;
      break;
    default:
      // Invalid operator
      *success = false;
      combVal = 0;
  }
  printf("combine value=%u\n", combVal);
  return combVal;
}



word_t expr(char *e, bool *success) {
  memset(tokens, 0, sizeof(tokens));
  if (!make_token(e)) {
    *success = false;
    printf("make token failed\n");
    return 0;
  }

  print_token(0, nr_token);//DEBUG

  /* Insert codes to evaluate the expression. DONE*/

   return evaluate_expression(0, nr_token - 1, success);
}