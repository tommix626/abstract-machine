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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <monitor/sdb.h>
#include <monitor/watchpoint.h> // for watchpoint
#include <monitor/iringbuf.h> // for iringbuf

#include <memory/paddr.h>
// #include<common.h> //used for word_t !already imported in sdb.h
// #include "expr.c" //used for expr parsing !already imported in sdb.h

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void iringbuf_init();

/* We use the `readline` library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

/// @brief [c]ontinue program until done
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}



/// @brief print information
/// @param args [r/w]
///             r - register
///             w - watchpoint
static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  // int i;

  if (arg == NULL) {
    /* no argument given */
    printf("Missing argument. Option: [r]egister /[w]atchpoint \n");
    return 0;
  }
  else if (strcmp(arg,"r")==0){
    isa_reg_display(); //rax            0x0                 0
  }
  else if (strcmp(arg,"w")==0){
    print_watchpoint();

  }
  else {
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

// //helper method: parse the value of an expression.
// static bool parse_EXPR ( char * str, int * N_ptr ){
//   *N_ptr = 0x80000000;
//   return true;
// }



/// @brief helper method: read an int-like string `str` and change `N_ptr` into int. 
/// @return Return `false` if str is invalid int (eg. '23sfef').
static bool stringToInt( char * str, int * N_ptr ){
	*N_ptr = 0;
	while(*str != 0) {
		if( (isdigit(*str)) ) {
			*N_ptr *= 10;
			*N_ptr += (*str++) - '0';
		}
		else { return false;}
	}
	return true;
}

/// @brief stepi, run a program a single step
/// @param args int, how many stepi to repeat.
static int cmd_si( char *args ) {
	char *arg = strtok(NULL, " ");
	int N=0;
	if (arg == NULL) {
    /* no argument given */
		cpu_exec(1);
  }
  else {
		/* convert arg into int */
		bool flag = stringToInt(arg,&N);
		if (flag) {
      cpu_exec(N); //run main things
			// printf("running si successfully N=%d\n", N);
		}
		else { printf("invalid argument '%s'\n", arg);}
	}
	return 0;
}

/// @brief  quit the program
static int cmd_q(char *args) {
  return -1;
}

/// @brief print help message
static int cmd_help(char *args);

/// @brief scan memory
///         eg: x 10 0x80000000
/// @param args [N] EXPR
///             N - number to 4-byte to be evaluate
static int cmd_x(char *args) {
  /*eg: x 10 0x80000000*/
  int ori_arg_len = strlen(args);
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    /* no argument given */
    printf("Missing argument. usage: x [N EXPR] \n");
    return 0;
  }
  else {
    /*check args is int.*/
    int scan_len;
    bool flag = stringToInt(arg, &scan_len);
    if(!flag){
      printf("Invalid argument: N not int!\n");
      return 0;
    }
    char * arg2 = arg + strlen(arg) + 1; //skip to read next arg: EXPR
    if (ori_arg_len - strlen(arg) == 0 || *arg2 == '\0' || arg2 == NULL) {
      /* no second argument given */
      printf("Missing EXPR argument. usage: x [N EXPR] \n");
      return 0;
    }
    printf("EXPR = %s\n", arg2);
    flag = true;
    word_t expr_val=expr(arg2, &flag);
    if(!flag) {
      printf("Invalid EXPR argument.\n");
      return 0;
    }
    paddr_t start_addr = (paddr_t) expr_val; //FIXME: convert int to addr, might be buggy
    printf("x read %d memory starting from %#x\n", scan_len, start_addr); //debug

    for (int i = 0; i<scan_len; i++,start_addr++){ //Q:TODO: if start_addr++ is implemented using iterator, then we might get outofbound error here.
      word_t val = paddr_read(start_addr,4); // 4: reading 32-bit.
      printf("%#x: %-10d\t%#-10x\n",start_addr,val,val);
    }
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL || *args == '\0') {
    /* no argument given */
    printf("Missing argument. usage: p [EXPR] \n");
    return 0;
  }

  printf("EXPR = %s\n", args); //debug
  bool flag = true;
  word_t expr_val=expr(args, &flag);
  if(!flag) {
    printf("Invalid EXPR argument.\n");
    return 0;
  }
  printf("%u \t [%#x]\n",expr_val,expr_val);

  return 0;
}

/// @brief set watchpoint
///         eg: w *0x2000
/// @param args EXPR
///             N - number to 4-byte to be evaluate
static int cmd_w(char *args) {
  if (args == NULL || *args == '\0') {
    /* no argument given */
    printf("Missing argument. usage: w [EXPR] \n");
    return 0;
  }

  printf("EXPR = %s\n", args); //debug
  bool flag = true;
  word_t expr_val=expr(args, &flag);
  if(!flag) {
    printf("Invalid EXPR argument. Want to save the expr for future compilation? TODO:not implemented yet\n");
    //TODO
    return 0;
  }
  //store to an idle wp
  WP* idle = new_wp(expr_val);
  strcpy(idle->str,args);
  // idle->str = args; //why not a good idea? should copy to local mem,
  Log("Watchpoint %d added for EXPR=%s, initial value=%u\n",idle->NO, idle->str, idle->old_value);
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL || *args == '\0') {
    /* no argument given */
    printf("Missing argument. usage: d [Watchpoint_Number] \n");
    return 0;
  }
  char *arg1 = strtok(NULL, " ");
	int N=0;
	if (arg1 == NULL) {
    /* no argument given */
		panic("should not reach here!");
  }
  else {
		/* convert arg into int */
		bool flag = stringToInt(arg1,&N);
		if (flag) {
      free_wp(N);
		}
		else { printf("invalid argument '%s'\n", arg1);}
	}
	return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "[N] - Step through N times", cmd_si },
  { "info","rw - print information", cmd_info },
  { "x","N EXPR - scan N 4-byte memory starting at the place EXPR", cmd_x },
  { "p","EXPR - print expression", cmd_p },
  { "w","EXPR - set watchpoint", cmd_w },
  { "d","delete watchpoint", cmd_d },

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
  }
    printf("Unknown command '%s'\n", arg);
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1; //skip the command and get the args.
    if (args >= str_end) { // if skipping the cmd exceeds the end of the str addr, then no args.
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    /*go through command in the `cmd_table` and try to match*/
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }
    // if went through all the command. (exiting the loop by violating the loop constriant instead of using the break statement)
    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();

  /* init iringbuf*/
  IFDEF(CONFIG_ITRACE, iringbuf_init());
}
