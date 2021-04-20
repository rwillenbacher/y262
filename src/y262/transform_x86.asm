%if 0
Copyright (c) 2016, Ralf Willenbacher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
%endif


%include "x86inc.asm"

SECTION_RODATA

; fdct
ALIGN 16
y262_fdct_tab1:
dw  22724,  19265,  22724,  19265,  22724,  19265,  22724,  19265
dw  12872,   4520,  12872,   4520,  12872,   4520,  12872,   4520
dw  19265,  -4520,  19265,  -4520,  19265,  -4520,  19265,  -4520
dw -22724, -12872, -22724, -12872, -22724, -12872, -22724, -12872
dw  12872, -22724,  12872, -22724,  12872, -22724,  12872, -22724
dw   4520,  19265,   4520,  19265,   4520,  19265,   4520,  19265
dw   4520, -12872,   4520, -12872,   4520, -12872,   4520, -12872
dw  19265, -22724,  19265, -22724,  19265, -22724,  19265, -22724
dw  21406,   8867,  21406,   8867,  21406,   8867,  21406,   8867
dw  -8867, -21406,  -8867, -21406,  -8867, -21406,  -8867, -21406
dw   8867, -21406,   8867, -21406,   8867, -21406,   8867, -21406
dw  21406,  -8867,  21406,  -8867,  21406,  -8867,  21406,  -8867
dw  16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383
dw  16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383
dw  16383, -16383,  16383, -16383,  16383, -16383,  16383, -16383
dw -16383,  16383, -16383,  16383, -16383,  16383, -16383,  16383

ALIGN 16
y262_fdct_rnd1:
dd	1024, 1024, 1024, 1024

ALIGN 16
y262_fdct_tab2:
dw  16385,  16385,  22726,  19266,  -8867, -21408, -22726, -12873
dw  16385,  16385,  12873,   4521,  21408,   8867,  19266,  -4521
dw  16385, -16385,  12873, -22726,  21408,  -8867,  19266, -22726
dw -16385,  16385,   4521,  19266,   8867, -21408,   4521, -12873
dw  16385,  22726,  21408,  19266,  16385,  12873,   8867,   4521
dw  16385,  19266,   8867,  -4521, -16385, -22726, -21408, -12873
dw  16385,  12873,  -8867, -22726, -16385,   4521,  21408,  19266
dw  16385,   4521, -21408, -12873,  16385,  19266,  -8867, -22726

ALIGN16
y262_fdct_rnd2:
dd	524288, 524288, 524288, 524288

; idct

ALIGN 16
y262_idct_tab1:
dw  22724,  19265,  22724,  19265,  22724,  19265,  22724,  19265
dw  12872,   4520,  12872,   4520,  12872,   4520,  12872,   4520
dw  19265,  -4520,  19265,  -4520,  19265,  -4520,  19265,  -4520
dw -22724, -12872, -22724, -12872, -22724, -12872, -22724, -12872
dw  12872, -22724,  12872, -22724,  12872, -22724,  12872, -22724
dw   4520,  19265,   4520,  19265,   4520,  19265,   4520,  19265
dw   4520, -12872,   4520, -12872,   4520, -12872,   4520, -12872
dw  19265, -22724,  19265, -22724,  19265, -22724,  19265, -22724
dw  21406,   8867,  21406,   8867,  21406,   8867,  21406,   8867
dw  16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383
dw   8867, -21406,   8867, -21406,   8867, -21406,   8867, -21406
dw  16383, -16383,  16383, -16383,  16383, -16383,  16383, -16383

ALIGN 16
y262_idct_rnd1:
dd 1024, 1024, 1024, 1024

ALIGN 16
y262_idct_tab2:
dw  16385,  21408,  16385,   8867,  16385,  -8867,  16385, -21408
dw  16385,   8867, -16385, -21408, -16385,  21408,  16385,  -8867
dw  22726,  19266,  19266,  -4521,  12873, -22726,   4521, -12873
dw  12873,   4521, -22726, -12873,   4521,  19266,  19266, -22726

