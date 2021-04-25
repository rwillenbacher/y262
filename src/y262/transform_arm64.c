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

//#ifdef ASSEMBLY_ARM64

#include <arm_neon.h>

#include "y262.h"


#define RND1BITS ( 11 )
#define RND2BITS ( 31 - RND1BITS )


static const int16_t rgi16_y262_fdct_neon_cs1[ 8 ][ 8 ] = {
        { 16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383 },
        { 22724,  19265,  12872,   4520,  -4520, -12872, -19265, -22724 },
        { 21406,   8867,  -8867, -21406, -21406,  -8867,   8867,  21406 },
        { 19265,  -4520, -22724, -12872,  12872,  22724,   4520, -19265 },
        { 16383, -16383, -16383,  16383,  16383, -16383, -16383,  16383 },
        { 12872, -22724,   4520,  19265, -19265,  -4520,  22724, -12872 },
        {  8867, -21406,  21406,  -8867,  -8867,  21406, -21406,   8867 },
        {  4520, -12872,  19265, -22724,  22724, -19265,  12872,  -4520 },
};
static const int16_t rgi16_y262_fdct_neon_cs2[ 32 ] = {
	16385,  16385,  22726,  19266,  -8867, -21408, -22726, -12873,
	16385,  16385,  12873,   4521,  21408,   8867,  19266,  -4521,
	16385, -16385,  12873, -22726,  21408,  -8867,  19266, -22726,
	-16385,  16385,   4521,  19266,   8867, -21408,   4521, -12873,
};


