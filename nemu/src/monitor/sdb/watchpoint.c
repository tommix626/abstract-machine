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

#include <monitor/watchpoint.h>

//struct watchpoint (WP) is defined in watchpoint.h

static WP wp_pool[NR_WP] = {}; //NOTE:static var - cannot access from outside! ie: cannot use extern on them in other files
static WP *head = NULL, *free_ = NULL;

static void reset_wp(WP* wp, word_t initial_value){
  wp->enabled=true;
  wp->hit_count=0;
  wp->old_value=initial_value;
  wp->str[0]='\0';
}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

/// @brief check and return an idle wp.
/// @param initial_val the initial value of the wp
/// @return the idle wp, null if non-exist(also assert 0)
WP* get_idle_wp(word_t initial_val){
  if (free_ == NULL) {
    panic("Reach maximum watchpoint limit!\n");
    assert(0);
    return NULL;
  }

  WP* idle = free_;
  free_ = free_->next;
  //prepend idle to head
  idle->next = head;
  head = idle;
  reset_wp(idle,initial_val);
  return idle;
}
/// @brief get a idle wp from pool
/// @param initial_val the initial value of the wp
/// @return the new watch point
WP* new_wp(word_t initial_val){
  return get_idle_wp(initial_val);
}

/// @brief free the given watchpoint
/// @param num the number of the wp to be freed.
void free_wp(int num){
  if (num == head->NO) {
    WP* wp = head;
    head = head->next;
    wp->next = free_;
    free_ = wp;
    return;
  }
  WP* curr = head;
  while (curr->next != NULL) {
    if (curr->next->NO == num) { //Doubts:wp==curr vs curr->NO == wp->NO? Which is better? Answer: Num should be used as interface since we are using singly linked-list
      WP* wp = curr->next;
      curr->next = curr->next->next;
      wp->next = free_;
      free_ = wp;
      return;
    }
    curr = curr->next;
  }
  printf("Pass invalid wp number!\n");
  
}


/// @brief check if any watchpoint has changed in value.
/// @param stop_flag set *stop_flag to false if there's change in any watchpoint.
void check_watchpoint(bool *stop_flag){
  WP* curr = head;
  bool flag = true;
  while (curr != NULL) {
    if(!curr->enabled){curr =curr->next; continue;}

    flag = true;
    word_t expr_val=expr(curr->str, &flag);
    if(!flag) {
      Log("watchpoint %d has invalid EXPR\n",curr->NO);
    }
    else if (expr_val != curr->old_value) {
      Log("Watchpoint %d value change from %u to %u, EXPR=%s\n",curr->NO,curr->old_value,expr_val,curr->str);
      *stop_flag = false; // need a stop
      curr->old_value = expr_val; //update old val to new one
      curr->hit_count++;
    }
    curr = curr->next;
  }
}

void print_watchpoint() {
    WP *current = head;

    // Check if there are any watchpoints set
    if (current == NULL) {
        printf("No watchpoints set.\n");
        return;
    }

    printf("Num\tType\t\tDisp\tEnb\tAddress\t\tWhat\n");
    while (current != NULL) {
        printf("%d\twatchpoint\tkeep\t%s\t%s\t\t%s\n", 
               current->NO, 
               (current->enabled ? "y" : "n"), 
               "N/A",//(current->address ? in(current->address) : "N/A"), 
               current->str);

        if (current->hit_count > 0) {
            printf("\tbreakpoint already hit %d times\n", current->hit_count);
        }

        current = current->next;
    }
}
/*
Num     Type           Disp Enb Address    What
2       watchpoint     keep y              $rax
	breakpoint already hit 4 times
3       watchpoint     keep y              $rdx
*/
