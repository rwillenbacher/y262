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

#include <arm_neon.h>

#include "y262.h"

int32_t y262_sad_16x8_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_sad, i_y;
    int64_t i64_sad;
    uint8x16_t v16_blk1, v16_blk2;
    uint8x8_t v8_a, v8_b;
    uint16x8_t v16_res0, v16_res1;
    uint32x4_t v16_hadd0;
    uint32x2_t v8_hadd1;
    uint64x1_t v8_hadd2;
    
    v16_blk1 = vld1q_u8 ( pui8_blk1 );
    pui8_blk1 += i_stride1;
    v16_blk2 = vld1q_u8 ( pui8_blk2 );
    pui8_blk2 += i_stride2;
    v8_a = vget_low_u8( v16_blk1 );
    v8_b = vget_low_u8( v16_blk2 );
    v16_res0 = vabdl_u8 ( v8_a, v8_b );
    v8_a = vget_high_u8( v16_blk1 );
    v8_b = vget_high_u8( v16_blk2 );
    v16_res1 = vabdl_u8 ( v8_a, v8_b );

    for( i_y = 1; i_y < 8; i_y++ )
    {
        v16_blk1 = vld1q_u8 ( pui8_blk1 );
        pui8_blk1 += i_stride1;
        v16_blk2 = vld1q_u8 ( pui8_blk2 );
        pui8_blk2 += i_stride2;
        v8_a = vget_low_u8( v16_blk1 );
        v8_b = vget_low_u8( v16_blk2 );
        v16_res0 = vabal_u8 ( v16_res0, v8_a, v8_b );
        v8_a = vget_high_u8( v16_blk1 );
        v8_b = vget_high_u8( v16_blk2 );
        v16_res1 = vabal_u8 ( v16_res1, v8_a, v8_b );
    }

    v16_res0 = vaddq_u16( v16_res0, v16_res1 );
    v16_hadd0 = vpaddlq_u16( v16_res0 );
    v8_hadd1 = vadd_u32( vget_low_u32( v16_hadd0 ), vget_high_u32( v16_hadd0 ) );
    v8_hadd2 = vpaddl_u32( v8_hadd1 );

    i64_sad = vget_lane_u64( v8_hadd2, 0 );
    i_sad = ( int32_t )i64_sad;

    return i_sad;
}


int32_t y262_sad_16x16_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_sad, i_y;
    int64_t i64_sad;
    uint8x16_t v16_blk1, v16_blk2;
    uint8x8_t v8_a, v8_b;
    uint16x8_t v16_res0, v16_res1;
    uint32x4_t v16_hadd0;
    uint32x2_t v8_hadd1;
    uint64x1_t v8_hadd2;
    
    v16_blk1 = vld1q_u8 ( pui8_blk1 );
    pui8_blk1 += i_stride1;
    v16_blk2 = vld1q_u8 ( pui8_blk2 );
    pui8_blk2 += i_stride2;
    v8_a = vget_low_u8( v16_blk1 );
    v8_b = vget_low_u8( v16_blk2 );
    v16_res0 = vabdl_u8 ( v8_a, v8_b );
    v8_a = vget_high_u8( v16_blk1 );
    v8_b = vget_high_u8( v16_blk2 );
    v16_res1 = vabdl_u8 ( v8_a, v8_b );

    for( i_y = 1; i_y < 16; i_y++ )
    {
        v16_blk1 = vld1q_u8 ( pui8_blk1 );
        pui8_blk1 += i_stride1;
        v16_blk2 = vld1q_u8 ( pui8_blk2 );
        pui8_blk2 += i_stride2;
        v8_a = vget_low_u8( v16_blk1 );
        v8_b = vget_low_u8( v16_blk2 );
        v16_res0 = vabal_u8 ( v16_res0, v8_a, v8_b );
        v8_a = vget_high_u8( v16_blk1 );
        v8_b = vget_high_u8( v16_blk2 );
        v16_res1 = vabal_u8 ( v16_res1, v8_a, v8_b );
    }

    v16_res0 = vaddq_u16( v16_res0, v16_res1 );
    v16_hadd0 = vpaddlq_u16( v16_res0 );
    v8_hadd1 = vadd_u32( vget_low_u32( v16_hadd0 ), vget_high_u32( v16_hadd0 ) );
    v8_hadd2 = vpaddl_u32( v8_hadd1 );

    i64_sad = vget_lane_u64( v8_hadd2, 0 );
    i_sad = ( int32_t )i64_sad;

    return i_sad;
}


