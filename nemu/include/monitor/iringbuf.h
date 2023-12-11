#ifndef __IRINGBUF_H__
#define __IRINGBUF_H__

#include <monitor/sdb.h>

#define MAX_RING_LENGTH 32 // maximum ring size

typedef struct instruction_ringbuf{
  char logringbuf[MAX_RING_LENGTH][128];
  int curr_ptr;
} IRingBuf;



void iringbuf_init();
void iringbuf_add();
void iringbuf_print();

#endif