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

#include "y262.h"

#define RND1BITS ( 11 )
#define RND2BITS ( 31 - RND1BITS )

static const int16_t rgi16_y262_fdct_cs1[ 8 ][ 8 ] = {
        { 16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383 },
        { 22724,  19265,  12872,   4520,  -4520, -12872, -19265, -22724 },
        { 21406,   8867,  -8867, -21406, -21406,  -8867,   8867,  21406 },
        { 19265,  -4520, -22724, -12872,  12872,  22724,   4520, -19265 },
        { 16383, -16383, -16383,  16383,  16383, -16383, -16383,  16383 },
        { 12872, -22724,   4520,  19265, -19265,  -4520,  22724, -12872 },
        {  8867, -21406,  21406,  -8867,  -8867,  21406, -21406,   8867 },
        {  4520, -12872,  19265, -22724,  22724, -19265,  12872,  -4520 },
};
static const int16_t rgi16_y262_fdct_cs2[ 8 ][ 8 ] = {
        { 16385,  16385,  16385,  16385,  16385,  16385,  16385,  16385 },
        { 22726,  19266,  12873,   4521,  -4521, -12873, -19266, -22726 },
        { 21408,   8867,  -8867, -21408, -21408,  -8867,   8867,  21408 },
        { 19266,  -4521, -22726, -12873,  12873,  22726,   4521, -19266 },
        { 16385, -16385, -16385,  16385,  16385, -16385, -16385,  16385 },
        { 12873, -22726,   4521,  19266, -19266,  -4521,  22726, -12873 },
        {  8867, -21408,  21408,  -8867,  -8867,  21408, -21408,   8867 },
        {  4521, -12873,  19266, -22726,  22726, -19266,  12873,  -4521 },
};