#define HADAMARD_NEON_4x2( d0, d1, d2, d3 )                  \
    {                                                        \
        int32x4x2_t v32_a0, v32_b0;                          \
        int16x8x2_t v32_a1, v32_b1;                          \
                                                             \
        d0 = vaddq_s16( d0, d1 );                            \
        d2 = vaddq_s16( d2, d3 );                            \
                                                             \
        d1 = vaddq_s16( d1, d1 );                            \
        d3 = vaddq_s16( d3, d3 );                            \
        d1 = vsubq_s16( d1, d0 );                            \
        d3 = vsubq_s16( d3, d2 );                            \
                                                             \
        d0 = vaddq_s16( d0, d2 );                            \
        d1 = vaddq_s16( d1, d3 );                            \
                                                             \
        d2 = vaddq_s16( d2, d2 );                            \
        d3 = vaddq_s16( d3, d3 );                            \
        d2 = vsubq_s16( d2, d0 );                            \
        d3 = vsubq_s16( d3, d1 );                            \
                                                             \
        v32_a0 = vtrnq_s32( vreinterpretq_s32_s16( d0 ), vreinterpretq_s32_s16( d2 ) );  \
        v32_b0 = vtrnq_s32( vreinterpretq_s32_s16( d1 ), vreinterpretq_s32_s16( d3 ) );  \
        v32_a1 = vtrnq_s16( vreinterpretq_s16_s32( v32_a0.val[ 0 ] ), vreinterpretq_s16_s32( v32_b0.val[ 0 ] ) );  \
        v32_b1 = vtrnq_s16( vreinterpretq_s16_s32( v32_a0.val[ 1 ] ), vreinterpretq_s16_s32( v32_b0.val[ 1 ] ) );  \
        d0 = vcombine_s16( vget_low_s16( v32_a1.val[ 0 ] ), vget_high_s16( v32_a1.val[ 0 ] ) );    \
        d1 = vcombine_s16( vget_low_s16( v32_a1.val[ 1 ] ), vget_high_s16( v32_a1.val[ 1 ] ) );    \
        d2 = vcombine_s16( vget_low_s16( v32_b1.val[ 1 ] ), vget_high_s16( v32_b1.val[ 1 ] ) );    \
        d3 = vcombine_s16( vget_low_s16( v32_b1.val[ 0 ] ), vget_high_s16( v32_b1.val[ 0 ] ) );    \
                                                             \
        d0 = vaddq_s16( d0, d1 );                            \
        d2 = vaddq_s16( d2, d3 );                            \
                                                             \
        d1 = vaddq_s16( d1, d1 );                            \
        d3 = vaddq_s16( d3, d3 );                            \
        d1 = vsubq_s16( d1, d0 );                            \
        d3 = vsubq_s16( d3, d2 );                            \
                                                             \
        d0 = vaddq_s16( d0, d2 );                            \
        d1 = vaddq_s16( d1, d3 );                            \
                                                             \
        d2 = vaddq_s16( d2, d2 );                            \
        d3 = vaddq_s16( d3, d3 );                            \
        d2 = vsubq_s16( d2, d0 );                            \
        d3 = vsubq_s16( d3, d1 );                            \
    }

#define ADDSUB( d0, d1 )                \
    {                                   \
        int16x8_t v16_a, v16_s;         \
        v16_a = vaddq_s16( d0, d1 );    \
        v16_s = vsubq_s16( d1, d0 );    \
        d0 = v16_a;                     \
        d1 = v16_s;                     \
    }


#define ABSADDL( sum, vector0, vector1 )         \
    {                                            \
        vector0 = vabsq_s16( vector0 );          \
        vector1 = vabsq_s16( vector1 );          \
        sum = vaddq_s32( sum, vaddl_s16( vget_low_s16( vector0 ), vget_high_s16( vector0 ) ) );  \
        sum = vaddq_s32( sum, vaddl_s16( vget_low_s16( vector1 ), vget_high_s16( vector1 ) ) );  \
    }