void y262_fdct_neon( int16_t *pi16_block, int16_t *pi16_dst )
{
	int i_i, i_j, i_k;
	int16x8_t rgv16_tmp[ 8 ], rgv16_dsts[ 8 ];
	int16x8_t rgv16_e[ 4 ], rgv16_ee[ 2 ];
	int32x4x2_t rgv32_transA[ 4 ];
	int16x8x2_t rgv32_transB[ 4 ];
	int16x4_t v8_mt0, v8_mt1, v8_mt2, v8_mt3, v8_mt4, v8_mt5, v8_mt6, v8_mt7;

#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )

	for( i_j = 1; i_j < 8; i_j += 2 )
	{
		int16x8_t v16_d;
		int32x4_t v16_s0, v16_s1;

		v16_d = vsubq_s16( vld1q_s16( &pi16_block[ 8 * 0 ] ), vld1q_s16( &pi16_block[ 8 * 7 ] ) );
		v16_s0 = vmull_n_s16( vget_low_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ 0 ] );
		v16_s1 = vmull_n_s16( vget_high_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ 0 ] );

		for( i_k = 1; i_k < 4; i_k++ )
		{
			v16_d = vsubq_s16( vld1q_s16( &pi16_block[ 8 * i_k ] ), vld1q_s16( &pi16_block[ 8 * ( 7 - i_k ) ] ) );
			v16_s0 = vmlal_n_s16( v16_s0, vget_low_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ i_k ] );
			v16_s1 = vmlal_n_s16( v16_s1, vget_high_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ i_k ] );
		}

		rgv16_tmp[ i_j ] = vcombine_s16( vqrshrn_n_s32( v16_s0, RND1BITS ), vqrshrn_n_s32( v16_s1, RND1BITS ) );
	}

	for ( i_k = 0; i_k < 4; i_k++ )
	{
		rgv16_e[ i_k ] = vaddq_s16( vld1q_s16( &pi16_block[ 8 * i_k ] ), vld1q_s16( &pi16_block[ 8 * ( 7 - i_k ) ] ) );
	}

	for( i_j = 2; i_j < 8; i_j += 4 )
	{
		int16x8_t v16_d;
		int32x4_t v16_s0, v16_s1;

		v16_d = vsubq_s16( rgv16_e[ 0 ], rgv16_e[ 3 ] );
		v16_s0 = vmull_n_s16( vget_low_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ 0 ] );
		v16_s1 = vmull_n_s16( vget_high_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ 0 ] );

		v16_d = vsubq_s16( rgv16_e[ 1 ], rgv16_e[ 2 ] );
		v16_s0 = vmlal_n_s16( v16_s0, vget_low_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ 1 ] );
		v16_s1 = vmlal_n_s16( v16_s1, vget_high_s16( v16_d ), rgi16_y262_fdct_neon_cs1[ i_j ][ 1 ] );

		rgv16_tmp[ i_j ] = vcombine_s16( vqrshrn_n_s32( v16_s0, RND1BITS ), vqrshrn_n_s32( v16_s1, RND1BITS ) );
	}
	for ( i_k = 0; i_k < 2; i_k++ )
	{
		rgv16_ee[ i_k ] = vaddq_s16( rgv16_e[ i_k ], rgv16_e[ 3 - i_k ] );
	}
	for( i_j = 0; i_j < 8; i_j += 4 )
	{
		int16x8_t v16_d;
		int32x4_t v16_s0, v16_s1;

		v16_s0 = vmull_n_s16( vget_low_s16( rgv16_ee[ 0 ] ), rgi16_y262_fdct_neon_cs1[ i_j ][ 0 ] );
		v16_s1 = vmull_n_s16( vget_high_s16( rgv16_ee[ 0 ] ), rgi16_y262_fdct_neon_cs1[ i_j ][ 0 ] );

		v16_s0 = vmlal_n_s16( v16_s0, vget_low_s16( rgv16_ee[ 1 ] ), rgi16_y262_fdct_neon_cs1[ i_j ][ 1 ] );
		v16_s1 = vmlal_n_s16( v16_s1, vget_high_s16( rgv16_ee[ 1 ] ), rgi16_y262_fdct_neon_cs1[ i_j ][ 1 ] );

		rgv16_tmp[ i_j ] = vcombine_s16( vqrshrn_n_s32( v16_s0, RND1BITS ), vqrshrn_n_s32( v16_s1, RND1BITS ) );
    }

	v8_mt0 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 0 ] );
	v8_mt1 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 4 ] );
	v8_mt2 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 8 ] );
	v8_mt3 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 12 ] );
	v8_mt4 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 16 ] );
	v8_mt5 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 20 ] );
	v8_mt6 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 24 ] );
	v8_mt7 = vld1_s16( &rgi16_y262_fdct_neon_cs2[ 28 ] );

	for( i_j = 0; i_j < 8; i_j++ )
	{
		int16x4_t v8_l0, v8_l1, v8_o, v8_e, v8_m0, v8_m1, v8_m2, v8_m3;
		int16x4x2_t v16_trn0;
		int32x2x2_t v16_trn1;
		int32x4_t v16_s0, v16_s1, v16_s2, v16_s3;
		int16x8_t v16_row;

		v8_l0 = vget_low_s16( rgv16_tmp[ i_j ] );
		v8_l1 = vget_high_s16( rgv16_tmp[ i_j ] );
		v8_l1 = vrev64_s16( v8_l1 );
		v8_o = vsub_s16( v8_l0, v8_l1 );
		v8_e = vadd_s16( v8_l0, v8_l1 );

		v16_trn1 = vzip_s32( vreinterpret_s32_s16( v8_e ), vreinterpret_s32_s16( v8_o ) );
		v8_m0 = vreinterpret_s16_s32( v16_trn1.val[ 0 ] );
		v8_m1 = vreinterpret_s16_s32( v16_trn1.val[ 1 ] );

		v16_s0 = vmull_s16( v8_m0, v8_mt0 );
		v16_s1 = vmull_s16( v8_m1, v8_mt1 );
		v16_s0 = vmlal_s16( v16_s0, v8_m1, v8_mt2 );
		v16_s1 = vmlal_s16( v16_s1, v8_m0, v8_mt3 );

		v16_s2 = vmull_s16( v8_m0, v8_mt4 );
		v16_s3 = vmull_s16( v8_m1, v8_mt5 );
		v16_s2 = vmlal_s16( v16_s2, v8_m1, v8_mt6 );
		v16_s3 = vmlal_s16( v16_s3, v8_m0, v8_mt7 );

		v16_s0 = vpaddq_s32( v16_s0, v16_s1 );
		v16_s1 = vpaddq_s32( v16_s2, v16_s3 );

		v16_row = vcombine_s16( vmovn_s32( vrshrq_n_s32( v16_s0, RND2BITS ) ), vmovn_s32(vrshrq_n_s32( v16_s1, RND2BITS ) ) );
		vst1q_s16( pi16_dst + ( 8 * i_j ), v16_row );
	}
}


#define RND1BITS ( 11 )
#define RND2BITS ( 31 - RND1BITS )