ALIGN 16
y262_idct_rnd2:
dd 524288, 524288, 524288, 524288


; quant

ALIGN 16
minus_1 : dd 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
g_127   : dw 127, 127, 127, 127, 127, 127, 127, 127
g_n127  : dw -127, -127, -127, -127, -127, -127, -127, -127
g_2047   : dw 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047
g_m2048  : dw -2048, -2048, -2048, -2048, -2048, -2048, -2048, -2048

SECTION .text


INIT_XMM
;void y262_fdct_sse2( short *block, short *dst )
cglobal y262_fdct_sse2, 2, 5, 8
	lea		r2, [ y262_fdct_tab1 ]
	movdqu	m1, [ r0 ]
	movdqu	m7, [ r0 + 112 ]
	movdqa	m6, m1
	psubsw	m1, m7
	paddsw	m6, m7
	movdqu	[ r1 ], m6
	movdqu	m2, [ r0 + 16 ]
	movdqu	m7, [ r0 + 96 ]
	movdqa	m6, m2
	psubsw	m2, m7
	paddsw	m6, m7
	movdqu	m3, [ r0 + 32 ]
	movdqu	[ r1 + 32 ], m6
	movdqu	m7, [ r0 + 80 ]
	movdqa	m6, m3
	psubsw	m3, m7
	paddsw	m6, m7
	movdqu	m4, [ r0 + 48 ]
	movdqu	m7, [ r0 + 64 ]
	movdqu	[ r1 + 64 ], m6
	movdqa	m6, m4
	psubsw	m4, m7
	paddsw	m6, m7
	movdqu	[ r1 + 96 ], m6

	movdqa	m0, m1
	punpcklwd	m0, m2
	punpckhwd	m1, m2
	movdqa	m2, m3
	punpcklwd	m2, m4
	punpckhwd	m3, m4

	movdqa	m6, m0
	movdqa	m7, m1
	pmaddwd	m6, [ r2 ]
	pmaddwd	m7, [ r2 ]
	movdqa	m4, m2
	movdqa	m5, m3
	pmaddwd	m4, [ r2 + 16 ]
	pmaddwd	m5, [ r2 + 16 ]
	paddd	m6, m4
	paddd	m7, m5
	paddd	m6, [ y262_fdct_rnd1 ]
	paddd	m7, [ y262_fdct_rnd1 ]
	psrad	m6, 11
	psrad	m7, 11
	packssdw m6, m7
	movdqu	[ r1 + 16 ], m6
	add		r2, 32
	movdqa	m6, m0
	movdqa	m7, m1
	pmaddwd	m6, [ r2 ]
	pmaddwd	m7, [ r2 ]
	movdqa	m4, m2
	movdqa	m5, m3
	pmaddwd	m4, [ r2 + 16 ]
	pmaddwd	m5, [ r2 + 16 ]
	paddd	m6, m4
	paddd	m7, m5
	paddd	m6, [ y262_fdct_rnd1 ]
	paddd	m7, [ y262_fdct_rnd1 ]
	psrad	m6, 11
	psrad	m7, 11
	packssdw m6, m7
	movdqu	[ r1 + 48 ], m6
	add		r2, 32
	movdqa	m6, m0
	movdqa	m7, m1
	pmaddwd	m6, [ r2 ]
	pmaddwd	m7, [ r2 ]
	movdqa	m4, m2
	movdqa	m5, m3
	pmaddwd	m4, [ r2 + 16 ]
	pmaddwd	m5, [ r2 + 16 ]
	paddd	m6, m4
	paddd	m7, m5
	paddd	m6, [ y262_fdct_rnd1 ]
	paddd	m7, [ y262_fdct_rnd1 ]
	psrad	m6, 11
	psrad	m7, 11
	packssdw m6, m7
	movdqu	[ r1 + 80 ], m6
	add		r2, 32
	pmaddwd	m0, [ r2 ]
	pmaddwd	m1, [ r2 ]
	pmaddwd	m2, [ r2 + 16 ]
	pmaddwd	m3, [ r2 + 16 ]
	paddd	m0, m2
	paddd	m1, m3
	paddd	m0, [ y262_fdct_rnd1 ]
	paddd	m1, [ y262_fdct_rnd1 ]
	psrad	m0, 11
	psrad	m1, 11
	packssdw m0, m1
	movdqu	[ r1 + 112 ], m0
	add		r2, 32


	movdqu	m1, [ r1 ]
	movdqu	m7, [ r1 + 96 ]
	movdqa	m6, m1
	psubsw	m1, m7
	paddsw	m6, m7
	movdqu	[ r1 ], m6
	movdqu	m2, [ r1 + 32 ]
	movdqu	m7, [ r1 + 64 ]
	movdqa	m6, m2
	psubsw	m2, m7
	paddsw	m6, m7
	movdqu	[ r1 + 64 ], m6

	movdqa	m0, m1
	punpcklwd	m0, m2
	punpckhwd	m1, m2

	movdqa	m6, m0
	movdqa	m7, m1
	pmaddwd	m6, [ r2 ]
	pmaddwd	m7, [ r2 ]
	paddd	m6, [ y262_fdct_rnd1 ]
	paddd	m7, [ y262_fdct_rnd1 ]
	psrad	m6, 11
	psrad	m7, 11
	packssdw m6, m7
	movdqu	[ r1 + 32 ], m6
	add		r2, 32
	movdqa	m6, m0
	movdqa	m7, m1
	pmaddwd	m6, [ r2 ]
	pmaddwd	m7, [ r2 ]
	paddd	m6, [ y262_fdct_rnd1 ]
	paddd	m7, [ y262_fdct_rnd1 ]
	psrad	m6, 11
	psrad	m7, 11
	packssdw m6, m7
	movdqu	[ r1 + 96 ], m6
	add		r2, 32


	movdqu	m0, [ r1 ]
	movdqu	m2, [ r1 + 64 ]
	movdqa	m1, m0
	punpcklwd m0, m2
	punpckhwd m1, m2

	movdqa	m6, m0
	movdqa	m7, m1
	pmaddwd	m6, [ r2 ]
	pmaddwd	m7, [ r2 ]
	paddd	m6, [ y262_fdct_rnd1 ]
	paddd	m7, [ y262_fdct_rnd1 ]
	psrad	m6, 11
	psrad	m7, 11
	packssdw m6, m7
	movdqu	[ r1 + 0 ], m6
	add		r2, 32
	movdqa	m6, m0
	movdqa	m7, m1
	pmaddwd	m6, [ r2 ]
	pmaddwd	m7, [ r2 ]
	paddd	m6, [ y262_fdct_rnd1 ]
	paddd	m7, [ y262_fdct_rnd1 ]
	psrad	m6, 11
	psrad	m7, 11
	packssdw m6, m7
	movdqu	[ r1 + 64 ], m6

	mov			r3, 8
	lea			r2, [ y262_fdct_tab2 ]