int32_t y262_satd_8x8_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
    int32_t i_satd;
    int64_t i64_satd;
    int16x8_t v16_d0, v16_d1, v16_d2, v16_d3, v16_d4, v16_d5, v16_d6, v16_d7;
    int16x8_t v16_a0, v16_a1, v16_a2, v16_a3;
    int32x4_t v16_res;
    int32x2_t v8_hadd0;
    int64x1_t v8_hadd1;

    v16_res = vmovq_n_s32( 0 );

    v16_d0 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 0 ), vld1_u8( pui8_blk2 + i_stride2 * 0 ) ) );
    v16_d1 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 1 ), vld1_u8( pui8_blk2 + i_stride2 * 1 ) ) );
    v16_d2 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 2 ), vld1_u8( pui8_blk2 + i_stride2 * 2 ) ) );
    v16_d3 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 3 ), vld1_u8( pui8_blk2 + i_stride2 * 3 ) ) );
    v16_d4 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 4 ), vld1_u8( pui8_blk2 + i_stride2 * 4 ) ) );
    v16_d5 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 5 ), vld1_u8( pui8_blk2 + i_stride2 * 5 ) ) );
    v16_d6 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 6 ), vld1_u8( pui8_blk2 + i_stride2 * 6 ) ) );
    v16_d7 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + i_stride1 * 7 ), vld1_u8( pui8_blk2 + i_stride2 * 7 ) ) );

    HADAMARD_NEON_4x2( v16_d0, v16_d1, v16_d2, v16_d3 );
    HADAMARD_NEON_4x2( v16_d4, v16_d5, v16_d6, v16_d7 );

    ADDSUB( v16_d0, v16_d4 );
    ADDSUB( v16_d1, v16_d5 );
    ADDSUB( v16_d2, v16_d6 );
    ADDSUB( v16_d3, v16_d7 );

    v16_a0 = vcombine_s16( vget_low_s16( v16_d0 ), vget_low_s16( v16_d4 ) );
    v16_a1 = vcombine_s16( vget_high_s16( v16_d0 ), vget_high_s16( v16_d4 ) );
    ADDSUB( v16_a0, v16_a1 );
    ABSADDL( v16_res, v16_a0, v16_a1 );

    v16_a0 = vcombine_s16( vget_low_s16( v16_d1 ), vget_low_s16( v16_d5 ) );
    v16_a1 = vcombine_s16( vget_high_s16( v16_d1 ), vget_high_s16( v16_d5 ) );
    ADDSUB( v16_a0, v16_a1 );
    ABSADDL( v16_res, v16_a0, v16_a1 );

    v16_a0 = vcombine_s16( vget_low_s16( v16_d2 ), vget_low_s16( v16_d6 ) );
    v16_a1 = vcombine_s16( vget_high_s16( v16_d2 ), vget_high_s16( v16_d6 ) );
    ADDSUB( v16_a0, v16_a1 );
    ABSADDL( v16_res, v16_a0, v16_a1 );

    v16_a0 = vcombine_s16( vget_low_s16( v16_d3 ), vget_low_s16( v16_d7 ) );
    v16_a1 = vcombine_s16( vget_high_s16( v16_d3 ), vget_high_s16( v16_d7 ) );
    ADDSUB( v16_a0, v16_a1 );
    ABSADDL( v16_res, v16_a0, v16_a1 );

    v8_hadd0 = vadd_s32( vget_low_s32( v16_res ), vget_high_s32( v16_res ) );
    v8_hadd1 = vpaddl_s32( v8_hadd0 );

    i64_satd = vget_lane_s64( v8_hadd1, 0 );
    i_satd = ( int32_t )i64_satd;

    i_satd = ( i_satd + 2 ) >> 2;

    return i_satd;
}


int32_t y262_satd_16x16_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_satd;

	i_satd  = y262_satd_8x8_neon( pui8_blk1,                         i_stride1, pui8_blk2, i_stride2 );
	i_satd += y262_satd_8x8_neon( pui8_blk1 + 8,                     i_stride1, pui8_blk2 + 8, i_stride2 );
	i_satd += y262_satd_8x8_neon( pui8_blk1 + ( 8 * i_stride1 ),     i_stride1, pui8_blk2 + ( 8 * i_stride2 ), i_stride2 );
	i_satd += y262_satd_8x8_neon( pui8_blk1 + 8 + ( 8 * i_stride1 ), i_stride1, pui8_blk2 + 8 + ( 8 * i_stride2 ), i_stride2 );

	return i_satd;
}


