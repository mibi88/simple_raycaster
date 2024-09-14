/* A quick and dirty raycaster.
 * by Mibi88
 *
 * This software is licensed under the BSD-3-Clause license:
 *
 * Copyright 2024 Mibi88
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

#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

/* The fixed point type (to make the code easier to read). */
typedef int64_t fixed_t;

/* The precision of the fixed point numbers. */
#define PRECISION 15

#define SQRT_PRECISION 10

/* Convert a float to a fixed point number. */
#define TO_FIXED(num) (fixed_t)((num)*(fixed_t)(1<<PRECISION))
/* Convert a fixed point number to an integer. */
#define TO_INT(num) ((num)>>PRECISION)
/* Multiply two fixed point numbers together. */
#define MUL(a, b) (((a)*(b))>>PRECISION)

#define DIV(a, b) (((a)<<PRECISION)/(b))

#define FLOOR(num) (num&~((1<<PRECISION)-1))

/* See
 * https://en.wikipedia.org/wiki/Bh%C4%81skara_I%27s_sine_approximation_formula.
 */
fixed_t dsin(fixed_t org_d);

fixed_t dcos(fixed_t d);

fixed_t dtan(fixed_t d);

fixed_t fsqrt(fixed_t n);

#endif

