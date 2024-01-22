#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <NDL.h>

#define TEST_NDL
#ifndef TEST_NDL
int main() {
  struct timeval tv,tv_start;
  assert(gettimeofday(&tv_start, NULL) == 0); //fill in start time

  size_t times = 1;

  while (1) {
      assert(gettimeofday(&tv, NULL) == 0);
      time_t curr_sec = tv.tv_sec;
      suseconds_t curr_usec = tv.tv_usec;
      uint32_t delta = (curr_sec - tv_start.tv_sec) * 1000000 + (curr_usec - tv_start.tv_usec);
      if (delta > 500000 * times) {
          printf("curr half time =  %u\n", times);
          times++;
      }
  }

  return 0;
}
#else
int main() {
  NDL_Init(0);
  uint32_t start_time = NDL_GetTicks();
  size_t times = 1;

  while (1) {
      uint32_t uptime = NDL_GetTicks()-start_time;
      if (uptime > 500 * times) {
          printf("curr half time =  %u\n", times);
          times++;
      }
  }

  return 0;
}
#endif