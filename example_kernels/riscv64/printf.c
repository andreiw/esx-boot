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
#include <stdbool.h>

#ifdef TEST
#include <stdio.h>
#define simple_putchar putchar
#else
#include "kernel.h"
#define simple_printf printf
#define simple_sprintf sprintf
#define simple_snprintf snprintf
#define simple_vsnprintf vsnprintf
#define simple_putchar AABI_CALL(sbi_putchar, 1, 0)
#endif

static bool
simple_outputchar(char **str, size_t len,
                  int *offset, char c)
{
   if (*offset >= len) {
      return false;
   }

   if (str) {
      **str = c;
      ++(*str);
   } else {
      simple_putchar(c);
   }

   *offset = *offset + 1;
   return true;
}

enum flags {
   PAD_ZERO = 1,
   PAD_RIGHT = 2,
   PRE_OX = 4,
};

static bool
prints(char **out, size_t len, int *offset,
       const char *string, int width, int flags)
{
   int padchar = ' ';

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
         if (!simple_outputchar(out, len, offset, padchar)) {
            return false;
         }
      }
   }
   for ( ; *string ; ++string) {
      if (!simple_outputchar(out, len, offset, *string)) {
         return false;
      }
   }
   for ( ; width > 0; --width) {
      if (!simple_outputchar(out, len, offset, padchar)) {
         return false;
      }
   }

   return true;
}

#define PRINT_BUF_LEN 64

static bool
simple_outputi(char **out, size_t len,
               int *offset, intmax_t i,
               int base, bool sign,
               int width, int flags,
               int letbase)
{
   char print_buf[PRINT_BUF_LEN];
   char *s;
   int t, neg = 0;
   uintmax_t u = i;

   if (i == 0) {
      print_buf[0] = '0';
      print_buf[1] = '\0';
      return prints(out, len, offset, print_buf, width, flags);
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
         if (!simple_outputchar (out, len, offset, '0')) {
            return false;
         }
         if (!simple_outputchar (out, len, offset, 'x')) {
            return false;
         }
         width -= 2;
      } else {
         *--s = 'x';
         *--s = '0';
      }
   }

   if (neg) {
      if( width && (flags & PAD_ZERO) ) {
         if (!simple_outputchar (out, len, offset, '-')) {
            return false;
         }
         --width;
      }
      else {
         *--s = '-';
      }
   }

   return prints (out, len, offset, s, width, flags);
}