int32_t y262_satd_16x8_neon( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_satd;

	i_satd  = y262_satd_8x8_neon( pui8_blk1,                         i_stride1, pui8_blk2, i_stride2 );
	i_satd += y262_satd_8x8_neon( pui8_blk1 + 8,                     i_stride1, pui8_blk2 + 8, i_stride2 );

	return i_satd;
}



int32_t y262_ssd_8x8_neon( uint8_t *pui8_blk1, int32_t i_blk1_stride, uint8_t *pui8_blk2, int32_t i_blk2_stride )
{
    int32_t i_ssd;
    int16x8_t v16_d0, v16_d1, v16_d2, v16_d3;
    int32x4_t v16_ssd0, v16_ssd1;
    int32x2_t v8_hadd0;
    int64x1_t v8_hadd1;

    v16_d0 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 0 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 0 * i_blk2_stride ) ) ) );
    v16_d1 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 1 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 1 * i_blk2_stride ) ) ) );
    v16_d2 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 2 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 2 * i_blk2_stride ) ) ) );
    v16_d3 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 3 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 3 * i_blk2_stride ) ) ) );

    v16_ssd0 = vmull_s16( vget_low_s16( v16_d0 ), vget_low_s16( v16_d0 ) );
    v16_ssd1 = vmull_s16( vget_high_s16( v16_d0 ), vget_high_s16( v16_d0 ) );

    v16_ssd0 = vmlal_s16( v16_ssd0, vget_low_s16( v16_d1 ), vget_low_s16( v16_d1 ) );
    v16_ssd1 = vmlal_s16( v16_ssd1, vget_high_s16( v16_d1 ), vget_high_s16( v16_d1 ) );

    v16_ssd0 = vmlal_s16( v16_ssd0, vget_low_s16( v16_d2 ), vget_low_s16( v16_d2 ) );
    v16_ssd1 = vmlal_s16( v16_ssd1, vget_high_s16( v16_d2 ), vget_high_s16( v16_d2 ) );

    v16_ssd0 = vmlal_s16( v16_ssd0, vget_low_s16( v16_d3 ), vget_low_s16( v16_d3 ) );
    v16_ssd1 = vmlal_s16( v16_ssd1, vget_high_s16( v16_d3 ), vget_high_s16( v16_d3 ) );

    v16_d0 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 4 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 4 * i_blk2_stride ) ) ) );
    v16_d1 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 5 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 5 * i_blk2_stride ) ) ) );
    v16_d2 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 6 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 6 * i_blk2_stride ) ) ) );
    v16_d3 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_blk1 + ( 7 * i_blk1_stride ) ), vld1_u8( pui8_blk2 + ( 7 * i_blk2_stride ) ) ) );

    v16_ssd0 = vmlal_s16( v16_ssd0, vget_low_s16( v16_d0 ), vget_low_s16( v16_d0 ) );
    v16_ssd1 = vmlal_s16( v16_ssd1, vget_high_s16( v16_d0 ), vget_high_s16( v16_d0 ) );

    v16_ssd0 = vmlal_s16( v16_ssd0, vget_low_s16( v16_d1 ), vget_low_s16( v16_d1 ) );
    v16_ssd1 = vmlal_s16( v16_ssd1, vget_high_s16( v16_d1 ), vget_high_s16( v16_d1 ) );

    v16_ssd0 = vmlal_s16( v16_ssd0, vget_low_s16( v16_d2 ), vget_low_s16( v16_d2 ) );
    v16_ssd1 = vmlal_s16( v16_ssd1, vget_high_s16( v16_d2 ), vget_high_s16( v16_d2 ) );

    v16_ssd0 = vmlal_s16( v16_ssd0, vget_low_s16( v16_d3 ), vget_low_s16( v16_d3 ) );
    v16_ssd1 = vmlal_s16( v16_ssd1, vget_high_s16( v16_d3 ), vget_high_s16( v16_d3 ) );

    v16_ssd0 = vaddq_s32( v16_ssd0, v16_ssd1 );

    v8_hadd0 = vadd_s32( vget_low_s32( v16_ssd0 ), vget_high_s32( v16_ssd0 ) );
    v8_hadd0 = vpadd_s32( v8_hadd0, v8_hadd0 );

    i_ssd = vget_lane_s32( v8_hadd0, 0 );

	return i_ssd;
}

