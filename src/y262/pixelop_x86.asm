%if 0
Copyright (c) 2013, Ralf Willenbacher
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

ALIGN 16
M128_FILTER_HOR_MASK_INNER : dw 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000
ALIGN 16
M128_FILTER_HOR_MASK_OUTER : dw 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff
ALIGN 16
M128_FILTER_TWO			   : dw 2, 2, 2, 2, 2, 2, 2, 2
ALIGN 16
M128_FILTER_EIGHT		   : dw 8, 8, 8, 8, 8, 8, 8, 8

ALIGN 16
M128_ONE		   : dw 1, 1, 1, 1, 1, 1, 1, 1

ALIGN 16
M128_ONE_BYTE : db 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1

SECTION .text

INIT_XMM






%macro HADAMARD4x2 5
	paddw	%1, %2
	paddw	%3, %4
	paddw	%2, %2
	paddw	%4, %4
	psubw   %2, %1
	psubw   %4, %3
	paddw	%1, %3
	paddw	%2, %4
	paddw	%3, %3
	paddw	%4, %4
	psubw   %3, %1
	psubw   %4, %2
	mova	%5, %1
	punpcklwd %1, %2
	punpckhwd %5, %2
	mova %2, %3
	punpcklwd %3, %4
	punpckhwd %2, %4
	mova %4, %1
	punpckldq %1, %3
	punpckhdq %4, %3
	mova %3, %5
	punpckhdq %5, %2
	punpckldq %3, %2
	mova %2, %1
	punpcklqdq %1, %3
	punpckhqdq %2, %3
	mova %3, %4
	punpckhqdq %3, %5
	punpcklqdq %4, %5
	paddw	%1, %2
	paddw	%3, %4
	paddw	%2, %2
	paddw	%4, %4
	psubw   %2, %1
	psubw   %4, %3
	paddw	%1, %3
	paddw	%2, %4
	paddw	%3, %3
	paddw	%4, %4
	psubw   %3, %1
	psubw   %4, %2
%endmacro

%macro ABSSUM 7
	mova	%6, %1
	pxor	%5, %5
	psubw	%5, %6
	pmaxsw  %6, %5
	paddusw	%7, %6
	mova	%6, %2
	pxor	%5, %5
	psubw	%5, %6
	pmaxsw  %6, %5
	paddusw	%7, %6
	mova	%6, %3
	pxor	%5, %5
	psubw	%5, %6
	pmaxsw  %6, %5
	paddusw	%7, %6
	mova	%6, %4
	pxor	%5, %5
	psubw	%5, %6
	pmaxsw  %6, %5
	paddusw	%7, %6
%endmacro

%macro ABSSUMX2 5
	mova	%4, %1
	pxor	%3, %3
	psubw	%3, %4
	pmaxsw  %4, %3
	paddusw	%5, %4
	mova	%4, %2
	pxor	%3, %3
	psubw	%3, %4
	pmaxsw  %4, %3
	paddusw	%5, %4
%endmacro

%macro ADDSUB 2
	paddw	%1, %2
	paddw	%2, %2
	psubw	%2, %1
%endmacro

%macro LOADDIFF8x4 10	; 0-3, tmp, zero, p1, stride1, p2, stride2
	movq	  %1, [ %7 ]
	movq	  %5, [ %9 ]
	punpcklbw %1, %6
	punpcklbw %5, %6
	psubsw	  %1, %5
	movq	  %2, [ %7 + %8 ]
	movq	  %5, [ %9 + %10 ]
	lea		  %7, [ %7 + 2 * %8 ]
	lea		  %9, [ %9 + 2 * %10 ]
	punpcklbw %2, %6
	punpcklbw %5, %6
	psubsw	  %2, %5
	movq	  %3, [ %7 ]
	movq	  %5, [ %9 ]
	punpcklbw %3, %6
	punpcklbw %5, %6
	psubsw	  %3, %5
	movq	  %4, [ %7 + %8 ]
	movq	  %5, [ %9 + %10 ]
	lea		  %7, [ r0 + 2 * r1 ]
	lea		  %9, [ r2 + 2 * r3 ]
	punpcklbw %4, %6
	punpcklbw %5, %6
	psubsw	  %4, %5
%endmacro

; int32_t __cdecl y262_satd_8x8_sse2( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )

