#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

#ifdef CONFIG_VGA_SIZE_800x600
#define SCREEN_W 800
#define SCREEN_H 600
#else
#define SCREEN_W 400
#define SCREEN_H 300
#endif

void __am_gpu_init() { 
  //just a init test. can leave blank
  // int i;
  // int w = SCREEN_W;
  // int h = SCREEN_H; 
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i; //initialize the screen.
  //debug: draw a red rect
  // for (int i = 30; i < 50; i ++) {
  //   for (int j = 100; j < 200; j ++) {
  //     putch(i%10+'0');putch(j%10+'0');putch('\t');
  //     fb[SCREEN_W*(i)+j] = 0x00ff0000;
  //   }
  // }

  // outl(SYNC_ADDR, 1); //update screen. by triggering the io_handler callback in vga.c, which then write the memory with the newest pixel.
  // putch('@'); //DEBUG
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = SCREEN_W, .height = SCREEN_H,
    .vmemsz = SCREEN_W * SCREEN_H
  };
}

//test program call io_read/write will call this ultimately.   AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  
  //TODO:check xywh vaild.

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int cnt = 0;
  for (int i = ctl->y; i < ctl->y+ctl->h; i ++) {
    for (int j = ctl->x; j < ctl->x+ctl->w; j ++) {
      // putch(i%10+'0');putch(j%10+'0');putch('\t');
      fb[SCREEN_W*(i)+j] = ((uint32_t *) ctl->pixels)[cnt++];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1); //write sync register to 1, this call make vga sync/updates
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
