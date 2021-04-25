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

int32_t y262_sad_16x16_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 );
int32_t y262_sad_16x8_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 );

int32_t y262_satd_16x16_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 );
int32_t y262_satd_16x8_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 );

int32_t y262_ssd_8x8_neon( uint8_t *pui8_blk1, int32_t i_blk1_stride, uint8_t *pui8_blk2, int32_t i_blk2_stride );
int32_t y262_ssd_16x16_neon( uint8_t *pui8_blk1, int32_t i_blk1_stride, uint8_t *pui8_blk2, int32_t i_blk2_stride );

void y262_sub_8x8_neon( int16_t *pi16_diff, uint8_t *pui8_src1, int32_t i_stride_src1, uint8_t *pui8_src2, int32_t i_stride_src2 );
void y262_add_8x8_neon( uint8_t *pui8_destination, int32_t i_destination_stride, uint8_t *pui8_base, int32_t i_base_stride, int16_t *pi16_difference );

