#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}



int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  int result = NDL_PollEvent(buf,sizeof(buf));
  if (result==0) return 0; //key none
  else {
    if(strncmp(buf, "ku", 2) == 0) {
      ev->key.type = SDL_KEYUP;
      // printf("KEYUP\n");
    }
    else if(strncmp(buf, "kd", 2) == 0) {
      ev->key.type = SDL_KEYDOWN;
      // printf("KEYDOWN\n");
    }
    else assert(0);

    for (int k = 0; k  < sizeof(keyname)/sizeof(keyname[0]); k ++)
    {
      if(strncmp(buf+3, keyname[k], sizeof(buf+3))==0) {
        ev->key.keysym.sym = k;
        // printf("SDL poll key sym = %d\n",e.keysym.sym);
        break;
      }
    }
    // printf("FInal CHekc SDL poll key sym = %d\n",ev->key.keysym.sym);
    return 1;
  }
}

int SDL_WaitEvent(SDL_Event *event) {
  // printf("waiting for event:SDL...\n");
  while (SDL_PollEvent(event)==0); //keep polling until SDL_PollEvent return 1.
  // printf("returning from waiting event:SDL...received keysym=%d\n",event->key.keysym.sym);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
