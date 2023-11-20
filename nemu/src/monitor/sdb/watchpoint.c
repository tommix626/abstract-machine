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

#include "sdb.h"

#define NR_WP 32 //number of wp

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {}; //NOTE:static var - cannot access from outside! ie: cannot use extern on them in other files
static WP *head = NULL, *free_ = NULL;


WP* new_wp();
void free_wp(WP *wp);

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
/// @return the idle wp, null if non-exist(also assert 0)
WP* get_idle_wp(){
  if (free_ == NULL) {
    panic("Reach maximum watchpoint limit!");
    assert(0);
    return NULL;
  }

  WP* idle = free_;
  free_ = free_->next;
  //prepend idle to head
  idle->next = head;
  head = idle;
  return idle;
}
/// @brief get a idle wp from pool
/// @return the new watch point
WP* new_wp(){
  return get_idle_wp();
}

/// @brief free the given watchpoint
/// @param wp the wp reference to be freed.
void free_wp(WP *wp){
  if (wp == head) {
    head = head->next;
    wp->next = free_;
    free_ = wp;
    return;
  }
  WP* curr = head;
  while (curr->next != NULL) {
    if (curr->next == wp) { //Doubts: why not use curr->NO == wp->NO? Which is better?
      curr->next = curr->next->next;
      wp->next = free_;
      free_ = wp;
      return;
    }
  }
  panic("Pass invalid wp reference!");
  
}