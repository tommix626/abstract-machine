#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

/// @brief fill the dst buffer with digits of int, in backward fashion
/// @param dst destination buffer.
/// @param d integer to be converted.
/// @return the number of digits of the int `d`.
static int int_to_backstr(char* dst,int d) {
  int cnt=0;
  while (d) {
    *dst++ = (char)(d%10+(int)'0'); 
    d/=10; cnt++;
  }
  return cnt;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int d,cnt=0;
  char c;
  char *s;

  va_start(ap, fmt);
  while (*fmt) {
    if(*fmt=='%') {
      switch (*++fmt) {
        case 's':              /* string */
            s = va_arg(ap, char *);
            int len_s = strlen(s);
            out = strcpy(out,s) + len_s;cnt+=len_s;
            break;
        case 'd':              /* int */
            d = va_arg(ap, int);
            char int_str[10+2];
            int int_num = int_to_backstr(int_str,d);
            while (int_num-->0)
            {
              *out++ = int_str[int_num];
              cnt++;
            }
            break;
        case 'c':              /* char */
            /* need a cast here since va_arg only
              takes fully promoted types */
            c = (char) va_arg(ap, int);
            memcpy(out,&c,1); out++;cnt++;
            break;
      }
      fmt++;
    }
    else {
      *out++ = *fmt++;cnt++;
    }
  }
  *out = '\0';
  va_end(ap);
  return cnt;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
