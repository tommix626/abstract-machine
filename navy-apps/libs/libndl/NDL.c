#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_x = 0, canvas_y = 0; //picture displayed on the left corner
static int full_h=-1,full_w=-1;
//added
static int fd_kbdevtdev = -1;
static int fd_procdispinfo = -1;
uint32_t NDL_GetTicks() {
  
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == 0) {
    uint32_t uptime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return uptime; //unit = us
  }
  printf("gettimeofday return error!");
  return 0;
}


int NDL_PollEvent(char *buf, int len) {
  int res = read(fd_kbdevtdev, buf, len);
  // printf("read kbd result=%d\n",res); //DEBUG
  return res;
}

static void parse_dispinfo(char* buf, int buf_size, int *w, int *h) {

  int i = 0;
  int width = 0, height = 0;

  assert(strncmp(buf + i, "WIDTH", 5) == 0); //start with WIDTH
  i += 5;
  for (; i < buf_size; ++i) { //<whitespace> and :
      if (buf[i] == ':') { i++; break; }
      assert(buf[i] == ' ');
  }
  for (; i < buf_size; ++i) { //following <whitespace>
      if (buf[i] >= '0' && buf[i] <= '9') break;
      assert(buf[i] == ' ');
  }
  for (; i < buf_size; ++i) { //read width base 10
      if (buf[i] >= '0' && buf[i] <= '9') {
          width = width * 10 + buf[i] - '0';
      } else {
          break;
      }
  }
  assert(buf[i++] == '\n'); //must change line

  assert(strncmp(buf + i, "HEIGHT", 6) == 0); //start with HEIGHT
  i += 6;
  for (; i < buf_size; ++i) { //<whitespace> and :
      if (buf[i] == ':') { i++; break; }
      assert(buf[i] == ' ');
  }
  for (; i < buf_size; ++i) { //following <whitespace>
      if (buf[i] >= '0' && buf[i] <= '9') break;
      assert(buf[i] == ' ');
  }
  for (; i < buf_size; ++i) { //read height base 10
      if (buf[i] >= '0' && buf[i] <= '9') {
          height = height * 10 + buf[i] - '0';
      } else {
          break;
      }
  }
  
  *w = width; *h = height;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];

    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }

  

  if(*w == 0 && *h == 0) {//full screen
    *w = full_w; *h = full_h;
  }
  else {
    assert(*w<=full_w && *h<=full_h);
  }
  screen_w = *w; screen_h = *h;
  
  printf("w=%d h=%d\n",screen_w,screen_h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) { //xywh on canvas
  assert(w<=screen_w && h<=screen_h);

  for (size_t line = 0; line < h; line++)
  {
    //draw line by invoke write on file dev/fb and setting open_offset
    int real_x = x + canvas_x, real_y =  y + canvas_y + line;
    int offset = real_x + real_y * full_w;
    lseek(fbdev,offset,SEEK_SET);
    write(fbdev,pixels+w*line,w);
  }
  
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}


int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  fd_kbdevtdev = open("/dev/events","r+");
  fd_procdispinfo = open("/proc/dispinfo","r+");
  fbdev = open("/dev/fb","w");

  //screen size
  char scrinfo[105];
  int size = read(fd_procdispinfo,scrinfo,100);
  parse_dispinfo(scrinfo,size,&full_w,&full_h);
  return 0;
}

void NDL_Quit() {
}