int32_t y262_ssd_16x16_neon( uint8_t *pui8_blk1, int32_t i_blk1_stride, uint8_t *pui8_blk2, int32_t i_blk2_stride )
{
	int32_t i_ssd;

	i_ssd = y262_ssd_8x8_neon( pui8_blk1,                              i_blk1_stride, pui8_blk2,                            i_blk2_stride );
	i_ssd += y262_ssd_8x8_neon( pui8_blk1 + 8,                         i_blk1_stride, pui8_blk2 + 8,                        i_blk2_stride );
	i_ssd += y262_ssd_8x8_neon( pui8_blk1 + ( 8 * i_blk1_stride ),     i_blk1_stride, pui8_blk2 + ( 8 * i_blk2_stride ),    i_blk2_stride );
	i_ssd += y262_ssd_8x8_neon( pui8_blk1 + 8 + ( 8 * i_blk1_stride ), i_blk1_stride, pui8_blk2 + 8 + ( 8 * i_blk2_stride), i_blk2_stride );
	return i_ssd;
}


void y262_sub_8x8_neon( int16_t *pi16_diff, uint8_t *pui8_src1, int32_t i_stride_src1, uint8_t *pui8_src2, int32_t i_stride_src2 )
{
    int16x8_t v16_d0, v16_d1, v16_d2, v16_d3;

    v16_d0 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 0 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 0 * i_stride_src2 ) ) ) );
    v16_d1 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 1 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 1 * i_stride_src2 ) ) ) );
    v16_d2 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 2 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 2 * i_stride_src2 ) ) ) );
    v16_d3 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 3 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 3 * i_stride_src2 ) ) ) );

    vst1q_s16( pi16_diff +  0, v16_d0 );
    vst1q_s16( pi16_diff +  8, v16_d1 );
    vst1q_s16( pi16_diff + 16, v16_d2 );
    vst1q_s16( pi16_diff + 24, v16_d3 );

    v16_d0 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 4 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 4 * i_stride_src2 ) ) ) );
    v16_d1 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 5 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 5 * i_stride_src2 ) ) ) );
    v16_d2 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 6 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 6 * i_stride_src2 ) ) ) );
    v16_d3 = vreinterpretq_s16_u16( vsubl_u8( vld1_u8( pui8_src1 + ( 7 * i_stride_src1 ) ), vld1_u8( pui8_src2 + ( 7 * i_stride_src2 ) ) ) );

    vst1q_s16( pi16_diff + 32, v16_d0 );
    vst1q_s16( pi16_diff + 40, v16_d1 );
    vst1q_s16( pi16_diff + 48, v16_d2 );
    vst1q_s16( pi16_diff + 56, v16_d3 );
}

void y262_add_8x8_neon( uint8_t *pui8_destination, int32_t i_destination_stride, uint8_t *pui8_base, int32_t i_base_stride, int16_t *pi16_difference )
{
    int32_t i_y;

    int16x8_t v16_zero = vmovq_n_s16( 0 );

    for( i_y = 0; i_y < 8; i_y += 2 )
    {
        int16x8_t v16_d0, v16_b0, v16_d1, v16_b1;

        v16_d0 = vld1q_s16( pi16_difference + ( i_y * 8 ) );
        v16_d1 = vld1q_s16( pi16_difference + ( ( i_y + 1 ) * 8 ) );
        v16_b0 = vreinterpretq_s16_u16( vshll_n_u8( vld1_u8( pui8_base + ( i_y * i_base_stride ) ), 0 ) );
        v16_b1 = vreinterpretq_s16_u16( vshll_n_u8( vld1_u8( pui8_base + ( ( i_y + 1 ) * i_base_stride ) ), 0 ) );

        v16_d0 = vaddq_s16( v16_d0, v16_b0 );
        v16_d1 = vaddq_s16( v16_d1, v16_b1 );

        v16_d0 = vmaxq_s16( v16_zero, v16_d0 );
        v16_d1 = vmaxq_s16( v16_zero, v16_d1 );

        vst1_u8( pui8_destination + ( i_y * i_destination_stride ), vqmovn_u16( vreinterpretq_u16_s16( v16_d0 ) ) );
        vst1_u8( pui8_destination + ( ( i_y + 1 ) * i_destination_stride ), vqmovn_u16( vreinterpretq_u16_s16( v16_d1 ) ) );
    }
}


/* MC */

