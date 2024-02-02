#include <common.h>
// #include <assert.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

// #define CONFIG_STRACE 1


size_t serial_write(const void *buf, size_t offset, size_t len) {
  #ifdef CONFIG_STRACE
  Log("serial_write:write to STDOUT/STDERR");
  #endif
  for (size_t i = 0; i < len; i++)
    {
      putch(*(char*)buf++);
    }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  #ifdef CONFIG_STRACE
  Log("Events_read!");
  #endif
  AM_INPUT_KEYBRD_T kbd = io_read(AM_INPUT_KEYBRD);
  #ifdef CONFIG_STRACE
  Log("keycode=%#x, %s",kbd.keycode,(kbd.keydown? "kd":"ku"));
  #endif
  if(kbd.keycode == AM_KEY_NONE) {
    #ifdef CONFIG_STRACE
    Log("No Key Input!");
    #endif
    
    return 0;
  }
  return snprintf(buf,len,"%s %s",(kbd.keydown? "kd":"ku"),keyname[kbd.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T gpu_conf = io_read(AM_GPU_CONFIG);
  
  return snprintf(buf,len,"WIDTH:%d\nHEIGHT:%d",gpu_conf.width,gpu_conf.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T gpu_conf = io_read(AM_GPU_CONFIG);
  int px_x = offset % gpu_conf.width;
  int px_y = offset / gpu_conf.width;
  assert(px_y <= gpu_conf.height);
  assert(px_x + len <= gpu_conf.width); //assume only one line of information
  io_write(AM_GPU_FBDRAW, px_x, px_y, (void*) buf, len, 1, true);
  // for (size_t i = 0; i < len; i++)
  // {
  //   //draw px with ioe
    
  
  //   //move to next pix
  //   px_h += (px_w+1)/ gpu_conf.width;
  //   px_w =  (px_w+1)% gpu_conf.width;
  // }
  
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