.y262_fdct_sse2_rowloop:
	movq		m0, [ r1 ]
	movq		m2, [ r1 + 8 ]
	movdqa		m1, m0
	pshuflw		m2, m2, 0x1b
	psubsw		m1, m2
	paddsw		m0, m2
	punpckldq   m0, m1
	pshufd		m1, m0, 0x4e
	movdqa		m2, [ r2 ]
	movdqa		m3, [ r2 + 16 ]
	movdqa		m4, [ r2 + 32 ]
	movdqa		m5, [ r2 + 48 ]
	pmaddwd		m2, m0
	pmaddwd		m3, m1
	pmaddwd		m4, m0
	pmaddwd		m5, m1
	paddd		m2, m3
	paddd		m4, m5
	paddd		m2, [ y262_fdct_rnd2 ]
	paddd		m4, [ y262_fdct_rnd2 ]
	psrad		m2, 20
	psrad		m4, 20
	packssdw	m2, m4
	movdqu		[ r1 ], m2
	add			r1, 16

	sub			r3, 1
	jnz .y262_fdct_sse2_rowloop

	RET



INIT_XMM
; void y262_idct_sse2( int16_t *pi16_src, int16_t *pi_dst )
cglobal y262_idct_sse2, 2, 5, 8
	lea			r2, [ y262_idct_tab1 ]
	mov			r3, 2
