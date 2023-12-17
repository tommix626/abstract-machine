#include <am.h>
#include <nemu.h>
// NOTE: functions here are called in abstract-machine/am/src/platform/nemu/ioe/ioe.c

void __am_timer_init() {
  // outl(RTC_ADDR,0);
  // outl(RTC_ADDR+4,0);
}

//this is invoked when ioe_read/ioe_write is called, which should: //invoke callback and then callback read. TODO
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = inl(RTC_ADDR+4); //this access also update time.
  uptime->us <<= 32;
  uptime->us = inl(RTC_ADDR);
  // mmio_write(RTC_ADDR+4,4,NOTIFY4); //data is irrlevant, just notifying io to update time
  // word_t low_time = mmio_read(RTC_ADDR,4); //can I just write 8 here, even though my current ISA is 32-bit system? NO: return value is word_t=utin32_t
  // word_t high_time = mmio_read(RTC_ADDR+4,4);
  //IMPORTANT NOTE: Why we 1. cannot use mmio.h, and 2. we can deref directly the physical addr of the guest.
  // how the code in AM_HOME relates to code in NEMU. we make AM into image and load the image to NEMU. That's why the program here are really running on nemu, deref means dereferencing on nemu. 
  // all the addr here are paddr on nemu, and host machine is invisible to us(AM env) due to abstraction in nemu!
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