void y262_fdct_c( int16_t *pi16_block, int16_t *pi16_dst )
{
	int i_i, i_j, i_k;
	int32_t i_s;
	int16_t rgi16_tmp[ 64 ];
	int32_t rgi16_e[ 4 ][ 8 ], rgi16_ee[ 2 ][ 8 ];

#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )

	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 1; i_j < 8; i_j += 2 )
		{
			i_s = 0;
			for ( i_k = 0; i_k < 4; i_k++ )
			{
				i_s += rgi16_y262_fdct_cs1[ i_j ][ i_k ] * ( pi16_block[ 8 * i_k + i_i ] - pi16_block[ 8 * ( 7 - i_k ) + i_i ] );
			}
			rgi16_tmp[ 8 * i_i + i_j ] = RND( i_s, RND1BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for ( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_e[ i_k ][ i_i ] = ( pi16_block[ 8 * i_k + i_i ] + pi16_block[ 8 * ( 7 - i_k ) + i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 2; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for ( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_y262_fdct_cs1[ i_j ][ i_k ] * ( rgi16_e[ i_k ][ i_i ] - rgi16_e[ 3 - i_k ][ i_i ] );
			}
			rgi16_tmp[ 8 * i_i + i_j ] = RND( i_s, RND1BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for ( i_k = 0; i_k < 2; i_k++ )
		{
			rgi16_ee[ i_k ][ i_i ] = ( rgi16_e[ i_k ][ i_i ] + rgi16_e[ 3 - i_k ][ i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 0; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for ( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_y262_fdct_cs1[ i_j ][ i_k ] * rgi16_ee[ i_k ][ i_i ];
			}
			rgi16_tmp[ 8 * i_i + i_j ] = RND( i_s, RND1BITS );
		}
    }

	/* ... */

	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 1; i_j < 8; i_j += 2 )
		{
			i_s = 0;
			for ( i_k = 0; i_k < 4; i_k++ )
			{
				i_s += rgi16_y262_fdct_cs2[ i_j ][ i_k ] * ( rgi16_tmp[ 8 * i_k + i_i ] - rgi16_tmp[ 8 * ( 7 - i_k ) + i_i ] );
			}
			pi16_dst[ 8 * i_i + i_j ] = RND( i_s, RND2BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for ( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_e[ i_k ][ i_i ] = ( rgi16_tmp[ 8 * i_k + i_i ] + rgi16_tmp[ 8 * ( 7 - i_k ) + i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 2; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for ( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_y262_fdct_cs2[ i_j ][ i_k ] * ( rgi16_e[ i_k ][ i_i ] - rgi16_e[ 3 - i_k ][ i_i ] );
			}
			pi16_dst[ 8 * i_i + i_j ] = RND( i_s, RND2BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for ( i_k = 0; i_k < 2; i_k++ )
		{
			rgi16_ee[ i_k ][ i_i ] = ( rgi16_e[ i_k ][ i_i ] + rgi16_e[ 3 - i_k ][ i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 0; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for ( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_y262_fdct_cs2[ i_j ][ i_k ] * rgi16_ee[ i_k ][ i_i ];
			}
			pi16_dst[ 8 * i_i + i_j ] = RND( i_s, RND2BITS );
		}
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


void y262_idct_c( int16_t *pi16_block, int16_t *pi16_dst )
{
	int i_j, i_k;
	int16_t rgi16_tmp[ 64 ];
	int32_t rgi_e[ 4 ], rgi_o[ 4 ];
	int32_t rgi_ee[ 2 ], rgi_eo[ 2 ];


#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_o[ 0 ] = rgi16_y262_idct_cs1[ 1 ][ 0 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 0 ] * pi16_block[ i_j + 8 * 3 ] + 
			rgi16_y262_idct_cs1[ 5 ][ 0 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 0 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 1 ] = rgi16_y262_idct_cs1[ 1 ][ 1 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 1 ] * pi16_block[ i_j + 8 * 3 ] + 
			rgi16_y262_idct_cs1[ 5 ][ 1 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 1 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 2 ] = rgi16_y262_idct_cs1[ 1 ][ 2 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 2 ] * pi16_block[ i_j + 8 * 3 ] + 
			rgi16_y262_idct_cs1[ 5 ][ 2 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 2 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 3 ] = rgi16_y262_idct_cs1[ 1 ][ 3 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 3 ] * pi16_block[ i_j + 8 * 3 ] + 
			rgi16_y262_idct_cs1[ 5 ][ 3 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 3 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_eo[ 0 ] = rgi16_y262_idct_cs1[ 2 ][ 0 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_y262_idct_cs1[ 6 ][ 0 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_eo[ 1 ] = rgi16_y262_idct_cs1[ 2 ][ 1 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_y262_idct_cs1[ 6 ][ 1 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_ee[ 0 ] = rgi16_y262_idct_cs1[ 0 ][ 0 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_y262_idct_cs1[ 4 ][ 0 ] * pi16_block[ i_j + 8 * 4 ];
		rgi_ee[ 1 ] = rgi16_y262_idct_cs1[ 0 ][ 1 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_y262_idct_cs1[ 4 ][ 1 ] * pi16_block[ i_j + 8 * 4 ];

		rgi_e[ 0 ] = rgi_ee[ 0 ] + rgi_eo[ 0 ];
		rgi_e[ 1 ] = rgi_ee[ 1 ] + rgi_eo[ 1 ];
		rgi_e[ 2 ] = rgi_ee[ 1 ] - rgi_eo[ 1 ];
		rgi_e[ 3 ] = rgi_ee[ 0 ] - rgi_eo[ 0 ];

		for( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_tmp[ i_j + 8 * i_k ] = RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND1BITS );
			rgi16_tmp[ i_j + 8 * ( i_k + 4 ) ] = RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND1BITS );
		}
	}

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_e[ 0 ] = rgi16_y262_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_y262_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_y262_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 1 ] = rgi16_y262_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_y262_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			 rgi16_y262_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_y262_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 2 ] = rgi16_y262_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_y262_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_y262_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 3 ] = rgi16_y262_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_y262_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_y262_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];

		rgi_o[ 0 ] = rgi16_y262_idct_cs2[ 1 ][ 0 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 0 ] * rgi16_tmp[ i_j * 8 + 3 ] + 
			rgi16_y262_idct_cs2[ 5 ][ 0 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 0 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 1 ] = rgi16_y262_idct_cs2[ 1 ][ 1 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 1 ] * rgi16_tmp[ i_j * 8 + 3 ] + 
			rgi16_y262_idct_cs2[ 5 ][ 1 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 1 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 2 ] = rgi16_y262_idct_cs2[ 1 ][ 2 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 2 ] * rgi16_tmp[ i_j * 8 + 3 ] + 
			rgi16_y262_idct_cs2[ 5 ][ 2 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 2 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 3 ] = rgi16_y262_idct_cs2[ 1 ][ 3 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 3 ] * rgi16_tmp[ i_j * 8 + 3 ] + 
			rgi16_y262_idct_cs2[ 5 ][ 3 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 3 ] * rgi16_tmp[ i_j * 8 + 7 ];

		for( i_k = 0; i_k < 4; i_k++ )
		{
			pi16_dst[ i_j * 8 + i_k ] = RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND2BITS );
			pi16_dst[ i_j * 8 + ( i_k + 4 ) ] = RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND2BITS );
		}		
	}
}



int32_t y262_quant8x8_intra_fw_mpeg2( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias )
{
	int32_t i_y, i_x, i_qm, i_nz;

	i_nz = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = ( i_y == 0 ) ? 1 : 0; i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_qm = pui16_qmat[ i_y * 8 + i_x ];
			i_level = pi_coeffs[ i_y * i_stride + i_x ];
			if( i_level < 0 )
			{
				i_level = -( ( ( ( -i_level + pui16_bias[ i_y * 8 + i_x ] ) * i_qm ) ) >> 16 );
			}
			else
			{
				i_level = ( ( ( i_level + pui16_bias[ i_y * 8 + i_x ] ) * i_qm ) ) >> 16;
			}
			pi_coeffs[ i_y * i_stride + i_x ] = i_level;
			i_nz |= i_level;
		}
	}
	return i_nz;
}

#define CLAMP256( x ) ( ( x ) < -256 ? -256 : ( ( x ) > 255 ? 255 : ( x ) ) )

int32_t y262_quant8x8_intra_fw_mpeg1( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias )
{
	int32_t i_y, i_x, i_qm, i_nz;

	i_nz = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = ( i_y == 0 ) ? 1 : 0; i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_qm = pui16_qmat[ i_y * 8 + i_x ];
			i_level = pi_coeffs[ i_y * i_stride + i_x ];
			if( i_level < 0 )
			{
				i_level = -( ( ( ( -i_level + pui16_bias[ i_y * 8 + i_x ] ) * i_qm ) ) >> 16 );
			}
			else
			{
				i_level = ( ( ( i_level + pui16_bias[ i_y * 8 + i_x ] ) * i_qm ) ) >> 16;
			}
			pi_coeffs[ i_y * i_stride + i_x ] = CLAMP256( i_level );
			i_nz |= i_level;
		}
	}
	return i_nz;
}


int32_t y262_quant8x8_intra_fw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias )
{
	if( !ps_y262->b_sequence_mpeg1 )
	{
		return ps_y262->s_funcs.f_quant8x8_intra_fw( pi_coeffs, i_stride, pui16_qmat, pui16_bias );
	}
	else
	{
		return y262_quant8x8_intra_fw_mpeg1( pi_coeffs, i_stride, pui16_qmat, pui16_bias );
	}
}

#define CLAMP_2047( x ) ( ( x ) < -2048 ? -2048 : ( ( x ) > 2047 ? 2047 : ( x ) ) )

void y262_quant8x8_intra_bw_mpeg2( int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat )
{
	int32_t i_y, i_x, i_qm, i_qt, i_missmatch_ctrl;

	i_missmatch_ctrl = pi_coeffs[ 0 ] + 1;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = ( ( i_y == 0 ) ? 1 : 0 ); i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_level = pi_coeffs[ i_y * i_stride + i_x ];

			if( i_level != 0 )
			{
				i_qm = pui8_qmat[ i_y * 8 + i_x ];
				i_qt = i_qm * i_quantizer * 2;

				i_level = ( i_level * i_qt ) / 32;

				pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( i_level );
				i_missmatch_ctrl += pi_coeffs[ i_y * i_stride + i_x ];
			}
		}
	}
	pi_coeffs[ 7 * i_stride + 7 ] ^= ( int16_t ) ( i_missmatch_ctrl & 1 );
}

void y262_quant8x8_intra_bw_mpeg1( int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat )
{
	int32_t i_y, i_x, i_qm, i_qt;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = ( ( i_y == 0 ) ? 1 : 0 ); i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_level = pi_coeffs[ i_y * i_stride + i_x ];

			if( i_level != 0 )
			{
				i_qm = pui8_qmat[ i_y * 8 + i_x ];
				i_qt = i_qm * i_quantizer * 2;

				i_level = ( i_level * i_qt ) / 32;
				if( i_level < 0 )
				{
					i_level = -i_level;
					i_level = ( i_level - 1 ) | 1;
					i_level = -i_level;
				}
				else
				{
					i_level = ( i_level - 1 ) | 1;
				}

				pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( i_level );
			}
		}
	}
}


void y262_quant8x8_intra_bw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat )
{
	if( !ps_y262->b_sequence_mpeg1 )
	{
		y262_quant8x8_intra_bw_mpeg2( pi_coeffs, i_stride, i_quantizer, pui8_qmat );
	}
	else
	{
		y262_quant8x8_intra_bw_mpeg1( pi_coeffs, i_stride, i_quantizer, pui8_qmat );
	}
}



int32_t y262_quant8x8_inter_fw_mpeg2( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat )
{
	int32_t i_y, i_x, i_qm, i_nz;

	i_nz = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_qm = pui16_qmat[ i_y * 8 + i_x ];
			i_level = pi_coeffs[ i_y * i_stride + i_x ];
			if( i_level < 0 )
			{
				i_level = -( ( -i_level * i_qm ) >> 16 );
			}
			else
			{
				i_level = ( i_level * i_qm ) >> 16;
			}
			pi_coeffs[ i_y * i_stride + i_x ] = i_level;
			i_nz |= i_level;
		}
	}
	return i_nz;
}