y262_idct_sse2_loop_v:
	movq		m0, [ r0 + 16 ]
	movq		m2, [ r0 + 48 ]
	movq		m1, [ r0 + 80 ]
	movq		m3, [ r0 + 112 ]
	punpcklwd	m0, m2
	punpcklwd	m1, m3

	movdqa		m2, [ r2 ]
	movdqa		m7, [ r2 + 16 ]
	pmaddwd		m2, m0
	pmaddwd		m7, m1
	paddd		m2, m7

	movdqa		m3, [ r2 + 32 ]
	movdqa		m7, [ r2 + 48 ]
	pmaddwd		m3, m0
	pmaddwd		m7, m1
	paddd		m3, m7

	movdqa		m4, [ r2 + 64 ]
	movdqa		m7, [ r2 + 80 ]
	pmaddwd		m4, m0
	pmaddwd		m7, m1
	paddd		m4, m7

	movdqa		m5, [ r2 + 96 ]
	movdqa		m7, [ r2 + 112 ]
	pmaddwd		m5, m0
	pmaddwd		m7, m1
	paddd		m5, m7

	movq		m6, [ r0 + 32 ]
	movq		m0, [ r0 + 96 ]
	punpcklwd	m6, m0
	pmaddwd		m6, [ r2 + 128 ]

	movq		m7, [ r0 + 0 ]
	movq		m0, [ r0 + 64 ]
	punpcklwd	m7, m0
	pmaddwd		m7, [ r2 + 144 ]

	movdqa		m0, m6
	paddd		m0, m7
	psubd		m7, m6

	movdqa		m1, m2
	paddd		m1, m0
	psubd		m0, m2
	paddd		m0, [ y262_idct_rnd1 ]
	paddd		m1, [ y262_idct_rnd1 ]
	psrad		m1, 11
	psrad		m0, 11
	packssdw	m1, m1
	packssdw	m0, m0
	movq		[ r1 + 112 ], m0

	movdqa		m2, m5
	paddd		m2, m7
	psubd		m7, m5
	paddd		m2, [ y262_idct_rnd1 ]
	paddd		m7, [ y262_idct_rnd1 ]
	psrad		m2, 11
	psrad		m7, 11
	packssdw	m2, m2
	movq		[ r1 + 48 ], m2
	packssdw	m7, m7

	movq		m6, [ r0 + 32 ]
	movq		m0, [ r0 + 96 ]
	punpcklwd	m6, m0
	pmaddwd		m6, [ r2 + 160 ]

	movq		m2, [ r0 + 0 ]
	movq		m0, [ r0 + 64 ]
	movq		[ r1 ], m1
	movq		[ r1 + 64 ], m7
	punpcklwd	m2, m0
	pmaddwd		m2, [ r2 + 176 ]

	movdqa		m0, m6
	paddd		m0, m2
	psubd		m2, m6

	movdqa		m7, m3
	paddd		m7, m0
	psubd		m0, m3
	paddd		m7, [ y262_idct_rnd1 ]
	paddd		m0, [ y262_idct_rnd1 ]
	psrad		m7, 11
	psrad		m0, 11
	packssdw	m7, m7
	packssdw	m0, m0
	movq		[ r1 + 16 ], m7
	movq		[ r1 + 96 ], m0

	movdqa		m1, m4
	paddd		m1, m2
	psubd		m2, m4
	paddd		m1, [ y262_idct_rnd1 ]
	paddd		m2, [ y262_idct_rnd1 ]
	psrad		m1, 11
	psrad		m2, 11
	packssdw	m1, m1
	movq		[ r1 + 32 ], m1
	packssdw	m2, m2
	movq		[ r1 + 80 ], m2

	add			r0, 8
	add			r1, 8
	sub			r3, 1
	jnz y262_idct_sse2_loop_v

	sub			r1, 16
	lea			r2, [ y262_idct_tab2 ]
	mov			r3, 8
