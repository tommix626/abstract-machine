#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"
#include <assert.h>

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
    printf("IMG_Load, path=%s\n",filename);
    FILE * fp = fopen(filename, "r");
    if (!fp) return NULL;
    else printf("FP load successfully!\n");

    fseek(fp, 0L, SEEK_END);
    printf("fseek\n");
    long size = ftell(fp);
    printf("size!\n");
    rewind(fp);
    printf("rewind\n");
    unsigned char * buf = (unsigned char *)SDL_malloc(size * sizeof(unsigned char));
    
    printf("malloc\n");
    // assert(fread(buf, 1, size, fp) == size);

    SDL_Surface * surface = STBIMG_LoadFromMemory(buf, size);
    
    printf("get surface\n");
    assert(surface != NULL);

    fclose(fp);
    
    printf("fclose\n");
    free(buf);
    
    printf("free buf\n");
    return surface;
}

// SDL_Surface* IMG_Load(const char *filename) {
//   printf("IMG_Load, path=%s\n",filename);
//   int fp = open(filename,"r+");
//   // FILE *img = fopen(filename,"r+");
//
//   assert(fp);
//   lseek(fp, 0, SEEK_END);
//   size_t size = tell(fp);
//   printf("file Size=%d\n",size);
//   lseek(fp, 0, SEEK_SET);
//   void* mem = SDL_malloc(size);
//   read(fp,mem,sizeof(mem));
//   SDL_Surface* result = STBIMG_LoadFromMemory((const unsigned char*) mem,size);
//   close(fp);
//   free(mem);
//   return result;
// }

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
