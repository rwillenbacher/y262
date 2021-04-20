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

int32_t y262_sad_16x16( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_sad, i_y;

	i_sad = 0;
	for( i_y = 0; i_y < 16; i_y++ )
	{
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 0 ] - pui8_blk2[ i_y * i_stride2 + 0 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 1 ] - pui8_blk2[ i_y * i_stride2 + 1 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 2 ] - pui8_blk2[ i_y * i_stride2 + 2 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 3 ] - pui8_blk2[ i_y * i_stride2 + 3 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 4 ] - pui8_blk2[ i_y * i_stride2 + 4 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 5 ] - pui8_blk2[ i_y * i_stride2 + 5 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 6 ] - pui8_blk2[ i_y * i_stride2 + 6 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 7 ] - pui8_blk2[ i_y * i_stride2 + 7 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 8 ] - pui8_blk2[ i_y * i_stride2 + 8 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 9 ] - pui8_blk2[ i_y * i_stride2 + 9 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 10 ] - pui8_blk2[ i_y * i_stride2 + 10 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 11 ] - pui8_blk2[ i_y * i_stride2 + 11 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 12 ] - pui8_blk2[ i_y * i_stride2 + 12 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 13 ] - pui8_blk2[ i_y * i_stride2 + 13 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 14 ] - pui8_blk2[ i_y * i_stride2 + 14 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 15 ] - pui8_blk2[ i_y * i_stride2 + 15 ] );
	}
	return i_sad;
}

int32_t y262_sad_16x8( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_sad, i_y;

	i_sad = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 0 ] - pui8_blk2[ i_y * i_stride2 + 0 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 1 ] - pui8_blk2[ i_y * i_stride2 + 1 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 2 ] - pui8_blk2[ i_y * i_stride2 + 2 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 3 ] - pui8_blk2[ i_y * i_stride2 + 3 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 4 ] - pui8_blk2[ i_y * i_stride2 + 4 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 5 ] - pui8_blk2[ i_y * i_stride2 + 5 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 6 ] - pui8_blk2[ i_y * i_stride2 + 6 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 7 ] - pui8_blk2[ i_y * i_stride2 + 7 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 8 ] - pui8_blk2[ i_y * i_stride2 + 8 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 9 ] - pui8_blk2[ i_y * i_stride2 + 9 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 10 ] - pui8_blk2[ i_y * i_stride2 + 10 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 11 ] - pui8_blk2[ i_y * i_stride2 + 11 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 12 ] - pui8_blk2[ i_y * i_stride2 + 12 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 13 ] - pui8_blk2[ i_y * i_stride2 + 13 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 14 ] - pui8_blk2[ i_y * i_stride2 + 14 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 15 ] - pui8_blk2[ i_y * i_stride2 + 15 ] );
	}
	return i_sad;
}

int32_t y262_sad_8x8( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_sad, i_y;

	i_sad = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 0 ] - pui8_blk2[ i_y * i_stride2 + 0 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 1 ] - pui8_blk2[ i_y * i_stride2 + 1 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 2 ] - pui8_blk2[ i_y * i_stride2 + 2 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 3 ] - pui8_blk2[ i_y * i_stride2 + 3 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 4 ] - pui8_blk2[ i_y * i_stride2 + 4 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 5 ] - pui8_blk2[ i_y * i_stride2 + 5 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 6 ] - pui8_blk2[ i_y * i_stride2 + 6 ] );
		i_sad += abs( pui8_blk1[ i_y * i_stride1 + 7 ] - pui8_blk2[ i_y * i_stride2 + 7 ] );
	}
	return i_sad;
}