int32_t y262_quant8x8_inter_fw_mpeg1( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat )
{
	int32_t i_y, i_x, i_qm, i_nz;

	i_nz = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_qm = pui16_qmat[ i_y * 8 + i_x ];
			i_level = pi_coeffs[ i_y * i_stride + i_x ];
			if( i_level < 0 )
			{
				i_level = -( ( -i_level * i_qm ) >> 16 );
			}
			else
			{
				i_level = ( i_level * i_qm ) >> 16;
			}
			pi_coeffs[ i_y * i_stride + i_x ] = CLAMP256( i_level );
			i_nz |= i_level;
		}
	}
	return i_nz;
}

int32_t y262_quant8x8_inter_fw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat )
{
	if( !ps_y262->b_sequence_mpeg1 )
	{
		return ps_y262->s_funcs.f_quant8x8_inter_fw( pi_coeffs, i_stride, pui16_qmat );
	}
	else
	{
		return y262_quant8x8_inter_fw_mpeg1( pi_coeffs, i_stride, pui16_qmat );
	}
}


#define CLAMP_2047( x ) ( ( x ) < -2048 ? -2048 : ( ( x ) > 2047 ? 2047 : ( x ) ) )

void y262_quant8x8_inter_bw_mpeg2( int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat )
{
	int32_t i_y, i_x, i_qm, i_qt, i_missmatch_ctrl;

	i_missmatch_ctrl = 1;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_level = pi_coeffs[ i_y * i_stride + i_x ];

			if( i_level != 0 )
			{
				i_qm = pui8_qmat[ i_y * 8 + i_x ];
				i_qt = i_qm * i_quantizer;

				if( i_level > 0 )
				{
					i_level = ( ( i_level * 2 + 1 ) * i_qt ) / 32;
				}
				else if( i_level < 0 )
				{
					i_level = ( ( i_level * 2 - 1 ) * i_qt ) / 32;
				}

				pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( i_level );
				i_missmatch_ctrl += pi_coeffs[ i_y * i_stride + i_x ];
			}
		}
	}
	pi_coeffs[ 7 * i_stride + 7 ] ^= ( int16_t ) ( i_missmatch_ctrl & 1 );
}