static const int16_t rgi16_y262_idct_cs1[ 8 ][ 8 ] = {
        { 16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383 },
        { 22724,  19265,  12872,   4520,  -4520, -12872, -19265, -22724 },
        { 21406,   8867,  -8867, -21406, -21406,  -8867,   8867,  21406 },
        { 19265,  -4520, -22724, -12872,  12872,  22724,   4520, -19265 },
        { 16383, -16383, -16383,  16383,  16383, -16383, -16383,  16383 },
        { 12872, -22724,   4520,  19265, -19265,  -4520,  22724, -12872 },
        {  8867, -21406,  21406,  -8867,  -8867,  21406, -21406,   8867 },
        {  4520, -12872,  19265, -22724,  22724, -19265,  12872,  -4520 },
};
static const int16_t rgi16_y262_idct_cs2[ 8 ][ 8 ] = {
        { 16385,  16385,  16385,  16385,  16385,  16385,  16385,  16385 },
        { 22726,  19266,  12873,   4521,  -4521, -12873, -19266, -22726 },
        { 21408,   8867,  -8867, -21408, -21408,  -8867,   8867,  21408 },
        { 19266,  -4521, -22726, -12873,  12873,  22726,   4521, -19266 },
        { 16385, -16385, -16385,  16385,  16385, -16385, -16385,  16385 },
        { 12873, -22726,   4521,  19266, -19266,  -4521,  22726, -12873 },
        {  8867, -21408,  21408,  -8867,  -8867,  21408, -21408,   8867 },
        {  4521, -12873,  19266, -22726,  22726, -19266,  12873,  -4521 },
};

static const int16_t rgi16_y262_idct_neon_cs2[ 32 ] = {
	16385,  21408,  16385,   8867,  16385,  -8867,  16385, -21408,
	16385,   8867, -16385, -21408, -16385,  21408,  16385,  -8867,
	22726,  19266,  19266,  -4521,  12873, -22726,   4521, -12873,
	12873,   4521, -22726, -12873,   4521,  19266,  19266, -22726
};