int32_t y262_satd_8x8( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_idx, m[ 8 ][ 8 ], d[ 8 ][ 8 ], rgi_diff[ 8 ][ 8 ];
	int32_t i_satd;

	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		rgi_diff[ i_idx ][ 0 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );
		rgi_diff[ i_idx ][ 1 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );
		rgi_diff[ i_idx ][ 2 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );
		rgi_diff[ i_idx ][ 3 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );
		rgi_diff[ i_idx ][ 4 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );
		rgi_diff[ i_idx ][ 5 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );
		rgi_diff[ i_idx ][ 6 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );
		rgi_diff[ i_idx ][ 7 ] = ( *pui8_blk1++ ) - ( *pui8_blk2++ );

		pui8_blk1 += ( i_stride1 - 8 );
		pui8_blk2 += ( i_stride2 - 8 );
	}

	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		m[ i_idx ][ 0 ] = rgi_diff[ i_idx ][ 0 ] + rgi_diff[ i_idx ][ 4 ];
		m[ i_idx ][ 1 ] = rgi_diff[ i_idx ][ 1 ] + rgi_diff[ i_idx ][ 5 ];
		m[ i_idx ][ 2 ] = rgi_diff[ i_idx ][ 2 ] + rgi_diff[ i_idx ][ 6 ];
		m[ i_idx ][ 3 ] = rgi_diff[ i_idx ][ 3 ] + rgi_diff[ i_idx ][ 7 ];
		m[ i_idx ][ 4 ] = rgi_diff[ i_idx ][ 0 ] - rgi_diff[ i_idx ][ 4 ];
		m[ i_idx ][ 5 ] = rgi_diff[ i_idx ][ 1 ] - rgi_diff[ i_idx ][ 5 ];
		m[ i_idx ][ 6 ] = rgi_diff[ i_idx ][ 2 ] - rgi_diff[ i_idx ][ 6 ];
		m[ i_idx ][ 7 ] = rgi_diff[ i_idx ][ 3 ] - rgi_diff[ i_idx ][ 7 ];

		d[ i_idx ][ 0 ] = m[ i_idx ][ 0 ] + m[ i_idx ][ 2 ];
		d[ i_idx ][ 1 ] = m[ i_idx ][ 1 ] + m[ i_idx ][ 3 ];
		d[ i_idx ][ 2 ] = m[ i_idx ][ 0 ] - m[ i_idx ][ 2 ];
		d[ i_idx ][ 3 ] = m[ i_idx ][ 1 ] - m[ i_idx ][ 3 ];
		d[ i_idx ][ 4 ] = m[ i_idx ][ 4 ] + m[ i_idx ][ 6 ];
		d[ i_idx ][ 5 ] = m[ i_idx ][ 5 ] + m[ i_idx ][ 7 ];
		d[ i_idx ][ 6 ] = m[ i_idx ][ 4 ] - m[ i_idx ][ 6 ];
		d[ i_idx ][ 7 ] = m[ i_idx ][ 5 ] - m[ i_idx ][ 7 ];

		m[ i_idx ][ 0 ] = d[ i_idx ][ 0 ] + d[ i_idx ][ 1 ];
		m[ i_idx ][ 1 ] = d[ i_idx ][ 0 ] - d[ i_idx ][ 1 ];
		m[ i_idx ][ 2 ] = d[ i_idx ][ 2 ] + d[ i_idx ][ 3 ];
		m[ i_idx ][ 3 ] = d[ i_idx ][ 2 ] - d[ i_idx ][ 3 ];
		m[ i_idx ][ 4 ] = d[ i_idx ][ 4 ] + d[ i_idx ][ 5 ];
		m[ i_idx ][ 5 ] = d[ i_idx ][ 4 ] - d[ i_idx ][ 5 ];
		m[ i_idx ][ 6 ] = d[ i_idx ][ 6 ] + d[ i_idx ][ 7 ];
		m[ i_idx ][ 7 ] = d[ i_idx ][ 6 ] - d[ i_idx ][ 7 ];
	}

	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		d[ 0 ][ i_idx ] = m[ 0 ][ i_idx ] + m[ 4 ][ i_idx ];
		d[ 1 ][ i_idx ] = m[ 1 ][ i_idx ] + m[ 5 ][ i_idx ];
		d[ 2 ][ i_idx ] = m[ 2 ][ i_idx ] + m[ 6 ][ i_idx ];
		d[ 3 ][ i_idx ] = m[ 3 ][ i_idx ] + m[ 7 ][ i_idx ];
		d[ 4 ][ i_idx ] = m[ 0 ][ i_idx ] - m[ 4 ][ i_idx ];
		d[ 5 ][ i_idx ] = m[ 1 ][ i_idx ] - m[ 5 ][ i_idx ];
		d[ 6 ][ i_idx ] = m[ 2 ][ i_idx ] - m[ 6 ][ i_idx ];
		d[ 7 ][ i_idx ] = m[ 3 ][ i_idx ] - m[ 7 ][ i_idx ];

		m[ 0 ][ i_idx ] = d[ 0 ][ i_idx ] + d[ 2 ][ i_idx ];
		m[ 1 ][ i_idx ] = d[ 1 ][ i_idx ] + d[ 3 ][ i_idx ];
		m[ 2 ][ i_idx ] = d[ 0 ][ i_idx ] - d[ 2 ][ i_idx ];
		m[ 3 ][ i_idx ] = d[ 1 ][ i_idx ] - d[ 3 ][ i_idx ];
		m[ 4 ][ i_idx ] = d[ 4 ][ i_idx ] + d[ 6 ][ i_idx ];
		m[ 5 ][ i_idx ] = d[ 5 ][ i_idx ] + d[ 7 ][ i_idx ];
		m[ 6 ][ i_idx ] = d[ 4 ][ i_idx ] - d[ 6 ][ i_idx ];
		m[ 7 ][ i_idx ] = d[ 5 ][ i_idx ] - d[ 7 ][ i_idx ];

		d[ 0 ][ i_idx ] = m[ 0 ][ i_idx ] + m[ 1 ][ i_idx ];
		d[ 1 ][ i_idx ] = m[ 0 ][ i_idx ] - m[ 1 ][ i_idx ];
		d[ 2 ][ i_idx ] = m[ 2 ][ i_idx ] + m[ 3 ][ i_idx ];
		d[ 3 ][ i_idx ] = m[ 2 ][ i_idx ] - m[ 3 ][ i_idx ];
		d[ 4 ][ i_idx ] = m[ 4 ][ i_idx ] + m[ 5 ][ i_idx ];
		d[ 5 ][ i_idx ] = m[ 4 ][ i_idx ] - m[ 5 ][ i_idx ];
		d[ 6 ][ i_idx ] = m[ 6 ][ i_idx ] + m[ 7 ][ i_idx ];
		d[ 7 ][ i_idx ] = m[ 6 ][ i_idx ] - m[ 7 ][ i_idx ];
	}

	i_satd = 0;
	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		i_satd += abs( d[ i_idx ][ 0 ] );
		i_satd += abs( d[ i_idx ][ 1 ] );
		i_satd += abs( d[ i_idx ][ 2 ] );
		i_satd += abs( d[ i_idx ][ 3 ] );
		i_satd += abs( d[ i_idx ][ 4 ] );
		i_satd += abs( d[ i_idx ][ 5 ] );
		i_satd += abs( d[ i_idx ][ 6 ] );
		i_satd += abs( d[ i_idx ][ 7 ] );
	}

	i_satd = ( ( i_satd + 2 ) >> 2 );

	return i_satd;
}


