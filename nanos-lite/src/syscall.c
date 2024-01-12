#include <common.h>
#include "syscall.h"

#define CONFIG_STRACE 1

enum { //from navy-apps/libs/libos/src/syscall.h
  SYS_exit = 0,
  SYS_yield,
  SYS_open,
  SYS_read,
  SYS_write,
  SYS_kill,
  SYS_getpid,
  SYS_close,
  SYS_lseek,
  SYS_brk,
  SYS_fstat,
  SYS_time,
  SYS_signal,
  SYS_execve,
  SYS_fork,
  SYS_link,
  SYS_unlink,
  SYS_wait,
  SYS_times,
  SYS_gettimeofday
};


void do_syscall(Context *c) {
  #ifdef CONFIG_STRACE
  Log("STrace:syscall type=%d, arg= %d, %d, %d.",c->GPR1,c->GPR2,c->GPR3,c->GPR4);
  #endif

  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_exit: halt(0);break;//exit
    case SYS_yield: c->GPRx = sys_yield();break; //yield
    case SYS_write: c->GPRx = sys_write(a[1],a[2],a[3]);break; //write
    case SYS_brk: c->GPRx = sys_brk(a[1]);break; //set new brk
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

uintptr_t sys_write(int fd, void *buf, size_t count){
  #ifdef CONFIG_STRACE 
  Log("do_syscall call helper method sys_write!");
  #endif
  if(fd==1 || fd==2){
    #ifdef CONFIG_STRACE 
    Log("sys_write:write to STDOUT/STDERR");
    #endif
    
    for (size_t i = 0; i < count; i++)
    {
      putch(*(char*)buf++);
    }
    return count;
  }
  /** peusdocode
   * 1. check fd (which arg) is 1/2 -> putch(stdout, stderr[?]) [man 2 write/man syscalls/man syscall]
   * 2. what does it return (man 2 write) On success, the number of bytes written is returned.  On error, -1 is returned,
  */
  Log("Other write stream unsupported");
  return -1;
}


static uintptr_t *nanos_brk;
uintptr_t sys_brk(uintptr_t addr){
  #ifdef CONFIG_STRACE 
  Log("do_syscall update brk!");
  #endif
  *nanos_brk = addr;
  return 0;
}