/*
 * Copyright © 2005-2014 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

.global longjmp
.type longjmp,   %function
longjmp:
        ld s0,    0(a0)
        ld s1,    8(a0)
        ld s2,    16(a0)
        ld s3,    24(a0)
        ld s4,    32(a0)
        ld s5,    40(a0)
        ld s6,    48(a0)
        ld s7,    56(a0)
        ld s8,    64(a0)
        ld s9,    72(a0)
        ld s10,   80(a0)
        ld s11,   88(a0)
        ld sp,    96(a0)
        ld ra,    104(a0)

        seqz a0, a1
        add a0, a0, a1
        ret

.global setjmp
.type setjmp,   %function
setjmp:
        sd s0,    0(a0)
        sd s1,    8(a0)
        sd s2,    16(a0)
        sd s3,    24(a0)
        sd s4,    32(a0)
        sd s5,    40(a0)
        sd s6,    48(a0)
        sd s7,    56(a0)
        sd s8,    64(a0)
        sd s9,    72(a0)
        sd s10,   80(a0)
        sd s11,   88(a0)
        sd sp,    96(a0)
        sd ra,    104(a0)

        li a0, 0
        ret
