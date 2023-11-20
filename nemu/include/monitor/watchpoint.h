#include <monitor/sdb.h>

#define NR_WP 32 //number of wp


typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char str[3200]; // expr theis wp tracks
  word_t old_value; // last value
  bool active; //if wp is active

  /* TODO: Add more members if necessary */

} WP;


// from watchpoint.c
WP* new_wp();
void free_wp(WP *wp);
void check_watchpoint(bool* stop_flag);