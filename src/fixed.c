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

#include <fixed.h>
#include <stdio.h>

/* See
 * https://en.wikipedia.org/wiki/Bh%C4%81skara_I%27s_sine_approximation_formula.
 */
fixed_t dsin(fixed_t d) {
    fixed_t v;
    fixed_t org_d = d%TO_FIXED(360);
    while(org_d < TO_FIXED(0)) org_d += TO_FIXED(360);
    d = org_d;
    d %= TO_FIXED(180);
    v = DIV(MUL(MUL(TO_FIXED(4), d), (TO_FIXED(180)-d)),
            (TO_FIXED(40500)-MUL(d, (TO_FIXED(180)-d))));
    if(org_d > TO_FIXED(180)) v = -v;
    return v;
}

fixed_t dcos(fixed_t d) {
    return dsin(d+TO_FIXED(90));
}

fixed_t dtan(fixed_t d) {
    return DIV(dsin(d), dcos(d));
}

#define HALF (1<<PRECISION>>1)

fixed_t fsqrt(fixed_t x) {
    int i = 0;
    fixed_t n = 1;
    fixed_t a = x;
    fixed_t rx;
    fixed_t lx;
    while(n < x && i < 64){
        n <<= 1;
        a >>= 1;
        i++;
    }
    lx = MUL((HALF+MUL(HALF, a)), n);
    if(!lx){
        lx = 1;
    }
    for(i=0;i<SQRT_PRECISION;i++){
        rx = MUL(HALF, (lx+DIV(x, lx)));
        lx = rx;
        if(!lx) lx = 1;
    }
    return lx;
}