void y262_idct_neon( int16_t *pi16_block, int16_t *pi16_dst )
{
	int i_j, i_k;
	int16_t rgi16_tmp[ 64 ];
	int32_t rgi_e[ 4 ], rgi_o[ 4 ];
	int32_t rgi_ee[ 2 ], rgi_eo[ 2 ];
	int32x4_t rgv16_o[ 4 ];
	int32x4_t rgv16_eo[ 4 ];
	int32x4_t rgv16_ee[ 4 ];
	int32x4_t rgv16_e[ 4 ];
	int16x4_t rgv8_tmp[ 8 ][ 2 ];
	int16x4_t v8_mt0, v8_mt1, v8_mt2, v8_mt3, v8_mt4, v8_mt5, v8_mt6, v8_mt7;

#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )


	for( i_j = 0; i_j < 2; i_j++ )
	{
		int16x4_t v8_b0, v8_b1, v8_b2, v8_b3;

		v8_b0 = vld1_s16( pi16_block + ( 8 * 1 ) + ( i_j * 4 ) );
		v8_b1 = vld1_s16( pi16_block + ( 8 * 3 ) + ( i_j * 4 ) );
		v8_b2 = vld1_s16( pi16_block + ( 8 * 5 ) + ( i_j * 4 ) );
		v8_b3 = vld1_s16( pi16_block + ( 8 * 7 ) + ( i_j * 4 ) );

		rgv16_o[ 0 ] = vmull_n_s16( v8_b0, rgi16_y262_idct_cs1[ 1 ][ 0 ] );
		rgv16_o[ 0 ] = vmlal_n_s16( rgv16_o[ 0 ], v8_b1, rgi16_y262_idct_cs1[ 3 ][ 0 ] );
		rgv16_o[ 0 ] = vmlal_n_s16( rgv16_o[ 0 ], v8_b2, rgi16_y262_idct_cs1[ 5 ][ 0 ] );
		rgv16_o[ 0 ] = vmlal_n_s16( rgv16_o[ 0 ], v8_b3, rgi16_y262_idct_cs1[ 7 ][ 0 ] );

		rgv16_o[ 1 ] = vmull_n_s16( v8_b0, rgi16_y262_idct_cs1[ 1 ][ 1 ] );
		rgv16_o[ 1 ] = vmlal_n_s16( rgv16_o[ 1 ], v8_b1, rgi16_y262_idct_cs1[ 3 ][ 1 ] );
		rgv16_o[ 1 ] = vmlal_n_s16( rgv16_o[ 1 ], v8_b2, rgi16_y262_idct_cs1[ 5 ][ 1 ] );
		rgv16_o[ 1 ] = vmlal_n_s16( rgv16_o[ 1 ], v8_b3, rgi16_y262_idct_cs1[ 7 ][ 1 ] );

		rgv16_o[ 2 ] = vmull_n_s16( v8_b0, rgi16_y262_idct_cs1[ 1 ][ 2 ] );
		rgv16_o[ 2 ] = vmlal_n_s16( rgv16_o[ 2 ], v8_b1, rgi16_y262_idct_cs1[ 3 ][ 2 ] );
		rgv16_o[ 2 ] = vmlal_n_s16( rgv16_o[ 2 ], v8_b2, rgi16_y262_idct_cs1[ 5 ][ 2 ] );
		rgv16_o[ 2 ] = vmlal_n_s16( rgv16_o[ 2 ], v8_b3, rgi16_y262_idct_cs1[ 7 ][ 2 ] );

		rgv16_o[ 3 ] = vmull_n_s16( v8_b0, rgi16_y262_idct_cs1[ 1 ][ 3 ] );
		rgv16_o[ 3 ] = vmlal_n_s16( rgv16_o[ 3 ], v8_b1, rgi16_y262_idct_cs1[ 3 ][ 3 ] );
		rgv16_o[ 3 ] = vmlal_n_s16( rgv16_o[ 3 ], v8_b2, rgi16_y262_idct_cs1[ 5 ][ 3 ] );
		rgv16_o[ 3 ] = vmlal_n_s16( rgv16_o[ 3 ], v8_b3, rgi16_y262_idct_cs1[ 7 ][ 3 ] );

		v8_b0 = vld1_s16( pi16_block + ( 8 * 2 ) + ( i_j * 4 ) );
		v8_b1 = vld1_s16( pi16_block + ( 8 * 6 ) + ( i_j * 4 ) );
		v8_b2 = vld1_s16( pi16_block + ( 8 * 0 ) + ( i_j * 4 ) );
		v8_b3 = vld1_s16( pi16_block + ( 8 * 4 ) + ( i_j * 4 ) );

		rgv16_eo[ 0 ] = vmull_n_s16( v8_b0, rgi16_y262_idct_cs1[ 2 ][ 0 ] );
		rgv16_eo[ 0 ] = vmlal_n_s16( rgv16_eo[ 0 ], v8_b1, rgi16_y262_idct_cs1[ 6 ][ 0 ] );
		rgv16_eo[ 1 ] = vmull_n_s16( v8_b0, rgi16_y262_idct_cs1[ 2 ][ 1 ] );
		rgv16_eo[ 1 ] = vmlal_n_s16( rgv16_eo[ 1 ], v8_b1, rgi16_y262_idct_cs1[ 6 ][ 1 ] );
		rgv16_ee[ 0 ] = vmull_n_s16( v8_b2, rgi16_y262_idct_cs1[ 0 ][ 0 ] );
		rgv16_ee[ 0 ] = vmlal_n_s16( rgv16_ee[ 0 ], v8_b3, rgi16_y262_idct_cs1[ 4 ][ 0 ] );
		rgv16_ee[ 1 ] = vmull_n_s16( v8_b2, rgi16_y262_idct_cs1[ 0 ][ 1 ] );
		rgv16_ee[ 1 ] = vmlal_n_s16( rgv16_ee[ 1 ], v8_b3, rgi16_y262_idct_cs1[ 4 ][ 1 ] );

		rgv16_e[ 0 ] = vaddq_s32( rgv16_ee[ 0 ], rgv16_eo[ 0 ] );
		rgv16_e[ 1 ] = vaddq_s32( rgv16_ee[ 1 ], rgv16_eo[ 1 ] );
		rgv16_e[ 2 ] = vsubq_s32( rgv16_ee[ 1 ], rgv16_eo[ 1 ] );
		rgv16_e[ 3 ] = vsubq_s32( rgv16_ee[ 0 ], rgv16_eo[ 0 ] );


		for( i_k = 0; i_k < 4; i_k++ )
		{
			int32x4_t v16_eoa, v16_eos;
			v16_eoa = vaddq_s32( rgv16_e[ i_k ], rgv16_o[ i_k ]);
			rgv8_tmp[ i_k ][ i_j ] = vqrshrn_n_s32( v16_eoa, RND1BITS );
			v16_eos = vsubq_s32( rgv16_e[ 3 - i_k ], rgv16_o[ 3 - i_k ]);
			rgv8_tmp[ i_k + 4 ][ i_j ] = vqrshrn_n_s32( v16_eos, RND1BITS );
		}
	}

	v8_mt0 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 0 ] );
	v8_mt1 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 4 ] );
	v8_mt2 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 8 ] );
	v8_mt3 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 12 ] );
	v8_mt4 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 16 ] );
	v8_mt5 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 20 ] );
	v8_mt6 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 24 ] );
	v8_mt7 = vld1_s16( &rgi16_y262_idct_neon_cs2[ 28 ] );

	for( i_j = 0; i_j < 8; i_j++ )
	{
		int16x4_t v8_l02, v8_l46, v8_l13, v8_l57, v8_m0, v8_m1, v8_m2, v8_m3, v8_m4, v8_m5, v8_m6, v8_m7;
		int16x4x2_t v16_trn0;
		int32x2x2_t v16_trn1;
		int32x4_t v16_s0, v16_s1, v16_s2, v16_s3, v16_s4, v16_s5, v16_s6, v16_s7, v16_e, v16_o;
		int16x8_t v16_row;

		v8_m0 = rgv8_tmp[ i_j ][ 0 ];
		v8_m1 = rgv8_tmp[ i_j ][ 1 ];

		v16_trn1 = vtrn_s32( vreinterpret_s32_s16( v8_m0 ), vreinterpret_s32_s16( v8_m1 ) );
		v16_trn0 = vzip_s16( vreinterpret_s16_s32( v16_trn1.val[ 0 ] ),vreinterpret_s16_s32( v16_trn1.val[ 1 ] ) );

		v16_trn1 = vtrn_s32( vreinterpret_s32_s16( v16_trn0.val[ 0 ] ), vreinterpret_s32_s16( v16_trn0.val[ 0 ] ) );
		v8_l02 = vreinterpret_s16_s32( v16_trn1.val[ 0 ] );
		v8_l13 = vreinterpret_s16_s32( v16_trn1.val[ 1 ] );

		v16_trn1 = vtrn_s32( vreinterpret_s32_s16( v16_trn0.val[ 1 ] ), vreinterpret_s32_s16( v16_trn0.val[ 1 ] ) );
		v8_l46 = vreinterpret_s16_s32( v16_trn1.val[ 0 ] );
		v8_l57 = vreinterpret_s16_s32( v16_trn1.val[ 1 ] );

		v16_s0 = vmull_s16( v8_l02, v8_mt0 );
		v16_s0 = vmlal_s16( v16_s0, v8_l46, v8_mt2 );
		v16_s1 = vmull_s16( v8_l02, v8_mt1 );
		v16_s1 = vmlal_s16( v16_s1, v8_l46, v8_mt3 );
		v16_s2 = vmull_s16( v8_l13, v8_mt4 );
		v16_s2 = vmlal_s16( v16_s2, v8_l57, v8_mt6 );
		v16_s3 = vmull_s16( v8_l13, v8_mt5 );
		v16_s3 = vmlal_s16( v16_s3, v8_l57, v8_mt7 );

		v16_s0 = vpaddq_s32( v16_s0, v16_s1 );
		v16_s1 = vpaddq_s32( v16_s2, v16_s3 );

		v16_e = vaddq_s32( v16_s0, v16_s1 );
		v16_o = vsubq_s32( v16_s0, v16_s1 );
		v16_o = vcombine_s32( vrev64_s32( vget_high_s32( v16_o ) ), vrev64_s32( vget_low_s32( v16_o ) ) );

		v16_row = vcombine_s16( vmovn_s32( vrshrq_n_s32( v16_e, RND2BITS ) ), vmovn_s32(vrshrq_n_s32( v16_o, RND2BITS ) ) );
		vst1q_s16( pi16_dst + ( 8 * i_j ), v16_row );
	}
}


