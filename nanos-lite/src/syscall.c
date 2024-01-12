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
    case 0: halt(0);break;//exit
    case 1: c->GPRx = sys_yield();break; //yield
    case 4: c->GPRx = sys_write();break; //write
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  #ifdef CONFIG_STRACE
  Log("syscall return %d",c->GPRx);
  #endif
}

uintptr_t sys_yield(){
  #ifdef CONFIG_STRACE 
  Log("do_syscall dispatch yield!");
  #endif
  yield();
  return 0;
}

uintptr_t sys_write(){
  #ifdef CONFIG_STRACE 
  Log("do_syscall call helper method sys_write!");
  #endif
  /** peusdocode
   * 1. check fd (which arg) is 1/2 -> putch(stdout, stderr[?]) [man 2 write/man syscalls/man syscall]
   * 2. what does it return (man 2 write) On success, the number of bytes written is returned.  On error, -1 is returned,
  */
  return 0;
}