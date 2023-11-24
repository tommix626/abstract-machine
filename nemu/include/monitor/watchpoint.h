#include <monitor/sdb.h>

#define NR_WP 32 //number of wp


typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char str[3200]; // expr theis wp tracks
  word_t old_value; // last value
  bool enabled; //if wp is active
  int hit_count;
  // word_t address; this is extra feature that I don't want to implement now!
  /* TODO: Add more members if necessary */

} WP;


// from watchpoint.c
WP* new_wp();
void free_wp(int num);
void check_watchpoint(bool* stop_flag);
void print_watchpoint();
WP *get_idle_wp();