static int
simple_vsnprintf(char **out, size_t len,
                 const char *format, va_list ap)
{
   int width, flags;
   int pc = 0;
   char scr[2];
   bool cont;
   uintmax_t u;

   for (cont = true; cont && *format != 0; ++format) {
      if (*format == '%') {
         bool do_num = false;
         bool num_sign = false;
         int num_base = 10;
         bool num_caps = false;

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
            u = va_arg(ap, int);
            num_sign = true;
            do_num = true;
            break;

         case('u'):
            u = va_arg(ap, unsigned int);
            do_num = true;
            break;

         case('X'):
            num_caps = true;
         case('x'):
            u = va_arg(ap, unsigned int);
            num_base = 16;
            do_num = true;
            break;

         case('c'):
            u = va_arg(ap, int);
            scr[0] = u;
            scr[1] = '\0';
            cont = prints(out, len, &pc, scr, width, flags);
            break;

         case('s'):
            u = (uintmax_t) va_arg(ap, char *);
            cont = prints(out, len, &pc, u ? (char *) u : "(null)",
                          width, flags);
            break;

         case('p'):
            u = va_arg(ap, uintptr_t);
            num_base = 16;
            do_num = true;
            flags |= PRE_OX;
            break;

         case('l'):
            ++format;
            switch (*format) {
            case('d'):
               u = va_arg(ap, long);
               num_sign = true;
               do_num = true;
               break;

            case('u'):
               u = va_arg(ap, unsigned long);
               do_num = true;
               break;

            case('X'):
               num_caps = true;
            case('x'):
               u = va_arg(ap, unsigned long);
               num_base = 16;
               do_num = true;
               break;

            case('l'):
               ++format;
               switch (*format) {
               case('d'):
                  u = va_arg(ap, long long);
                  num_sign = true;
                  do_num = true;
                  break;

               case('u'):
                  u = va_arg(ap, unsigned long long);
                  do_num = true;
                  break;

               case('X'):
                  num_caps = true;
               case('x'):
                  u = va_arg(ap, unsigned long long);
                  num_base = 16;
                  do_num = true;
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
               u = va_arg(ap, int);
               num_sign = true;
               do_num = true;
               break;

            case('u'):
               u = va_arg(ap, unsigned int);
               do_num = true;
               break;

            case('X'):
               num_caps = true;
            case('x'):
               u = va_arg(ap, unsigned int);
               num_base = 16;
               do_num = true;
               break;

            case('h'):
               ++format;
               switch (*format) {
               case('d'):
                  u = va_arg(ap, int);
                  num_sign = true;
                  do_num = true;
                  break;

               case('u'):
                  u = va_arg(ap, unsigned int);
                  do_num = true;
                  break;

               case('X'):
                  num_caps = true;
               case('x'):
                  u = va_arg(ap, unsigned int);
                  num_base = 16;
                  do_num = true;
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

         if (do_num) {
            cont = simple_outputi(out, len, &pc, u, num_base, num_sign,
                                  width, flags, num_caps ? 'A' : 'a');
         }
      } else {
      out:
         cont = simple_outputchar (out, len, &pc, *format);
      }
   }
   if (out && pc < len) {
      **out = '\0';
   }
   return pc;
}

__attribute__ ((format (printf, 1, 2)))
int
simple_printf(const char *fmt, ...)
{
   va_list ap;
   int r;

   va_start(ap, fmt);
   r = simple_vsnprintf(NULL, -1, fmt, ap);
   va_end(ap);

   return r;
}

__attribute__ ((format (printf, 2, 3)))
int
simple_sprintf(char *buf, const char *fmt, ...)
{
   va_list ap;
   int r;

   va_start(ap, fmt);
   r = simple_vsnprintf(&buf, -1, fmt, ap);
   va_end(ap);

   return r;
}

__attribute__ ((format (printf, 3, 4)))
int
simple_snprintf(char *buf, size_t size, const char *fmt, ...)
{
   va_list ap;
   int r;

   va_start(ap, fmt);
   r = simple_vsnprintf(&buf, size, fmt, ap);
   va_end(ap);

   return r;
}

#ifdef TEST

#define printf simple_printf
#define sprintf simple_sprintf
#define snprintf simple_snprintf
#define simple_vsnprintf vsnprintf

int
main(int argc, char *argv[])
{
   int ret;
   int line;
   static char shortstr[] = "Test";
   char buf[256];

#define T(x, ...) ret = printf(x, ## __VA_ARGS__); printf("+L%u: %d chars\n", __LINE__ - line, ret);
#define TS(b, x, ...) ret = sprintf(b, x, ## __VA_ARGS__); printf("+L%u: %d chars\n", __LINE__ - line,ret);
#define TSN(b, l, x, ...) ret = snprintf(b, l, x, ## __VA_ARGS__); printf("+L%u: %d chars\n", __LINE__ - line,ret);
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
   printf("sprintf: %s", buf);
   buf[5] = '?';
   TSN(buf, 5, "%d", -2345);
   if (buf[5] != '?') {
      fprintf(stderr, "L%u: corrupted buf\n", __LINE__);
   }
   buf[5] = '\0';
   printf("snprintf: '%s'\n", buf);
   buf[5] = '?';
   TSN(buf, 6, "%d", -2345);
   if (buf[5] != '\0') {
      fprintf(stderr, "L%u: corrupted buf\n", __LINE__);
   }

#undef T
#undef TS
#undef TSN
}
#endif /* TEST */
