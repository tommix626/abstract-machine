#include <common.h>
#include "syscall.h"

#define CONFIG_STRACE 1

void do_syscall(Context *c) {
  #ifdef CONFIG_STRACE
  Log("STrace:syscall type=%d, arg= %d, %d, %d.",c->GPR1,c->GPR2,c->GPR3,c->GPR4);
  #endif

  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case 1: c->GPRx = sys_yield();break; //yield
    case 0: halt(0);break;//exit
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  #ifdef CONFIG_STRACE
  Log("syscall return %d",c->GPRx);
  #endif
}

uintptr_t sys_yield(){
  Log("do_syscall dispatch yield!");
  yield();
  return 0;
}