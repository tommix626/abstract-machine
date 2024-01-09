#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

//excluding '\0'
size_t strlen(const char *s) {
  size_t len = 0; 
  while(*s!='\0'){
    len++;s++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *dptr = dst;
  *dptr = *src;
  do {
    src++; dptr++;
    *dptr = *src;
  } while(*src!='\0');
  return dst;
}

//might not end with '\0'!
char *strncpy(char *dst, const char *src, size_t n) {
  char *dptr = dst;
  while(n-- > 0) {
    if(*src!='\0') {
      *dptr = *src;
      src++; dptr++;
    }
    else {
      *dptr = '\0'; dptr++;
    }
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *dptr = dst;
  while(*dst != '\0') dst++; //move to null terminator
  strcpy(dst,src);
  return dptr;
}

int strcmp(const char *s1, const char *s2) {
  int cmp = 0;
  while(cmp == 0 && !(*s1 == '\0' && *s2 == '\0')) {
    cmp = *s1 - *s2;
    s1++; s2++;
  }
  return cmp;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int cmp = 0;
  while(n-- >0 && cmp == 0 && !(*s1 == '\0' && *s2 == '\0')) {
    cmp = *s1 - *s2;
    s1++; s2++;
  }
  return cmp;
}

/// @brief set a part of mem (s,s+n) with c.
/// @param s mem start
/// @param c the content
/// @param n size
/// @return 
void *memset(void *s, int c, size_t n) {
  void* sptr = s;
  while(n-->0){
    *(char*)s = (char) c;
    // *(char*)s |= c << 12;
    s++;
  }
  return sptr;
}

void *memmove(void *dst, const void *src, size_t n) {
  //copy the src to heap, stored temporately. Copying it back to dst and free mem on heap. BUT here, just use extra space on the outer computer.
  char temp[n+4];
  memcpy(temp,src,n);
  return memcpy(dst,temp,n);
}

void *memcpy(void *out, const void *in, size_t n) {
  char *dptr = out;
  while(n-- > 0) {
    *dptr = *(char*)in;
    in++; dptr++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  int cmp = 0;
  while(n-- >0 && cmp == 0) {
    cmp = *(unsigned char*)s1 - *(unsigned char*)s2;
    s1++; s2++;
  }
  return cmp;
}

#endif
