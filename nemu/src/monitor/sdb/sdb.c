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
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
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

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}



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
    
  }
    printf("Unknown command '%s'\n", arg);
  return 0;
}


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

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

// static int cmd_x(char *args) {
//   char *arg = strtok(NULL, " ");
//   int i;

//   if (arg == NULL) {
//     /* no argument given */
//     printf("Missing argument. Option: [r]egister /[w]atchpoint \n");
//     return 0;
//   }
//   else {
    
//   }
//     printf("Unknown command '%s'\n", arg);
//   return 0;
// }

// static int cmd_p(char *args) {
//   char *arg = strtok(NULL, " ");
//   int i;

//   if (arg == NULL) {
//     /* no argument given */
//     printf("Missing argument. Option: [r]egister /[w]atchpoint \n");
//     return 0;
//   }
//   else {
    
//   }
//     printf("Unknown command '%s'\n", arg);
//   return 0;
// }

// static int cmd_w(char *args) {
//   char *arg = strtok(NULL, " ");
//   int i;

//   if (arg == NULL) {
//     /* no argument given */
//     printf("Missing argument. Option: [r]egister /[w]atchpoint \n");
//     return 0;
//   }
//   else {
    
//   }
//     printf("Unknown command '%s'\n", arg);
//   return 0;
// }

// static int cmd_d(char *args) {
//   char *arg = strtok(NULL, " ");
//   int i;

//   if (arg == NULL) {
//     /* no argument given */
//     printf("Missing argument. Option: [r]egister /[w]atchpoint \n");
//     return 0;
//   }
//   else {
    
//   }
//     printf("Unknown command '%s'\n", arg);
//   return 0;
// }

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands */
  { "si", "[N] Step through N times", cmd_si },
  { "info","[r/w] print information", cmd_info },
  // { "x","[N EXPR] scan X memory at the place EXPR", cmd_x },
  // { "p","print", cmd_p },
  // { "w","set watchpoint", cmd_w },
  // { "d","delete watchpoint", cmd_d },

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
}