#define MC_FUNC_NEON( name, i_width, i_height, hpelidx )	\
void y262_motcomp_##name##_put_neon( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )		\
{														\
	int32_t i_x, i_y;									\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a;                         \
                v8_a = vld1_u8( pui8_src );             \
                vst1_u8( pui8_dst, v8_a );              \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a;                       \
                v16_a = vld1q_u8( pui8_src );           \
                vst1q_u8( pui8_dst, v16_a );            \
            }                                           \
			pui8_src += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 1 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b;                   \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_b ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b;                \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_b ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 2 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + i_src_stride;			\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b;                   \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_b ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b;                \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_b ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;		\
        uint8x8_t v8_a, v8_b, v8_c, v8_d;               \
        uint8x16_t v16_a, v16_b, v16_c, v16_d;          \
        uint8x8_t v8_one = vmov_n_u8( 1 );              \
        uint8x16_t v16_one = vmovq_n_u8( 1 );           \
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
        if( i_width == 8 )                              \
        {                                               \
            v8_a = vld1_u8( pui8_src1 );                \
            v8_b = vld1_u8( pui8_src2 );                \
        }                                               \
        else if( i_width == 16 )                        \
        {                                               \
            v16_a = vld1q_u8( pui8_src1 );              \
            v16_b = vld1q_u8( pui8_src2 );              \
        }                                               \
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_carry0, v8_carry1;         \
                v8_c = vld1_u8( pui8_src3 );            \
                v8_d = vld1_u8( pui8_src4 );            \
                                                        \
                v8_carry0 = veor_u8( v8_a, v8_c);       \
                v8_carry1 = veor_u8( v8_b, v8_d);       \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_c );         \
                v8_b = vrhadd_u8( v8_b, v8_d );         \
                v8_carry0 = vorr_u8( v8_carry0, v8_carry1 ); \
                                                        \
                v8_carry1 = veor_u8( v8_a, v8_b);       \
                v8_carry0 = vand_u8( v8_carry0, v8_carry1 );    \
                v8_carry0 = vand_u8( v8_carry0, v8_one );   \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                v8_a = vsub_u8( v8_a, v8_carry0 );      \
                                                        \
                vst1_u8( pui8_dst, v8_a );              \
                                                        \
                v8_a = v8_c;                            \
                v8_b = v8_d;                            \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_carry0, v16_carry1;         \
                v16_c = vld1q_u8( pui8_src3 );            \
                v16_d = vld1q_u8( pui8_src4 );            \
                                                        \
                v16_carry0 = veorq_u8( v16_a, v16_c);       \
                v16_carry1 = veorq_u8( v16_b, v16_d);       \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_c );         \
                v16_b = vrhaddq_u8( v16_b, v16_d );         \
                v16_carry0 = vorrq_u8( v16_carry0, v16_carry1 ); \
                                                        \
                v16_carry1 = veorq_u8( v16_a, v16_b);       \
                v16_carry0 = vandq_u8( v16_carry0, v16_carry1 );    \
                v16_carry0 = vandq_u8( v16_carry0, v16_one );   \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_b );         \
                v16_a = vsubq_u8( v16_a, v16_carry0 );      \
                                                        \
                vst1q_u8( pui8_dst, v16_a );              \
                                                        \
                v16_a = v16_c;                            \
                v16_b = v16_d;                            \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_src3 += i_src_stride;					\
			pui8_src4 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
}														\
														\
														\