.y262_idct_sse2_loop_h:
	movdqu		m7, [ r1 ]
	pshuflw		m0, m7, 0x88
	punpckldq	m0, m0
	pshufhw		m1, m7, 0x88
	punpckhdq	m1, m1

	pshuflw		m2, m7, 0xdd
	punpckldq	m2, m2
	pshufhw		m3, m7, 0xdd
	punpckhdq	m3, m3

	pmaddwd		m0, [ r2 ]
	pmaddwd		m1, [ r2 + 16 ]
	pmaddwd		m2, [ r2 + 32 ]
	pmaddwd		m3, [ r2 + 48 ]

	paddd		m0, m1
	paddd		m2, m3
	movdqa		m1, m0
	paddd		m0, m2
	psubd		m1, m2
	pshufd		m1, m1, 0x1b

	paddd		m0, [ y262_idct_rnd2 ]
	paddd		m1, [ y262_idct_rnd2 ]
	psrad		m0, 20
	psrad		m1, 20
	packssdw	m0, m1

	movdqu		[ r1 ], m0
	add			r1, 16
	sub			r3, 1
	jnz .y262_idct_sse2_loop_h

	RET








; int32_t y262_quant8x8_intra_fw_sse2( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias )
INIT_XMM
cglobal y262_quant8x8_intra_fw_sse2, 4, 6, 5
%ifdef ARCH_X86_64
	movsxd		r1, r1d
%endif
    pxor		xmm3, xmm3
	shl			r1, 1
	mov			r5, r0
	mov			r4w, [ r5 ]
%rep 8
    movdqu		xmm0, [ r0 ]
	movdqu		xmm2, [ r2 ]
	movdqu		xmm4, [ r3 ]

    pxor        xmm1, xmm1
    pcmpgtw     xmm1, xmm0
    pxor        xmm0, xmm1
    psubw       xmm0, xmm1
	paddusw		xmm0, xmm4
    pmulhuw     xmm0, xmm2
    pxor        xmm0, xmm1
    psubw       xmm0, xmm1

	pminsw		xmm0, [ g_2047  ]	
	pmaxsw		xmm0, [ g_m2048  ]	

    por			xmm3, xmm0
    movdqu      [ r0 ], xmm0
    
    add         r0, r1
	add			r2, 16
	add			r3, 16
%endrep

	movdqa      xmm0, [ minus_1  ]
	pxor        xmm1, xmm1
	pcmpeqb     xmm3, xmm1
	pxor        xmm3, xmm0
	
	mov			[ r5 ], r4w
	pmovmskb    eax, xmm3

	RET

; int32_t y262_quant8x8_inter_fw_sse2( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat )
INIT_XMM
cglobal y262_quant8x8_inter_fw_sse2, 3, 3, 4
%ifdef ARCH_X86_64
	movsxd		r1, r1d
%endif
    pxor		xmm3, xmm3
	shl			r1, 1
%rep 8
    movdqu		xmm0, [ r0 ]
	movdqu		xmm2, [ r2 ]

    pxor        xmm1, xmm1
    pcmpgtw     xmm1, xmm0
    pxor        xmm0, xmm1
    psubw       xmm0, xmm1
    pmulhuw     xmm0, xmm2
    pxor        xmm0, xmm1
    psubw       xmm0, xmm1

	pminsw		xmm0, [ g_2047  ]	
	pmaxsw		xmm0, [ g_m2048  ]	

    por			xmm3, xmm0
    movdqu      [ r0 ], xmm0
    
    add         r0, r1
	add			r2, 16
%endrep

	movdqa      xmm0, [ minus_1  ]
	pxor        xmm1, xmm1
	pcmpeqb     xmm3, xmm1
	pxor        xmm3, xmm0
	
	pmovmskb    eax, xmm3

	RET

