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

void y262_motcomp_16x16_00_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x16_01_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x16_10_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x16_11_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_16x8_00_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x8_01_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x8_10_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x8_11_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_8x16_00_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x16_01_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x16_10_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x16_11_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_8x8_00_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x8_01_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x8_10_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x8_11_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_8x4_00_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x4_01_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x4_10_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x4_11_put_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_16x16_00_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x16_01_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x16_10_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x16_11_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_16x8_00_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x8_01_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x8_10_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_16x8_11_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_8x16_00_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x16_01_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x16_10_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x16_11_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_8x8_00_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x8_01_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x8_10_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x8_11_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262_motcomp_8x4_00_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x4_01_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x4_10_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262_motcomp_8x4_11_avg_mmxext( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );





#define MC_FUNC_REF( name, i_width, i_height, hpelidx )	\
void y262_motcomp_##name##_put( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )		\
{														\
	int32_t i_x, i_y;									\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = pui8_src[ i_x ];		\
			}											\
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
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = ( pui8_src1[ i_x ] + pui8_src2[ i_x ] + 1 ) >> 1;		\
			}			\
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
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = ( pui8_src1[ i_x ] + pui8_src2[ i_x ] + 1 ) >> 1;		\
			}											\
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;		\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = ( pui8_src1[ i_x ] + pui8_src2[ i_x ] + pui8_src3[ i_x ] + pui8_src4[ i_x ] + 2 ) >> 2;	\
			}											\
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
void y262_motcomp_##name##_avg( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )	\
{														\
	int32_t i_x, i_y;									\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = ( pui8_src[ i_x ] + pui8_dst[ i_x ] + 1 ) >> 1;	\
			}											\
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
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = ( ( ( pui8_src1[ i_x ] + pui8_src2[ i_x ] + 1 ) >> 1 ) + pui8_dst[ i_x ] + 1 ) / 2;	\
			}											\
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
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = ( ( ( pui8_src1[ i_x ] + pui8_src2[ i_x ] + 1 ) >> 1 ) + pui8_dst[ i_x ] + 1 ) / 2;	\
			}											\
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;		\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			for( i_x = 0; i_x < i_width; i_x++ )		\
			{											\
				pui8_dst[ i_x ] = ( ( ( pui8_src1[ i_x ] + pui8_src2[ i_x ] + pui8_src3[ i_x ] +	\
					pui8_src4[ i_x ] + 2 ) >> 2 ) + pui8_dst[ i_x ] + 1 ) >> 1;		\
			}											\
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_src3 += i_src_stride;					\
			pui8_src4 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
}														\


MC_FUNC_REF( 16x16_00, 16, 16, 0 );
MC_FUNC_REF( 16x16_01, 16, 16, 1 );
MC_FUNC_REF( 16x16_10, 16, 16, 2 );
MC_FUNC_REF( 16x16_11, 16, 16, 3 );

MC_FUNC_REF( 16x8_00, 16, 8, 0 );
MC_FUNC_REF( 16x8_01, 16, 8, 1 );
MC_FUNC_REF( 16x8_10, 16, 8, 2 );
MC_FUNC_REF( 16x8_11, 16, 8, 3 );

MC_FUNC_REF( 8x16_00, 8, 16, 0 );
MC_FUNC_REF( 8x16_01, 8, 16, 1 );
MC_FUNC_REF( 8x16_10, 8, 16, 2 );
MC_FUNC_REF( 8x16_11, 8, 16, 3 );

MC_FUNC_REF( 8x8_00, 8, 8, 0 );
MC_FUNC_REF( 8x8_01, 8, 8, 1 );
MC_FUNC_REF( 8x8_10, 8, 8, 2 );
MC_FUNC_REF( 8x8_11, 8, 8, 3 );

MC_FUNC_REF( 8x4_00, 8, 4, 0 );
MC_FUNC_REF( 8x4_01, 8, 4, 1 );
MC_FUNC_REF( 8x4_10, 8, 4, 2 );
MC_FUNC_REF( 8x4_11, 8, 4, 3 );