int32_t y262_quant8x8_intra_fw_mpeg2_neon( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias )
{
	int32_t i_y, i_x, i_qm, i_nz, i_intra_dc;
	int16x8_t v16_zero, v16_nz, v16_2047, v16_m2048;

	v16_nz = v16_zero = vmovq_n_s16( 0 );
	v16_2047 = vmovq_n_s16( 2047 );
	v16_m2048 = vmovq_n_s16( -2048 );

	i_intra_dc = pi_coeffs[ 0 ];
	pi_coeffs[ 0 ] = 0;

	i_nz = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		int16x8_t v16_co;
		int16x8_t v16_mask;
		uint16x8_t v16_qm, v16_bias, v16_cou;
		uint32x4_t v16_res0, v16_res1;

		v16_co = vld1q_s16( pi_coeffs + ( i_y * 8 ) );
		v16_qm = vld1q_u16( pui16_qmat + ( i_y * 8 ) );
		v16_bias = vld1q_u16( pui16_bias + ( i_y * 8 ) );
		v16_mask = vreinterpretq_s16_u16( vcgtq_s16( v16_zero, v16_co ) );
		v16_co = veorq_s16( v16_co, v16_mask );
		v16_co = vsubq_s16( v16_co, v16_mask );

		v16_cou = vaddq_u16( vreinterpretq_u16_s16( v16_co ), v16_bias );
		v16_res0 = vmull_u16( vget_low_u16( v16_cou ), vget_low_u16( v16_qm ) );
		v16_res1 = vmull_u16( vget_high_u16( v16_cou ), vget_high_u16( v16_qm ) );

		v16_co = vreinterpretq_s16_u16( vcombine_u16( vshrn_n_u32( v16_res0, 16 ), vshrn_n_u32( v16_res1, 16 ) ) );

		v16_co = veorq_s16( v16_co, v16_mask );
		v16_co = vsubq_s16( v16_co, v16_mask );

		v16_co = vminq_s16( v16_co, v16_2047 );
		v16_co = vmaxq_s16( v16_co, v16_m2048 );

		vst1q_s16( pi_coeffs + ( i_y * 8 ), v16_co );

		v16_nz = vorrq_s16( v16_co, v16_nz );
	}

	v16_nz = vnegq_s16( vreinterpretq_s16_u16( vceqzq_s16( v16_nz ) ) );
	i_nz = vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 0 );
	i_nz |= vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 1 );
	i_nz |= vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 2 );
	i_nz |= vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 3 );

	pi_coeffs[ 0 ] = i_intra_dc;

	return i_nz;
}



