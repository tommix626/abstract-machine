#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char printf_buf[1000];

int printf(const char *fmt, ...) {
  // while(1){}
  putch('@');
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

static int unsigned_int_to_backstr(char* dst, unsigned int d) {
    if (d == 0) {
        *dst = '0';
        return 1;
    }

    int cnt = 0;
    while (d) {
        int dgt = d % 10;
        *dst = (char)(dgt + (int)'0');
        d /= 10;
        cnt++;
        dst++;
    }

    return cnt;
}

static int uint_to_hexstr(char* dst, unsigned int d) {
    static const char hexdigits[] = "0123456789abcdef";
    int cnt = 0;

    if (d == 0) {
        *dst++ = '0';
        cnt++;
    } else {
        // Process each digit in reverse order
        while (d) {
            unsigned int digit = d % 16;
            *dst++ = hexdigits[digit];
            d /= 16;
            cnt++;
        }
    }

    // The string is in reverse order, so it will be reversed again when copied
    return cnt;
}

static int ptr_to_hexstr(char* dst, void* ptr) {
    static const char hexdigits[] = "0123456789abcdef";
    unsigned long addr = (unsigned long)ptr;
    int cnt = 0;

    do {
        unsigned int digit = addr % 16;
        *dst++ = hexdigits[digit];
        addr /= 16;
        cnt++;
    } while (addr > 0);

    return cnt;
}

static int uint16_to_str(char* dst, uint16_t d) {
    if (d == 0) {
        *dst = '0';
        return 1;
    }

    int cnt = 0;
    while (d) {
        int dgt = d % 10;
        *dst = (char)(dgt + '0');
        d /= 10;
        cnt++;
        dst++;
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
            char int_str[20+5];
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
        
        case 'u':              /* unsigned int */
            unsigned int u = va_arg(ap, unsigned int);
            char u_str[20+5];
            int u_num = unsigned_int_to_backstr(u_str, u); 
            while (u_num-->0) {
                *out++ = u_str[u_num];
                cnt++;
            }
            break;
        case '#':
            if (*++fmt == 'x') {
                unsigned int x = va_arg(ap, unsigned int);
                char hex_str[10]; // Enough to hold any 32-bit unsigned int in hex
                int hex_len = uint_to_hexstr(hex_str, x);

                // Prepend "0x"
                *out++ = '0'; *out++ = 'x';
                cnt += 2;

                // REVERSE output
                while (hex_len-- > 0) {
                    *out++ = hex_str[hex_len];
                    cnt++;
                }
            }
            break;
        case 'p': 
            void* p = va_arg(ap, void*);
            char ptr_str[20]; // Enough for a pointer's hexadecimal representation
            int ptr_len = ptr_to_hexstr(ptr_str, p);

            while (ptr_len-- > 0) {
                *out++ = ptr_str[ptr_len];
                cnt++;
            }
            break;
        case 'h':
            if (*++fmt == 'u') { /* uint16_t */
                uint16_t hu = (uint16_t)va_arg(ap, unsigned int); // Promoted to unsigned int
                char hu_str[6]; // Enough for any 16-bit value
                int hu_len = uint16_to_str(hu_str, hu);

                // Copy the uint16_t string in reverse order (since it was converted backward)
                while (hu_len-- > 0) {
                    *out++ = hu_str[hu_len];
                    cnt++;
                }
            }
            break;


      }
      fmt++;
    }
    else {
      *out++ = *fmt++;cnt++;
    }
  }
  *out = '\0';
  putch('*');
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