void y262_init_motion_compensation( y262_t *ps_y262 )
{
	/* copy */
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_00 ] = y262_motcomp_16x16_00_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_01 ] = y262_motcomp_16x16_01_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_10 ] = y262_motcomp_16x16_10_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_11 ] = y262_motcomp_16x16_11_put;

	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_00 ] = y262_motcomp_16x8_00_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_01 ] = y262_motcomp_16x8_01_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_10 ] = y262_motcomp_16x8_10_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_11 ] = y262_motcomp_16x8_11_put;

	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_00 ] = y262_motcomp_8x16_00_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_01 ] = y262_motcomp_8x16_01_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_10 ] = y262_motcomp_8x16_10_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_11 ] = y262_motcomp_8x16_11_put;

	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_00 ] = y262_motcomp_8x8_00_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_01 ] = y262_motcomp_8x8_01_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_10 ] = y262_motcomp_8x8_10_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_11 ] = y262_motcomp_8x8_11_put;

	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_00 ] = y262_motcomp_8x4_00_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_01 ] = y262_motcomp_8x4_01_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_10 ] = y262_motcomp_8x4_10_put;
	ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_11 ] = y262_motcomp_8x4_11_put;


	/* avg */
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_00 ] = y262_motcomp_16x16_00_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_01 ] = y262_motcomp_16x16_01_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_10 ] = y262_motcomp_16x16_10_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_11 ] = y262_motcomp_16x16_11_avg;

	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_00 ] = y262_motcomp_16x8_00_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_01 ] = y262_motcomp_16x8_01_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_10 ] = y262_motcomp_16x8_10_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_11 ] = y262_motcomp_16x8_11_avg;

	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_00 ] = y262_motcomp_8x16_00_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_01 ] = y262_motcomp_8x16_01_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_10 ] = y262_motcomp_8x16_10_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_11 ] = y262_motcomp_8x16_11_avg;

	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_00 ] = y262_motcomp_8x8_00_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_01 ] = y262_motcomp_8x8_01_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_10 ] = y262_motcomp_8x8_10_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_11 ] = y262_motcomp_8x8_11_avg;

	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_00 ] = y262_motcomp_8x4_00_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_01 ] = y262_motcomp_8x4_01_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_10 ] = y262_motcomp_8x4_10_avg;
	ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_11 ] = y262_motcomp_8x4_11_avg;

#if 1

	if( 1 )
	{
		/* copy */
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_00 ] = y262_motcomp_16x16_00_put_sse2;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_01 ] = y262_motcomp_16x16_01_put_sse2;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_10 ] = y262_motcomp_16x16_10_put_sse2;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ MC_BLOCK_11 ] = y262_motcomp_16x16_11_put_sse2;

		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_00 ] = y262_motcomp_16x8_00_put_sse2;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_01 ] = y262_motcomp_16x8_01_put_sse2;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_10 ] = y262_motcomp_16x8_10_put_sse2;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ MC_BLOCK_11 ] = y262_motcomp_16x8_11_put_sse2;
		
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_00 ] = y262_motcomp_8x16_00_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_01 ] = y262_motcomp_8x16_01_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_10 ] = y262_motcomp_8x16_10_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x16 ][ MC_BLOCK_11 ] = y262_motcomp_8x16_11_put_mmxext;

		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_00 ] = y262_motcomp_8x8_00_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_01 ] = y262_motcomp_8x8_01_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_10 ] = y262_motcomp_8x8_10_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x8 ][ MC_BLOCK_11 ] = y262_motcomp_8x8_11_put_mmxext;

		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_00 ] = y262_motcomp_8x4_00_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_01 ] = y262_motcomp_8x4_01_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_10 ] = y262_motcomp_8x4_10_put_mmxext;
		ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_8x4 ][ MC_BLOCK_11 ] = y262_motcomp_8x4_11_put_mmxext;
		


		/* avg */
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_00 ] = y262_motcomp_16x16_00_avg_sse2;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_01 ] = y262_motcomp_16x16_01_avg_sse2;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_10 ] = y262_motcomp_16x16_10_avg_sse2;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ MC_BLOCK_11 ] = y262_motcomp_16x16_11_avg_sse2;
		
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_00 ] = y262_motcomp_16x8_00_avg_sse2;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_01 ] = y262_motcomp_16x8_01_avg_sse2;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_10 ] = y262_motcomp_16x8_10_avg_sse2;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ MC_BLOCK_11 ] = y262_motcomp_16x8_11_avg_sse2;
		
		
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_00 ] = y262_motcomp_8x16_00_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_01 ] = y262_motcomp_8x16_01_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_10 ] = y262_motcomp_8x16_10_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x16 ][ MC_BLOCK_11 ] = y262_motcomp_8x16_11_avg_mmxext;

		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_00 ] = y262_motcomp_8x8_00_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_01 ] = y262_motcomp_8x8_01_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_10 ] = y262_motcomp_8x8_10_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x8 ][ MC_BLOCK_11 ] = y262_motcomp_8x8_11_avg_mmxext;

		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_00 ] = y262_motcomp_8x4_00_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_01 ] = y262_motcomp_8x4_01_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_10 ] = y262_motcomp_8x4_10_avg_mmxext;
		ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_8x4 ][ MC_BLOCK_11 ] = y262_motcomp_8x4_11_avg_mmxext;		
	}
#endif
}