INIT_XMM
cglobal y262_satd_8x8_sse2, 4, 5, 8
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	sub	      rsp, 0x40
	pxor	  m7, m7

	LOADDIFF8x4 m0, m1, m2, m3, m4, m7, r0, r1, r2, r3
	HADAMARD4x2 m0, m1, m2, m3, m4

	movu	[ rsp ], m0
	movu	[ rsp + 16 ], m1
	movu	[ rsp + 32 ], m2
	movu	[ rsp + 48 ], m3

	LOADDIFF8x4 m0, m1, m2, m3, m4, m7, r0, r1, r2, r3
	HADAMARD4x2 m0, m1, m2, m3, m4

	movu	m4, [ rsp ]
	movu	m5, [ rsp + 16 ]
	movu	m6, [ rsp + 32 ]
	movu	m7, [ rsp + 48 ]
	ADDSUB	m4, m0
	ADDSUB	m5, m1
	ADDSUB	m6, m2
	ADDSUB	m7, m3
	;movu	[ rsp ], m4
	movu	[ rsp + 16 ], m5
	movu	[ rsp + 32 ], m6
	movu	[ rsp + 48 ], m7

	;movu	m4, [ rsp ]
	mova	m5, m4
	punpcklqdq m4, m0
	punpckhqdq m5, m0
	ADDSUB	m4, m5
	pxor	m0, m0	; sum = 0
	ABSSUMX2	m4, m5, m6, m7, m0

	movu	m4, [ rsp + 16 ]
	mova	m5, m4
	punpcklqdq m4, m1
	punpckhqdq m5, m1
	ADDSUB	m4, m5
	ABSSUMX2	m4, m5, m6, m7, m0

	movu	m4, [ rsp + 32 ]
	mova	m5, m4
	punpcklqdq m4, m2
	punpckhqdq m5, m2
	ADDSUB	m4, m5
	ABSSUMX2	m4, m5, m6, m7, m0

	movu	m4, [ rsp + 48 ]
	mova	m5, m4
	punpcklqdq m4, m3
	punpckhqdq m5, m3
	ADDSUB	m4, m5
	ABSSUMX2	m4, m5, m6, m7, m0

	pmaddwd	m0, [ M128_ONE ]
	movhlps	m6, m0
	paddd	m0, m6
	pshuflw	m6, m0, 0xE
	paddd	m0, m6

	movd	r4d, m0
	add		r4d, 2
	shr		r4d, 2
	mov		eax, r4d

	add	    rsp, 0x40

	RET