void y262_motcomp_##name##_avg_neon( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )	\
{														\
	int32_t i_x, i_y;									\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_z;                   \
                v8_a = vld1_u8( pui8_src );             \
                v8_z = vld1_u8( pui8_dst );             \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_z;                \
                v16_a = vld1q_u8( pui8_src );           \
                v16_z = vld1q_u8( pui8_dst );           \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
            }                                           \
			pui8_src += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 1 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b, v8_z;             \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                v8_z = vld1_u8( pui8_dst );             \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b, v16_z;         \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                v16_z = vld1q_u8( pui8_dst );           \
                v16_a = vrhaddq_u8( v16_a, v16_b );     \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 2 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + i_src_stride;			\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b, v8_z;             \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                v8_z = vld1_u8( pui8_dst );             \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b, v16_z;         \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                v16_z = vld1q_u8( pui8_dst );           \
                v16_a = vrhaddq_u8( v16_a, v16_b );     \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;		\
        uint8x8_t v8_a, v8_b, v8_c, v8_d, v8_z;         \
        uint8x16_t v16_a, v16_b, v16_c, v16_d, v16_z;   \
        uint8x8_t v8_one = vmov_n_u8( 1 );              \
        uint8x16_t v16_one = vmovq_n_u8( 1 );           \
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
        if( i_width == 8 )                              \
        {                                               \
            v8_a = vld1_u8( pui8_src1 );                \
            v8_b = vld1_u8( pui8_src2 );                \
        }                                               \
        else if( i_width == 16 )                        \
        {                                               \
            v16_a = vld1q_u8( pui8_src1 );              \
            v16_b = vld1q_u8( pui8_src2 );              \
        }                                               \
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_carry0, v8_carry1;         \
                v8_c = vld1_u8( pui8_src3 );            \
                v8_d = vld1_u8( pui8_src4 );            \
                v8_z = vld1_u8( pui8_dst );             \
                                                        \
                v8_carry0 = veor_u8( v8_a, v8_c);       \
                v8_carry1 = veor_u8( v8_b, v8_d);       \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_c );         \
                v8_b = vrhadd_u8( v8_b, v8_d );         \
                v8_carry0 = vorr_u8( v8_carry0, v8_carry1 ); \
                                                        \
                v8_carry1 = veor_u8( v8_a, v8_b);       \
                v8_carry0 = vand_u8( v8_carry0, v8_carry1 ); \
                v8_carry0 = vand_u8( v8_carry0, v8_one ); \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                v8_a = vsub_u8( v8_a, v8_carry0 );      \
                                                        \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
                                                        \
                v8_a = v8_c;                            \
                v8_b = v8_d;                            \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_carry0, v16_carry1;      \
                v16_c = vld1q_u8( pui8_src3 );          \
                v16_d = vld1q_u8( pui8_src4 );          \
                v16_z = vld1q_u8( pui8_dst );           \
                                                        \
                v16_carry0 = veorq_u8( v16_a, v16_c);   \
                v16_carry1 = veorq_u8( v16_b, v16_d);   \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_c );     \
                v16_b = vrhaddq_u8( v16_b, v16_d );     \
                v16_carry0 = vorrq_u8( v16_carry0, v16_carry1 ); \
                                                        \
                v16_carry1 = veorq_u8( v16_a, v16_b);   \
                v16_carry0 = vandq_u8( v16_carry0, v16_carry1 ); \
                v16_carry0 = vandq_u8( v16_carry0, v16_one ); \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_b );     \
                v16_a = vsubq_u8( v16_a, v16_carry0 );  \
                                                        \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
                                                        \
                v16_a = v16_c;                          \
                v16_b = v16_d;                          \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_src3 += i_src_stride;					\
			pui8_src4 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
}														\


MC_FUNC_NEON( 16x16_00, 16, 16, 0 );
MC_FUNC_NEON( 16x16_01, 16, 16, 1 );
MC_FUNC_NEON( 16x16_10, 16, 16, 2 );
MC_FUNC_NEON( 16x16_11, 16, 16, 3 );

MC_FUNC_NEON( 16x8_00, 16, 8, 0 );
MC_FUNC_NEON( 16x8_01, 16, 8, 1 );
MC_FUNC_NEON( 16x8_10, 16, 8, 2 );
MC_FUNC_NEON( 16x8_11, 16, 8, 3 );

MC_FUNC_NEON( 8x16_00, 8, 16, 0 );
MC_FUNC_NEON( 8x16_01, 8, 16, 1 );
MC_FUNC_NEON( 8x16_10, 8, 16, 2 );
MC_FUNC_NEON( 8x16_11, 8, 16, 3 );

MC_FUNC_NEON( 8x8_00, 8, 8, 0 );
MC_FUNC_NEON( 8x8_01, 8, 8, 1 );
MC_FUNC_NEON( 8x8_10, 8, 8, 2 );
MC_FUNC_NEON( 8x8_11, 8, 8, 3 );

MC_FUNC_NEON( 8x4_00, 8, 4, 0 );
MC_FUNC_NEON( 8x4_01, 8, 4, 1 );
MC_FUNC_NEON( 8x4_10, 8, 4, 2 );
MC_FUNC_NEON( 8x4_11, 8, 4, 3 );