void y262_quant8x8_inter_bw_mpeg1( int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat )
{
	int32_t i_y, i_x, i_qm, i_qt;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			int32_t i_level;
			i_level = pi_coeffs[ i_y * i_stride + i_x ];

			if( i_level != 0 )
			{
				i_qm = pui8_qmat[ i_y * 8 + i_x ];
				i_qt = i_qm * i_quantizer;

				if( i_level > 0 )
				{
					i_level = ( ( i_level * 2 + 1 ) * i_qt ) / 32;
					i_level = ( i_level - 1 ) | 1;
				}
				else if( i_level < 0 )
				{
					i_level = -i_level;
					i_level = ( ( i_level * 2 + 1 ) * i_qt ) / 32;
					i_level = ( i_level - 1 ) | 1;
					i_level = -i_level;
				}
				
				pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( i_level );
			}
		}
	}
}

void y262_quant8x8_inter_bw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat )
{
	if( !ps_y262->b_sequence_mpeg1 )
	{
		y262_quant8x8_inter_bw_mpeg2( pi_coeffs, i_stride, i_quantizer, pui8_qmat );
	}
	else
	{
		y262_quant8x8_inter_bw_mpeg1( pi_coeffs, i_stride, i_quantizer, pui8_qmat );
	}
}



int32_t y262_size_run_level( int32_t i_run, int32_t i_level )
{
	int32_t i_ulevel;
	i_ulevel = i_level < 0 ? -i_level : i_level;

	if( i_run < 32 && i_ulevel < 41 )
	{
		return rgi_y262_run_level_bits_zero[ i_run ][ i_ulevel ];
	}
#if 0
	for( i_idx = 0; rgs_y262_dct_coefficients_table_zero[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
	{
		if( rgs_y262_dct_coefficients_lookup_table_zero[ i_idx ].i_run == i_run &&
			rgs_y262_dct_coefficients_lookup_table_zero[ i_idx ].i_level == i_ulevel )
		{
			return rgs_y262_dct_coefficients_table_zero[ i_idx ].i_length + 1; /* +1 sign */
		}
	}
#endif
	return 24; /* 6 escape 6 run 12 level */
}


void y262_quant8x8_trellis_copy_int8( int8_t *pi8_dst, const int8_t *pi8_src, int32_t i_cnt )
{
	int32_t i_idx;

	for( i_idx = 0; i_idx < i_cnt; i_idx++ )
	{
		pi8_dst[ i_idx ] = pi8_src[ i_idx ];
	}
}

void y262_quant8x8_trellis_copy_int16( int16_t *pi16_dst, const int16_t *pi16_src, int32_t i_cnt )
{
	int32_t i_idx;

	for( i_idx = 0; i_idx < i_cnt; i_idx++ )
	{
		pi16_dst[ i_idx ] = pi16_src[ i_idx ];
	}
}


int32_t y262_quant8x8_trellis_fw( y262_t *ps_y262, y262_slice_t *ps_slice, int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, bool_t b_intra )
{
	int32_t i_dc, i_nz;
	int32_t i_coeff, i_start, i_num_coeff, i_last_coeff, i_idx, i_idx2, i_run, i_level;
	int16_t rgi16_levels[ 64 ];
	int16_t rgi16_coeffs[ 64 ];
	int8_t rgi8_idx[ 64 ];
	int16_t rgi16_level[ 64 ];
	uint8_t *pui8_qmat;
	y262_macroblock_t *ps_mb;

	int32_t i_active_toggle;
	int32_t i_candidate_level, i_dir, i_end, i_ssd, i_bits, i_cost, i_lambda;

	ps_mb = &ps_slice->s_macroblock;

	i_lambda = ps_mb->i_lambda;

	i_run = 0;
	i_last_coeff = -1;
	i_num_coeff = 0;

	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		memcpy( &rgi16_levels[ i_idx * 8 ], pi_coeffs + ( i_idx * i_stride ), sizeof( int16_t ) * 8 );
	}

	if( b_intra )
	{
		i_dc = rgi16_levels[ 0 ];
		i_start = 1;
		pui8_qmat = ps_y262->rgui8_intra_quantiser_matrix;
		y262_quant8x8_intra_fw( ps_y262, rgi16_levels, 8, ps_y262->rgui16_intra_quantizer_matrices[ i_quantizer ], ps_y262->rgui16_intra_quantizer_matrices_trellis_bias[ i_quantizer ] );
		i_nz = 1;
	}
	else
	{
		i_dc = 0;
		i_start = 0;
		pui8_qmat = ps_y262->rgui8_non_intra_quantiser_matrix;
		y262_quant8x8_inter_fw( ps_y262, rgi16_levels, 8, ps_y262->rgui16_non_intra_quantizer_matrices[ i_quantizer ] );
	}

	if( i_nz )
	{
		for( i_idx = i_start; i_idx < 64; i_idx++ )
		{
			i_level = rgi16_levels[ rgui8_y262_scan_0_table[ i_idx ] ];

			if( i_level != 0 )
			{
				rgi16_coeffs[ i_num_coeff ] = pi_coeffs[ rgui8_y262_scan_0_table[ i_idx ] ];
				rgi8_idx[ i_num_coeff ] = i_idx;
				rgi16_level[ i_num_coeff ] = i_level;
				i_num_coeff++;
				i_last_coeff = i_idx;
			}
		}
	}

	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		memset( pi_coeffs + ( i_idx * i_stride ), 0, sizeof( int16_t ) * 8 );
	}
	pi_coeffs[ 0 ] = i_dc;

	if( !i_nz )
	{
		return i_dc;
	}
	if( i_last_coeff < 0 )
	{
		return i_dc;
	}
	
	memset( &ps_y262->trellis.rgi8_path_active, 0, sizeof( ps_y262->trellis.rgi8_path_active ) );
	memset( &ps_y262->trellis.rgi_path_cost, 0, sizeof( ps_y262->trellis.rgi_path_cost ) );

	i_active_toggle = 0;
	ps_y262->trellis.rgi8_path_active[ i_active_toggle ][ 0 ] = 1;

	for( i_idx = 0; i_idx < i_num_coeff; i_idx++ )
	{
		i_level = rgi16_level[ i_idx ];

		if( i_level > 0 )
		{
			i_dir = -1;
		}
		else
		{
			i_dir = 1;
		}
		i_end = i_level + i_dir * 2;
		for( i_candidate_level = i_level; i_candidate_level != i_end; i_candidate_level += i_dir )
		{
			int32_t i_qm, i_qt, i_x, i_y;

			i_x = rgui8_y262_scan_0_table[ rgi8_idx[ i_idx ] ] % 8;
			i_y = rgui8_y262_scan_0_table[ rgi8_idx[ i_idx ] ] / 8;
			i_qm = pui8_qmat[ i_y * 8 + i_x ];

			if( b_intra )
			{
				i_qt = i_qm * i_quantizer * 2;
				i_coeff = ( i_candidate_level * i_qt ) / 32;
			}
			else
			{
				i_qt = i_qm * i_quantizer;

				if( i_candidate_level > 0 )
				{
					i_coeff = ( ( i_candidate_level * 2 + 1 ) * i_qt ) / 32;
				}
				else if( i_candidate_level < 0 )
				{
					i_coeff = ( ( i_candidate_level * 2 - 1 ) * i_qt ) / 32;
				}
				else
				{
					i_coeff = 0;
				}
			}

			i_coeff = CLAMP_2047( i_coeff );

			i_ssd = ( i_coeff - rgi16_coeffs[ i_idx ] ) * ( i_coeff - rgi16_coeffs[ i_idx ] );

			for( i_idx2 = i_idx; i_idx2 >= 0; i_idx2-- )
			{
				if( ps_y262->trellis.rgi8_path_active[ i_active_toggle ][ i_idx2 ] )
				{
					if( i_candidate_level != 0 )
					{
						int32_t i_run;
						if( i_idx2 > 0 )
						{
							i_run = rgi8_idx[ i_idx ] - ps_y262->trellis.rgi8_path_idx[ i_idx2 ][ i_idx2 ] - 1;
						}
						else
						{
							i_run = rgi8_idx[ i_idx ];
						}
						i_bits = y262_size_run_level( i_run, i_candidate_level );
						i_cost = ps_y262->trellis.rgi_path_cost[ i_idx2 ] + ( ( i_bits * i_lambda ) >> Y262_LAMBDA_BITS ) + i_ssd;

						if( !ps_y262->trellis.rgi8_path_active[ !i_active_toggle ][ i_idx2 + 1 ] || i_cost < ps_y262->trellis.rgi_path_cost[ i_idx2 + 1 ] )
						{
							y262_quant8x8_trellis_copy_int8( &ps_y262->trellis.rgi8_path_idx[ i_idx2 + 1 ][ 0 ], &ps_y262->trellis.rgi8_path_idx[ i_idx2 ][ 0 ], ( i_idx2 + 1 ) );
							y262_quant8x8_trellis_copy_int16( &ps_y262->trellis.rgi16_path_level[ i_idx2 + 1 ][ 0 ], &ps_y262->trellis.rgi16_path_level[ i_idx2 ][ 0 ], ( i_idx2 + 1 ) );
							ps_y262->trellis.rgi8_path_idx[ i_idx2 + 1 ][ i_idx2 + 1 ] = rgi8_idx[ i_idx ];
							ps_y262->trellis.rgi16_path_level[ i_idx2 + 1 ][ i_idx2 + 1 ] = i_candidate_level;
							ps_y262->trellis.rgi8_path_active[ !i_active_toggle ][ i_idx2 + 1 ] = 1;
							ps_y262->trellis.rgi_path_cost[ i_idx2 + 1 ] = i_cost;
						}
					}
					else
					{
						/* last coeff candidate iter, we can overwrite/activate current path */
						i_cost = ps_y262->trellis.rgi_path_cost[ i_idx2 ] + i_ssd;
						if( !ps_y262->trellis.rgi8_path_active[ !i_active_toggle ][ i_idx2 ] || i_cost < ps_y262->trellis.rgi_path_cost[ i_idx2 ] )
						{
							ps_y262->trellis.rgi8_path_active[ !i_active_toggle ][ i_idx2 ] = 1;
							ps_y262->trellis.rgi_path_cost[ i_idx2 ] = i_cost;
						}
					}
				}
			}
		}
		memset( &ps_y262->trellis.rgi8_path_active[ i_active_toggle ][ 0 ], 0, sizeof( int8_t ) * 65 );
		i_active_toggle = i_active_toggle^1;
	}

	i_cost = MAX_COST;
	i_idx2 = 0;
	for( i_idx = 0; i_idx <= i_num_coeff; i_idx++ )
	{
		if( ps_y262->trellis.rgi8_path_active[ i_active_toggle ][ i_idx ] && ps_y262->trellis.rgi_path_cost[ i_idx ] < i_cost )
		{
			i_idx2 = i_idx;
			i_cost = ps_y262->trellis.rgi_path_cost[ i_idx ];
		}
	}
	for( i_idx = 1; i_idx <= i_idx2; i_idx++ )
	{
		int32_t i_x, i_y;
		if( ps_y262->trellis.rgi16_path_level[ i_idx2 ][ i_idx ] != rgi16_level[ i_idx - 1 ] )
		{
			i_idx = i_idx;
		}
		i_x = rgui8_y262_scan_0_table[ ps_y262->trellis.rgi8_path_idx[ i_idx2 ][ i_idx ] ] % 8;
		i_y = rgui8_y262_scan_0_table[ ps_y262->trellis.rgi8_path_idx[ i_idx2 ][ i_idx ] ] / 8;
		pi_coeffs[ i_x + i_y * i_stride ] = ps_y262->trellis.rgi16_path_level[ i_idx2 ][ i_idx ];
	}
	return i_idx2 != 0;
}

