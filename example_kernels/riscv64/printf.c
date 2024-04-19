/*
 * Copyright (c) 2016, Matt Redfearn
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>

#ifdef TEST
#include <stdio.h>
#define simple_putchar putchar
#else
#include "kernel.h"
#define simple_printf printf
#define simple_sprintf sprintf
#define simple_putchar AABI_CALL(sbi_putchar, 1, 0)
#endif

static void simple_outputchar(char **str, char c)
{
   if (str) {
      **str = c;
      ++(*str);
   } else {
      simple_putchar(c);
   }
}

enum flags {
   PAD_ZERO = 1,
   PAD_RIGHT = 2,
   PRE_OX = 4,
};

static int prints(char **out, const char *string, int width, int flags)
{
   int pc = 0, padchar = ' ';

   if (width > 0) {
      int len = 0;
      const char *ptr;
      for (ptr = string; *ptr; ++ptr) ++len;
      if (len >= width) width = 0;
      else width -= len;
      if (flags & PAD_ZERO)
         padchar = '0';
   }
   if (!(flags & PAD_RIGHT)) {
      for ( ; width > 0; --width) {
         simple_outputchar(out, padchar);
         ++pc;
      }
   }
   for ( ; *string ; ++string) {
      simple_outputchar(out, *string);
      ++pc;
   }
   for ( ; width > 0; --width) {
      simple_outputchar(out, padchar);
      ++pc;
   }

   return pc;
}

#define PRINT_BUF_LEN 64

static int simple_outputi(char **out, long long i, int base, int sign, int width, int flags, int letbase)
{
   char print_buf[PRINT_BUF_LEN];
   char *s;
   int t, neg = 0, pc = 0;
   unsigned long long u = i;

   if (i == 0) {
      print_buf[0] = '0';
      print_buf[1] = '\0';
      return prints(out, print_buf, width, flags);
   }

   if (sign && base == 10 && i < 0) {
      neg = 1;
      u = -i;
   }

   s = print_buf + PRINT_BUF_LEN-1;
   *s = '\0';

   while (u) {
      t = u % base;
      if( t >= 10 )
         t += letbase - '0' - 10;
      *--s = t + '0';
      u /= base;
   }

   if ((flags & PRE_OX) != 0) {
      if (width != 0) {
         simple_outputchar (out, '0');
         simple_outputchar (out, 'x');
         pc += 2;
         width -= 2;
      } else {
         *--s = 'x';
         *--s = '0';
      }
   }

   if (neg) {
      if( width && (flags & PAD_ZERO) ) {
         simple_outputchar (out, '-');
         ++pc;
         --width;
      }
      else {
         *--s = '-';
      }
   }

   return pc + prints (out, s, width, flags);
}


static int simple_vsprintf(char **out, char *format, va_list ap)
{
   int width, flags;
   int pc = 0;
   char scr[2];
   union {
      char c;
      char *s;
      int i;
      unsigned int u;
      long li;
      unsigned long lu;
      long long lli;
      unsigned long long llu;
      short hi;
      unsigned short hu;
      signed char hhi;
      unsigned char hhu;
      uintptr_t ptr;
   } u;

   for (; *format != 0; ++format) {
      if (*format == '%') {
         ++format;
         width = flags = 0;
         if (*format == '\0')
            break;
         if (*format == '%')
            goto out;
         if (*format == '-') {
            ++format;
            flags = PAD_RIGHT;
         }
         while (*format == '0') {
            ++format;
            flags |= PAD_ZERO;
         }
         if (*format == '*') {
            width = va_arg(ap, int);
            format++;
         } else {
            for ( ; *format >= '0' && *format <= '9'; ++format) {
               width *= 10;
               width += *format - '0';
            }
         }
         switch (*format) {
         case('d'):
            u.i = va_arg(ap, int);
            pc += simple_outputi(out, u.i, 10, 1, width, flags, 'a');
            break;

         case('u'):
            u.u = va_arg(ap, unsigned int);
            pc += simple_outputi(out, u.u, 10, 0, width, flags, 'a');
            break;

         case('x'):
            u.u = va_arg(ap, unsigned int);
            pc += simple_outputi(out, u.u, 16, 0, width, flags, 'a');
            break;

         case('X'):
            u.u = va_arg(ap, unsigned int);
            pc += simple_outputi(out, u.u, 16, 0, width, flags, 'A');
            break;

         case('c'):
            u.c = va_arg(ap, int);
            scr[0] = u.c;
            scr[1] = '\0';
            pc += prints(out, scr, width, flags);
            break;

         case('s'):
            u.s = va_arg(ap, char *);
            pc += prints(out, u.s ? u.s : "(null)", width, flags);
            break;

         case('p'):
            u.ptr = va_arg(ap, uintptr_t);
            pc += simple_outputi(out, u.ptr, 16, 0, width,
                                 flags | PRE_OX, 'a');
            break;

         case('l'):
            ++format;
            switch (*format) {
            case('d'):
               u.li = va_arg(ap, long);
               pc += simple_outputi(out, u.li, 10, 1, width, flags, 'a');
               break;

            case('u'):
               u.lu = va_arg(ap, unsigned long);
               pc += simple_outputi(out, u.lu, 10, 0, width, flags, 'a');
               break;

            case('x'):
               u.lu = va_arg(ap, unsigned long);
               pc += simple_outputi(out, u.lu, 16, 0, width, flags, 'a');
               break;

            case('X'):
               u.lu = va_arg(ap, unsigned long);
               pc += simple_outputi(out, u.lu, 16, 0, width, flags, 'A');
               break;

            case('l'):
               ++format;
               switch (*format) {
               case('d'):
                  u.lli = va_arg(ap, long long);
                  pc += simple_outputi(out, u.lli, 10, 1, width, flags, 'a');
                  break;

               case('u'):
                  u.llu = va_arg(ap, unsigned long long);
                  pc += simple_outputi(out, u.llu, 10, 0, width, flags, 'a');
                  break;

               case('x'):
                  u.llu = va_arg(ap, unsigned long long);
                  pc += simple_outputi(out, u.llu, 16, 0, width, flags, 'a');
                  break;

               case('X'):
                  u.llu = va_arg(ap, unsigned long long);
                  pc += simple_outputi(out, u.llu, 16, 0, width, flags, 'A');
                  break;

               default:
                  break;
               }
               break;
            default:
               break;
            }
            break;
         case('h'):
            ++format;
            switch (*format) {
            case('d'):
               u.hi = va_arg(ap, int);
               pc += simple_outputi(out, u.hi, 10, 1, width, flags, 'a');
               break;

            case('u'):
               u.hu = va_arg(ap, unsigned int);
               pc += simple_outputi(out, u.lli, 10, 0, width, flags, 'a');
               break;

            case('x'):
               u.hu = va_arg(ap, unsigned int);
               pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'a');
               break;

            case('X'):
               u.hu = va_arg(ap, unsigned int);
               pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'A');
               break;

            case('h'):
               ++format;
               switch (*format) {
               case('d'):
                  u.hhi = va_arg(ap, int);
                  pc += simple_outputi(out, u.hhi, 10, 1, width, flags, 'a');
                  break;

               case('u'):
                  u.hhu = va_arg(ap, unsigned int);
                  pc += simple_outputi(out, u.lli, 10, 0, width, flags, 'a');
                  break;

               case('x'):
                  u.hhu = va_arg(ap, unsigned int);
                  pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'a');
                  break;

               case('X'):
                  u.hhu = va_arg(ap, unsigned int);
                  pc += simple_outputi(out, u.lli, 16, 0, width, flags, 'A');
                  break;

               default:
                  break;
               }
               break;
            default:
               break;
            }
            break;
         default:
            break;
         }
      }
      else {
      out:
         simple_outputchar (out, *format);
         ++pc;
      }
   }
   if (out) **out = '\0';
   return pc;
}

__attribute__ ((format (printf, 1, 2)))
int simple_printf(char *fmt, ...)
{
   va_list ap;
   int r;

   va_start(ap, fmt);
   r = simple_vsprintf(NULL, fmt, ap);
   va_end(ap);

   return r;
}

__attribute__ ((format (printf, 2, 3)))
int simple_sprintf(char *buf, char *fmt, ...)
{
   va_list ap;
   int r;

   va_start(ap, fmt);
   r = simple_vsprintf(&buf, fmt, ap);
   va_end(ap);

   return r;
}

#ifdef TEST

#define printf simple_printf
#define sprintf simple_sprintf

int main(int argc, char *argv[])
{
   int ret;
   int line;
   static char shortstr[] = "Test";
   char buf[256];

#define T(x, ...) ret = printf(x, ## __VA_ARGS__); printf("+L%u: %d chars\n", __LINE__ - line, ret);
#define TS(b, x, ...) ret = sprintf(b, x, ## __VA_ARGS__); printf("+L%u: %d chars\n", __LINE__ - line,ret);
   line = __LINE__;
   T("percent:                \"%%\"\n");
   T("bad format:             \"%z\"\n");
   T("decimal:                \"%d\"\n", 12345);
   T("decimal negative:       \"%d\"\n", -2345);
   T("unsigned:               \"%u\"\n", 12345);
   T("unsigned negative:      \"%u\"\n", -2345);
   T("hex:                    \"%x\"\n", 0x12345);
   T("hex negative:           \"%x\"\n", -0x12345);
   T("long decimal:           \"%ld\"\n", 123456L);
   T("long decimal negative:  \"%ld\"\n", -23456L);
   T("long unsigned:          \"%lu\"\n", 123456L);
   T("long unsigned negative: \"%lu\"\n", -123456L);
   T("long hex:               \"%lx\"\n", 0x12345L);
   T("long hex negative:      \"%lx\"\n", -0x12345L);
   T("long long decimal:           \"%lld\"\n", 123456LL);
   T("long long decimal negative:  \"%lld\"\n", -23456LL);
   T("long long unsigned:          \"%llu\"\n", 123456LL);
   T("long long unsigned negative: \"%llu\"\n", -123456LL);
   T("long long hex:               \"%llx\"\n", 0x12345LL);
   T("long long hex negative:      \"%llx\"\n", -0x12345LL);
   T("zero-padded LD:         \"%010ld\"\n", (long) 123456);
   T("zero-padded LDN:        \"%010ld\"\n", (long) -123456);
   T("left-adjusted ZLDN:     \"%-010ld\"\n", (long) -123456);
   T("space-padded LDN:       \"%10ld\"\n", (long) -123456);
   T("left-adjusted SLDN:     \"%-10ld\"\n", (long) -123456);
   T("variable pad width:     \"%0*d\"\n", 15, -2345);
   T("char:                   \"%c%c%c%c\"\n", 'T', 'e', 's', 't');
   T("zero-padded string:     \"%010s\"\n", shortstr);
   T("left-adjusted Z string: \"%-010s\"\n", shortstr);
   T("space-padded string:    \"%10s\"\n", shortstr);
   T("left-adjusted S string: \"%-10s\"\n", shortstr);
   T("null string:            \"%s\"\n", (char *)NULL);
   T("pointer:                \"%p\"\n", (void *) 0x1234);

   TS(buf, "decimal:\t\"%d\"\n", -2345);
   T("sprintf: %s", buf);

#undef T
#undef TS
}
#endif /* TEST */
