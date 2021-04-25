/*
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
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#define ALIGNED(x) __declspec(align(x))
#else
#define ALIGNED(x) __attribute__((aligned(x)))
#endif


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "y262api.h"

#include "types.h"
#include "tables.h"
#include "bitstream.h"
#include "aboveslicelevel.h"
#include "lookahead.h"
#include "transform.h"
#include "pixelop.h"
#include "me.h"
#include "ratectrl.h"
#include "threads.h"

#ifdef ASSEMBLY_X86
#include "transform_x86.h"
#include "pixelop_x86.h"
#endif

#ifdef ASSEMBLY_ARM64
#include "transform_arm64.h"
#include "pixelop_arm64.h"
#endif

void y262_init_motion_compensation( y262_t *ps_y262 );
void y262_error( y262_t *ps_y262, int32_t i_error_code, int8_t* pi8_format, ... );
void y262_encode_picture( y262_t *ps_y262, y262_picture_t *ps_picture, int32_t i_picture_type, int32_t i_pon );


#define MIN( x, y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )
#define MAX( x, y ) ( ( x ) > ( y ) ? ( x ) : ( y ) )