int32_t y262_satd_16x16( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_satd;

	i_satd  = y262_satd_8x8( pui8_blk1,                         i_stride1, pui8_blk2, i_stride2 );
	i_satd += y262_satd_8x8( pui8_blk1 + 8,                     i_stride1, pui8_blk2 + 8, i_stride2 );
	i_satd += y262_satd_8x8( pui8_blk1 + ( 8 * i_stride1 ),     i_stride1, pui8_blk2 + ( 8 * i_stride2 ), i_stride2 );
	i_satd += y262_satd_8x8( pui8_blk1 + 8 + ( 8 * i_stride1 ), i_stride1, pui8_blk2 + 8 + ( 8 * i_stride2 ), i_stride2 );

	return i_satd;
}


int32_t y262_satd_16x8( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_satd;

	i_satd  = y262_satd_8x8( pui8_blk1,                         i_stride1, pui8_blk2, i_stride2 );
	i_satd += y262_satd_8x8( pui8_blk1 + 8,                     i_stride1, pui8_blk2 + 8, i_stride2 );

	return i_satd;
}

int32_t y262_satd_16x16_sse2( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_satd;

	i_satd  = y262_satd_8x8_sse2( pui8_blk1,                         i_stride1, pui8_blk2, i_stride2 );
	i_satd += y262_satd_8x8_sse2( pui8_blk1 + 8,                     i_stride1, pui8_blk2 + 8, i_stride2 );
	i_satd += y262_satd_8x8_sse2( pui8_blk1 + ( 8 * i_stride1 ),     i_stride1, pui8_blk2 + ( 8 * i_stride2 ), i_stride2 );
	i_satd += y262_satd_8x8_sse2( pui8_blk1 + 8 + ( 8 * i_stride1 ), i_stride1, pui8_blk2 + 8 + ( 8 * i_stride2 ), i_stride2 );

	return i_satd;
}


int32_t y262_satd_16x8_sse2( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 )
{
	int32_t i_satd;

	i_satd  = y262_satd_8x8_sse2( pui8_blk1,                         i_stride1, pui8_blk2, i_stride2 );
	i_satd += y262_satd_8x8_sse2( pui8_blk1 + 8,                     i_stride1, pui8_blk2 + 8, i_stride2 );

	return i_satd;
}