int32_t y262_quant8x8_inter_fw_mpeg2_neon( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat )
{
	int32_t i_y, i_x, i_qm, i_nz;
	int16x8_t v16_zero, v16_nz, v16_2047, v16_m2048;

	v16_nz = v16_zero = vmovq_n_s16( 0 );
	v16_2047 = vmovq_n_s16( 2047 );
	v16_m2048 = vmovq_n_s16( -2048 );

	i_nz = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		int16x8_t v16_co;
		int16x8_t v16_mask;
		uint16x8_t v16_qm;
		uint32x4_t v16_res0, v16_res1;

		v16_co = vld1q_s16( pi_coeffs + ( i_y * 8 ) );
		v16_qm = vld1q_u16( pui16_qmat + ( i_y * 8 ) );
		v16_mask = vreinterpretq_s16_u16( vcgtq_s16( v16_zero, v16_co ) );
		v16_co = veorq_s16( v16_co, v16_mask );
		v16_co = vsubq_s16( v16_co, v16_mask );

		v16_res0 = vmull_u16( vreinterpret_u16_s16( vget_low_s16( v16_co ) ), vget_low_u16( v16_qm ) );
		v16_res1 = vmull_u16( vreinterpret_u16_s16( vget_high_s16( v16_co ) ), vget_high_u16( v16_qm ) );

		v16_co = vreinterpretq_s16_u16( vcombine_u16( vshrn_n_u32( v16_res0, 16 ), vshrn_n_u32( v16_res1, 16 ) ) );

		v16_co = veorq_s16( v16_co, v16_mask );
		v16_co = vsubq_s16( v16_co, v16_mask );

		v16_co = vminq_s16( v16_co, v16_2047 );
		v16_co = vmaxq_s16( v16_co, v16_m2048 );

		vst1q_s16( pi_coeffs + ( i_y * 8 ), v16_co );

		v16_nz = vorrq_s16( v16_co, v16_nz );
	}

	v16_nz = vnegq_s16( vreinterpretq_s16_u16( vceqzq_s16( v16_nz ) ) );
	i_nz = vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 0 );
	i_nz |= vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 1 );
	i_nz |= vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 2 );
	i_nz |= vgetq_lane_u32( vreinterpretq_u32_s16( v16_nz ), 3 );

	return i_nz;
}



//#endif