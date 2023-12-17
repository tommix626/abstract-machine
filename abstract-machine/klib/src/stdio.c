#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char printf_buf[1000];

int printf(const char *fmt, ...) {
  // WLog("Don't know where is stdout!");
  va_list ap;
  va_start(ap, fmt);
  int cnt = vsprintf(printf_buf,fmt,ap);
  va_end(ap);

  for (const char *p = printf_buf; *p; p++) {
    putch(*p);
  }

  return cnt;
  // return 0;
}

/// @brief fill the dst buffer with digits of int, in backward fashion
/// @param dst destination buffer.
/// @param d integer to be converted.
/// @return the number of digits of the int `d`.
static int int_to_backstr(char* dst,int d) {
  if(d == 0) {
    *dst='0';
    return 1;
  }
  int cnt = 0; 
  bool negflag = (d < 0);
 
  while (d) {
    int dgt = d%10;
    if(dgt<0) dgt = -dgt;
    *dst = (char)(dgt+(int)'0');
    // putch(' '); putch('>'); putch((char)(dgt+(int)'0')); putch('='); putch(*dst); putch('<');
    // assert(*dst<=(int)'9' && *dst>=(int)'0');putch('\n'); 
    d/=10; cnt++;dst++;
  }

  if(negflag) {
    *dst++ = '-';cnt++;
  }
  return cnt;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int d,cnt=0;
  char c;
  char *s;

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
            char int_str[10+5];
            int int_num = int_to_backstr(int_str,d);
            while (int_num-->0)
            {
              *out++ = int_str[int_num];
              // putch('#');putch('>');
              // assert(int_str[int_num]>='0' && int_str[int_num]<='9');
              // putch('|');
              // printf("DEBUG: vsprintf output:%c\n",int_str[int_num]);
              cnt++;
            }
            break;
        case 'c':              /* char */
            /* need a cast here since va_arg only
              takes fully promoted types */
            c = (char) va_arg(ap, int);
            // putch('$');putch('>');
            // assert(c>='0' && c<='9');
            // putch('|');
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
  return cnt;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int cnt = vsprintf(out,fmt,ap);
  va_end(ap);
  return cnt;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  int cnt = vsnprintf(out,n,fmt,ap);
  va_end(ap);
  return cnt;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int d,cnt=0;
  char c;
  char *s;


  while (*fmt && n-->1) {
    if(*fmt=='%') {
      n++; // "%"" is not counted
      switch (*++fmt) {
        case 's':              /* string */
            s = va_arg(ap, char *);
            int len_s = strlen(s);
            if(len_s < n) {
              out = strcpy(out,s) + len_s;cnt+=len_s;n-=len_s;
            }
            else {
              strncpy(out,s,n-1);
              cnt+=n-1;n=1;
            }
            break;
        case 'd':              /* int */
            d = va_arg(ap, int);
            char int_str[10+5];
            int int_num = int_to_backstr(int_str,d);
            while (int_num-->0 && n > 1)
            {
              *out++ = int_str[int_num];
              cnt++;n--;
            }
            break;
        case 'c':              /* char */
            /* need a cast here since va_arg only
              takes fully promoted types */
            c = (char) va_arg(ap, int);
            memcpy(out,&c,1); out++;cnt++;n--;
            break;
      }
      fmt++;
    }
    else {
      *out++ = *fmt++;cnt++;
    }
  }
  *out = '\0';
  return cnt;
}

#endif
