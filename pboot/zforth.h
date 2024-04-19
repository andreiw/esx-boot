/*******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 * Copyright 2000 Ico Doornekamp <zforth@zevv.nl>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Matches commit f997c3b + untabify/indent.
 */

#ifndef zforth_h
#define zforth_h

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#include "zfconf.h"

/* Abort reasons */

typedef enum {
   ZF_OK,
   ZF_ABORT_INTERNAL_ERROR,
   ZF_ABORT_OUTSIDE_MEM,
   ZF_ABORT_DSTACK_UNDERRUN,
   ZF_ABORT_DSTACK_OVERRUN,
   ZF_ABORT_RSTACK_UNDERRUN,
   ZF_ABORT_RSTACK_OVERRUN,
   ZF_ABORT_NOT_A_WORD,
   ZF_ABORT_COMPILE_ONLY_WORD,
   ZF_ABORT_INVALID_SIZE,
   ZF_ABORT_DIVISION_BY_ZERO,
   ZF_ABORT_INVALID_USERVAR,
   ZF_ABORT_EXTERNAL
} zf_result;

typedef enum {
   ZF_INPUT_INTERPRET,
   ZF_INPUT_PASS_CHAR,
   ZF_INPUT_PASS_WORD
} zf_input_state;

typedef enum {
   ZF_SYSCALL_EMIT,
   ZF_SYSCALL_PRINT,
   ZF_SYSCALL_TELL,
   ZF_SYSCALL_USER = 128
} zf_syscall_id;

typedef enum {
   ZF_USERVAR_HERE = 0,
   ZF_USERVAR_LATEST,
   ZF_USERVAR_TRACE,
   ZF_USERVAR_COMPILING,
   ZF_USERVAR_POSTPONE,
   ZF_USERVAR_DSP,
   ZF_USERVAR_RSP,

   ZF_USERVAR_COUNT
} zf_uservar_id;


/* ZForth API functions */

void zf_init(int trace);
void zf_bootstrap(void);
void *zf_dump(size_t *len);
zf_result zf_eval(const char *buf);
void zf_abort(zf_result reason);

void zf_push(zf_cell v);
zf_cell zf_pop(void);
zf_cell zf_pick(zf_addr n);

zf_result zf_uservar_set(zf_uservar_id uv, zf_cell v);
zf_result zf_uservar_get(zf_uservar_id uv, zf_cell *v);

/* Host provides these functions */

zf_input_state zf_host_sys(zf_syscall_id id, const char *last_word);
void zf_host_trace(const char *fmt, va_list va);
zf_cell zf_host_parse_num(const char *buf);

#endif