int32_t y262_ssd_16x16( uint8_t *pui8_blk1, int32_t i_blk1_stride, uint8_t *pui8_blk2, int32_t i_blk2_stride )
{
	int32_t i_y, i_x, i_ssd;

	i_ssd = 0;
	for( i_y = 0; i_y < 16; i_y ++ )
	{
		for( i_x = 0; i_x < 16; i_x++ )
		{
			int32_t i_diff;
			i_diff = pui8_blk1[ i_x + i_y * i_blk1_stride ] - pui8_blk2[ i_x + i_y * i_blk2_stride ];
			i_ssd += i_diff * i_diff;
		}
	}
	return i_ssd;
}

int32_t y262_ssd_8x8( uint8_t *pui8_blk1, int32_t i_blk1_stride, uint8_t *pui8_blk2, int32_t i_blk2_stride )
{
	int32_t i_y, i_x, i_ssd;

	i_ssd = 0;
	for( i_y = 0; i_y < 8; i_y ++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			int32_t i_diff;
			i_diff = pui8_blk1[ i_x + i_y * i_blk1_stride ] - pui8_blk2[ i_x + i_y * i_blk2_stride ];
			i_ssd += i_diff * i_diff;
		}
	}
	return i_ssd;
}


int32_t y262_variance_16x16( uint8_t *pui8_blk, int32_t i_blk_stride )
{
	int32_t i_y, i_x, i_sum, i_sqr;

	i_sum = i_sqr = 0;
	for( i_y = 0; i_y < 16; i_y++ )
	{
		for( i_x = 0; i_x < 16; i_x++ )
		{
			i_sum += pui8_blk[ i_x ];
			i_sqr += pui8_blk[ i_x ] * pui8_blk[ i_x ];
		}
		pui8_blk += i_blk_stride;
	}
	return ( i_sqr - ( ( ( ( int64_t )i_sum ) * i_sum ) >> 8 ) );
}

int32_t y262_variance_8x8( uint8_t *pui8_blk, int32_t i_blk_stride )
{
	int32_t i_y, i_x, i_sum, i_sqr;

	i_sum = i_sqr = 0;
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			i_sum += pui8_blk[ i_x ];
			i_sqr += pui8_blk[ i_x ] * pui8_blk[ i_x ];
		}
		pui8_blk += i_blk_stride;
	}
	return ( i_sqr - ( ( ( ( int64_t )i_sum ) * i_sum ) >> 6 ) );
}


void y262_sub_8x8( int16_t *pi16_diff, uint8_t *pui8_src1, int32_t i_stride_src1, uint8_t *pui8_src2, int32_t i_stride_src2 )
{
	int32_t i_x, i_y;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			pi16_diff[ i_x + i_y * 8 ] = pui8_src1[ i_x + i_y * i_stride_src1 ] - pui8_src2[ i_x + i_y * i_stride_src2 ];
		}
	}
}

void y262_add_8x8( uint8_t *pui8_destination, int32_t i_destination_stride, uint8_t *pui8_base, int32_t i_base_stride, int16_t *pi16_difference )
{
	int32_t i_x, i_y;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			pui8_destination[ i_x + i_y * i_destination_stride ] = MIN( 255, MAX( 0, pui8_base[ i_x + i_y * i_base_stride ] + pi16_difference[ i_x + i_y * 8 ] ) );
		}
	}
}

bool_t y262_16x16_frame_field_pel_decision( uint8_t *pui8_src, int32_t i_src_stride )
{
	int32_t i_y, i_x, i_frame, i_field;

	i_frame = 0;
	for( i_y = 0; i_y < 16 - 1; i_y++ )
	{
		for( i_x = 0; i_x < 16; i_x++ )
		{
			i_frame += abs( pui8_src[ i_y * i_src_stride + i_x ] - pui8_src[ ( i_y + 1 ) * i_src_stride + i_x ] );
		}
	}

	i_field = 0;
	for( i_y = 0; i_y < 16 - 2; i_y++ )
	{
		for( i_x = 0; i_x < 16; i_x++ )
		{
			i_field += abs( pui8_src[ i_y * i_src_stride + i_x ] - pui8_src[ ( i_y + 2 ) * i_src_stride + i_x ] );
		}
	}
	return i_field < i_frame;
}