;-----------------------------------------------------------------------------
;   int32_t __cdecl y262_sad_16x16_sse2( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
;-----------------------------------------------------------------------------

cglobal y262_sad_16x16_sse2, 4, 4, 8
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
    movdqu m6, [r0]
    movdqu m7, [r0+r1]
    lea    r0,  [r0+2*r1]
    movdqu m5, [r0]
    movdqu m4, [r0+r1]
    lea    r0,  [r0+2*r1]
    psadbw m6, [r2]
    psadbw m7, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m0, [r0]
    paddw  m6, m7
    psadbw m5, [r2]
    psadbw m4, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m2, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m5, m4
    movdqu m3, [r0]
    movdqu m1, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m6, m5
    psadbw m0, [r2]
    psadbw m2, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m7, [r0]
    paddw  m0, m2
    psadbw m3, [r2]
    psadbw m1, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m5, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m3, m1
    movdqu m4, [r0]
    paddw  m6, m0
    movdqu m0, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m6, m3
    psadbw m7, [r2]
    psadbw m5, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m2, [r0]
    paddw  m7, m5
    psadbw m4, [r2]
    psadbw m0, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m3, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m4, m0
    movdqu m1, [r0]
    paddw  m6, m7
    movdqu m7, [r0+r1]
    paddw  m6, m4
    psadbw m2, [r2]
    psadbw m3, [r2+r3]
    lea    r2,  [r2+2*r3]
    paddw  m2, m3
    psadbw m1, [r2]
    psadbw m7, [r2+r3]
    paddw  m1, m7
    paddw  m6, m2
    paddw  m6, m1
    
    movdqa  m7, m6
    psrldq  m6,  8
    paddw   m6, m7
    movd    eax,  m6

    RET

;-----------------------------------------------------------------------------
;   int32_t __cdecl y262_sad_16x8_sse2( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
;-----------------------------------------------------------------------------

cglobal y262_sad_16x8_sse2, 4, 4, 8
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
    movdqu m7, [r0]
    movdqu m6, [r0+r1]
    lea    r0,  [r0+2*r1]
    movdqu m5, [r0]
    movdqu m4, [r0+r1]
    lea    r0,  [r0+2*r1]

    psadbw m7, [r2]
    psadbw m6, [r2+r3]
    lea    r2,  [r2+2*r3]
    psadbw m5, [r2]
    psadbw m4, [r2+r3]
    lea    r2,  [r2+2*r3]

    paddw  m7, m6
    paddw  m5, m4
    paddw  m7, m5

    movdqu m6, [r0]
    movdqu m5, [r0+r1]
    lea    r0,  [r0+2*r1]
    movdqu m4, [r0]
    movdqu m3, [r0+r1]

    psadbw m6, [r2]
    psadbw m5, [r2+r3]
    lea    r2,  [r2+2*r3]
    psadbw m4, [r2]
    psadbw m3, [r2+r3]

    paddw  m6, m5
    paddw  m4, m3
    paddw  m7, m6
    paddw  m7, m4
    
    movdqa  m6, m7
    psrldq  m6,  8
    paddw   m6, m7
    movd    eax, m6

    RET


;-----------------------------------------------------------------------------
; int32_t y262_ssd_8x8_sse2( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 );
;-----------------------------------------------------------------------------

cglobal y262_ssd_8x8_sse2, 4, 4, 8
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
    pxor		m7, m7
    pxor		m6, m6
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	

    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

	pshufd		m7, m6, 11100101b
	pshufd		m5, m6, 11100110b
	pshufd		m4, m6, 11100111b
	paddd		m7, m6
	paddd		m7, m5
	paddd		m7, m4
	
	movd		eax, m7    
	RET


;-----------------------------------------------------------------------------
;   int32_t __cdecl y262_ssd_16x16_sse2( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
;-----------------------------------------------------------------------------

cglobal y262_ssd_16x16_sse2, 4, 4, 8
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
    pxor		m7, m7
    pxor		m6, m6

; 0-1
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	

; 2-3
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 4-5
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 6-7
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
; 8-9
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]

    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 10-11
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 12-13
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]

    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 14-15
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

	pshufd		m7, m6, 11100101b
	pshufd		m5, m6, 11100110b
	pshufd		m4, m6, 11100111b
	paddd		m7, m6
	paddd		m7, m5
	paddd		m7, m4
	
	movd		eax, m7    
	RET



;-----------------------------------------------------------------------------------------------------------------
;Void __cdecl y262_sub_8x8_sse2( int16_t *pi16_diff, uint8_t *pui8_src1, int32_t i_stride_src1, uint8_t *pui8_src2, int32_t i_stride_src2 );
;-----------------------------------------------------------------------------------------------------------------
cglobal y262_sub_8x8_sse2, 5, 5, 5
%ifdef ARCH_X86_64
	movsxd	  r2, r2d
	movsxd	  r4, r4d
%endif

	pxor		m4, m4

	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2
	add			r0, 32
	
	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2
	add			r0, 32
	
	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2
	add			r0, 32
	
	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2

	RET

;-----------------------------------------------------------------------------------------------------------------
;void __cdecl y262_add_8x8_sse2( uint8_t *pui8_destination, int32_t i_destination_stride, uint8_t *pui8_base, int32_t i_base_stride, int16_t *pi_difference );
;-----------------------------------------------------------------------------------------------------------------
INIT_XMMS

cglobal y262_add_8x8_sse2, 5, 5, 5
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif

	pxor		m4, m4

%rep 4
	movq		m0, [ r2 ]
	movq		m2, [ r2 + r3 ]
	movdqu		m1, [ r4 ]
	movdqu		m3, [ r4 + 16 ]
	
	punpcklbw	m0, m4
	punpcklbw	m2, m4

	paddsw		m0, m1
	paddsw		m2, m3
	
	packuswb	m0, m0
	packuswb	m2, m2
	
	movq		[ r0 ], m0
	movq		[ r0 + r1 ], m2

	lea			r0, [ r0 + r1 * 2 ]
	lea			r2, [ r2 + r3 * 2 ]
	add			r4, 32
%endrep

	RET

	

; motcomp functions

%macro MMX_LINE_00 1
%rep %1
		movq mm0, [ r0 ]
		add r0, r1
		movq [ r2 ], mm0
		add r2, r3
%endrep
%endmacro

%macro MMX_LINE_01 1
%rep %1
		movq mm0, [ r0 ]
		movq mm1, [ r0 + 1 ]
		add r0, r1
		pavgb mm0, mm1
		movq [ r2 ], mm0
		add r2, r3
%endrep
%endmacro

%macro MMX_LINE_10 1
		movq mm0, [ r0 ]
%rep %1
		movq mm1, [ r0 + r1 ]
		add r0, r1
		pavgb mm0, mm1
		movq [ r2 ], mm0
		add r2, r3
		movq mm0, mm1
%endrep
%endmacro

%macro MMX_LINE_11 1
		movq mm0, [ r0 ]
		movq mm2, [ r0 + 1 ]
%rep %1
		movq mm1, [ r0 + r1 ]
		movq mm3, [ r0 + r1 + 1 ]
		add r0, r1
		movq mm5, mm0
		pxor mm5, mm1
		movq mm4, mm2
		pxor mm4, mm3
		pavgb mm0, mm1
		pavgb mm2, mm3
		por	mm5, mm4
		movq mm4, mm0
		pxor mm4, mm2
		pand mm5, mm4
		pand mm5, [ M128_ONE_BYTE ]
		pavgb mm0, mm2
		psubusb mm0, mm5
		movq [ r2 ], mm0
		add r2, r3
		movq mm0, mm1
		movq mm2, mm3
%endrep
%endmacro


%macro MMX_LINE_00A 1
%rep %1
		movq mm0, [ r0 ]
		add r0, r1
		movq mm1, [ r2 ]
		pavgb mm0, mm1
		movq [ r2 ], mm0
		add r2, r3
%endrep
%endmacro

%macro MMX_LINE_01A 1
%rep %1
		movq mm0, [ r0 ]
		movq mm1, [ r0 + 1 ]
		add r0, r1
		pavgb mm0, mm1
		movq mm1, [ r2 ]
		pavgb mm0, mm1
		movq [ r2 ], mm0
		add r2, r3
%endrep
%endmacro


%macro MMX_LINE_10A 1
		movq mm0, [ r0 ]
%rep %1
		movq mm1, [ r0 + r1 ]
		add r0, r1
		pavgb mm0, mm1
		movq mm2, [ r2 ]
		pavgb mm0, mm2
		movq [ r2 ], mm0
		add r2, r3
		movq mm0, mm1
%endrep
%endmacro



%macro MMX_LINE_11A 1
		movq mm0, [ r0 ]
		movq mm2, [ r0 + 1 ]
%rep %1
		movq mm1, [ r0 + r1 ]
		movq mm3, [ r0 + r1 + 1 ]
		add r0, r1
		movq mm6, mm0
		pxor mm6, mm1
		movq mm5, mm2
		pxor mm5, mm3
		pavgb mm0, mm1
		pavgb mm2, mm3
		por	mm6, mm5
		movq mm5, mm0
		pxor mm5, mm2
		pand mm6, mm5
		pand mm6, [ M128_ONE_BYTE ]
		pavgb mm0, mm2
		psubusb mm0, mm6
		movq mm4, [ r2 ]
		pavgb mm0, mm4
		movq [ r2 ], mm0
		add r2, r3
		movq mm0, mm1
		movq mm2, mm3
%endrep
%endmacro


%macro MOTCOMP_MMX_8W_FUNC 1
cglobal y262_motcomp_8x %+ %1 %+ _00_put_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_00 %1
	emms
	RET

cglobal y262_motcomp_8x %+ %1 %+ _01_put_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_01 %1
	emms
	RET

cglobal y262_motcomp_8x %+ %1 %+ _10_put_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_10 %1
	emms
	RET

cglobal y262_motcomp_8x %+ %1 %+ _11_put_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_11 %1
	emms
	RET

cglobal y262_motcomp_8x %+ %1 %+ _00_avg_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_00A %1
	emms
	RET

cglobal y262_motcomp_8x %+ %1 %+ _01_avg_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_01A %1
	emms
	RET

cglobal y262_motcomp_8x %+ %1 %+ _10_avg_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_10A %1
	emms
	RET

cglobal y262_motcomp_8x %+ %1 %+ _11_avg_mmxext, 4, 4, 0
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	MMX_LINE_11A %1
	emms
	RET

%endmacro


MOTCOMP_MMX_8W_FUNC 16
MOTCOMP_MMX_8W_FUNC  8
MOTCOMP_MMX_8W_FUNC  4



%macro SSE2_LINE_00 1
%rep %1
		movdqu xmm0, [ r0 ]
		add r0, r1
		movdqa [ r2 ], xmm0
		add r2, r3
%endrep
%endmacro

%macro SSE2_LINE_01 1
%rep %1
		movdqu xmm0, [ r0 ]
		movdqu xmm1, [ r0 + 1 ]
		add r0, r1
		pavgb xmm0, xmm1
		movdqa [ r2 ], xmm0
		add r2, r3
%endrep
%endmacro


%macro SSE2_LINE_10 1
		movdqu xmm0, [ r0 ]
%rep %1
		movdqu xmm1, [ r0 + r1 ]
		add r0, r1
		pavgb xmm0, xmm1
		movdqa [ r2 ], xmm0
		add r2, r3
		movdqu xmm0, xmm1
%endrep
%endmacro


%macro SSE2_LINE_11 1
		movdqu xmm0, [ r0 ]
		movdqu xmm2, [ r0 + 1 ]
%rep %1
		movdqu xmm1, [ r0 + r1 ]
		movdqu xmm3, [ r0 + r1 + 1 ]
		add r0, r1
		movdqu xmm6, xmm0
		pxor xmm6, xmm1
		movdqu xmm5, xmm2
		pxor xmm5, xmm3
		pavgb xmm0, xmm1
		pavgb xmm2, xmm3
		por	xmm6, xmm5
		movdqu xmm5, xmm0
		pxor xmm5, xmm2
		pand xmm6, xmm5
		pand xmm6, [ M128_ONE_BYTE ]
		pavgb xmm0, xmm2
		psubusb xmm0, xmm6
		movdqa [ r2 ], xmm0
		add r2, r3
		movdqu xmm0, xmm1
		movdqu xmm2, xmm3
%endrep
%endmacro

%macro SSE2_LINE_00A 1
%rep %1
		movdqu xmm0, [ r0 ]
		add r0, r1
		movdqa xmm1, [ r2 ]
		pavgb xmm0, xmm1
		movdqa [ r2 ], xmm0
		add r2, r3
%endrep
%endmacro

%macro SSE2_LINE_01A 1
%rep %1
		movdqu xmm0, [ r0 ]
		movdqu xmm1, [ r0 + 1 ]
		add r0, r1
		pavgb xmm0, xmm1
		movdqa xmm1, [ r2 ]
		pavgb xmm0, xmm1
		movdqa [ r2 ], xmm0
		add r2, r3
%endrep
%endmacro


%macro SSE2_LINE_10A 1
		movdqu xmm0, [ r0 ]
%rep %1
		movdqu xmm1, [ r0 + r1 ]
		add r0, r1
		pavgb xmm0, xmm1
		movdqa xmm2, [ r2 ]
		pavgb xmm0, xmm2
		movdqa [ r2 ], xmm0
		add r2, r3
		movdqu xmm0, xmm1
%endrep
%endmacro



%macro SSE2_LINE_11A 1
		movdqu xmm0, [ r0 ]
		movdqu xmm2, [ r0 + 1 ]
%rep %1
		movdqu xmm1, [ r0 + r1 ]
		movdqu xmm3, [ r0 + r1 + 1 ]
		add r0, r1
		movdqu xmm6, xmm0
		pxor xmm6, xmm1
		movdqu xmm5, xmm2
		pxor xmm5, xmm3
		pavgb xmm0, xmm1
		pavgb xmm2, xmm3
		por	xmm6, xmm5
		movdqu xmm5, xmm0
		pxor xmm5, xmm2
		pand xmm6, xmm5
		pand xmm6, [ M128_ONE_BYTE ]
		pavgb xmm0, xmm2
		psubusb xmm0, xmm6
		movdqa xmm4, [ r2 ]
		pavgb xmm0, xmm4
		movdqa [ r2 ], xmm0
		add r2, r3
		movdqu xmm0, xmm1
		movdqu xmm2, xmm3
%endrep
%endmacro


%macro MOTCOMP_SSE2_16W_FUNC 1
cglobal y262_motcomp_16x %+ %1 %+ _00_put_sse2, 4, 4, 2
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_00 %1
	RET

cglobal y262_motcomp_16x %+ %1 %+ _01_put_sse2, 4, 4, 2
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_01 %1
	RET

cglobal y262_motcomp_16x %+ %1 %+ _10_put_sse2, 4, 4, 3
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_10 %1
	RET

cglobal y262_motcomp_16x %+ %1 %+ _11_put_sse2, 4, 4, 7
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_11 %1
	RET

cglobal y262_motcomp_16x %+ %1 %+ _00_avg_sse2, 4, 4, 2
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_00A %1
	RET

cglobal y262_motcomp_16x %+ %1 %+ _01_avg_sse2, 4, 4, 2
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_01A %1
	RET

cglobal y262_motcomp_16x %+ %1 %+ _10_avg_sse2, 4, 4, 3
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_10A %1
	RET

cglobal y262_motcomp_16x %+ %1 %+ _11_avg_sse2, 4, 4, 7
%ifdef ARCH_X86_64
	movsxd	  r1, r1d
	movsxd	  r3, r3d
%endif
	SSE2_LINE_11A %1
	RET
%endmacro

MOTCOMP_SSE2_16W_FUNC 16
MOTCOMP_SSE2_16W_FUNC 8


