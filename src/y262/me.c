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


#define VALID_MV( x, y ) ( ( x ) >= i_min_mv_x && ( x ) <= i_max_mv_x && ( y ) >= i_min_mv_y && ( y ) <= i_max_mv_y )

#define MV_COST( x, y ) ( ( ps_y262->rgi_y262_motion_bits_x[ 2048 + ( x ) - i_pred_mv_x ] * i_lambda ) + ( ( ps_y262->rgi_y262_motion_bits_y[ 2048 + ( y ) - i_pred_mv_y ] * i_lambda ) ) )

#define TRY_MV( x, y )						\
do {										\
	uint8_t *pui8_mvref;					\
	pui8_mvref = pui8_ref + ( x ) + ( ( y ) * ps_ctx->i_ref_stride );					\
	i_sad = ps_y262->s_funcs.rgf_sad[ i_blk_type ]( pui8_mvref, i_ref_stride, pui8_blk, i_blk_stride );			\
	i_sad += MV_COST( ( x ) << 1, ( y ) << 1 );											\
	if( i_sad < i_best_sad )				\
	{										\
		i_best_sad = i_sad;					\
		i_best_mv_x = ( x );				\
		i_best_mv_y = ( y );				\
	}										\
} while( 0 )

bool_t y262_motion_search( y262_t *ps_y262, y262_me_context_t *ps_ctx )
{
	int32_t i_blk_stride, i_ref_stride, i_blk_type, i_idx, i_best_mv_x, i_best_mv_y, i_best_sad, i_mv_x, i_mv_y, i_scale, i_sad, i_ter;
	int32_t i_pred_mv_x, i_pred_mv_y;
	int32_t i_lambda;
	uint8_t *pui8_blk, *pui8_ref;
	int32_t i_min_mv_x, i_min_mv_y, i_max_mv_x, i_max_mv_y;
	int32_t i_omv_x, i_omv_y;
	
	static const int32_t rgi_exhaustive_pattern[ 8 ][ 2 ] = {
		{ -1, -1 }, { 0, -1 }, { 1, -1 },
		{ -1,  0 },            { 1,  0 },
		{ -1,  1 }, { 0,  1 }, { 1,  1 }
	};

	static const int32_t rgi_diamond[ 6 ][ 2 ] = {
		{ -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }
	};

	pui8_blk = ps_ctx->pui8_blk;
	i_blk_stride = ps_ctx->i_blk_stride;
	i_blk_type = ps_ctx->i_blk_type;
	i_ref_stride = ps_ctx->i_ref_stride;
	i_pred_mv_x = ps_ctx->i_pred_mv_x;
	i_pred_mv_y = ps_ctx->i_pred_mv_y;
	i_lambda = ps_ctx->i_lambda;

	i_min_mv_x = MAX( ps_ctx->i_min_mv_x, -ps_ctx->i_x_offset );
	i_max_mv_x = MIN( ps_ctx->i_max_mv_x, ps_ctx->i_ref_width - rgi_y262_block_type_dims[ ps_ctx->i_blk_type ][ 0 ] - ps_ctx->i_x_offset );
	i_min_mv_y = MAX( ps_ctx->i_min_mv_y, -ps_ctx->i_y_offset );
	i_max_mv_y = MIN( ps_ctx->i_max_mv_y, ps_ctx->i_ref_height - rgi_y262_block_type_dims[ ps_ctx->i_blk_type ][ 1 ] - ps_ctx->i_y_offset );

	pui8_ref = ps_ctx->pui8_ref + ps_ctx->i_x_offset + ( ps_ctx->i_y_offset * ps_ctx->i_ref_stride );

	i_best_sad = MAX_COST;
	i_best_mv_x = 0;
	i_best_mv_y = 0;
	TRY_MV( 0, 0 );

	i_mv_x = ( i_pred_mv_x + 1 ) >> 1;
	i_mv_y = ( i_pred_mv_y + 1 ) >> 1;

	if( VALID_MV( i_mv_x, i_mv_y ) )
	{
		TRY_MV( i_mv_x, i_mv_y );
	}

	for( i_idx = 0; i_idx < ps_ctx->i_num_candidates_fp; i_idx++ )
	{
		if( VALID_MV( ps_ctx->rgi_candidates_fp[ i_idx ][ 0 ] >> 1, ps_ctx->rgi_candidates_fp[ i_idx ][ 1 ] >> 1 ) )
		{
			TRY_MV( ps_ctx->rgi_candidates_fp[ i_idx ][ 0 ] >> 1, ps_ctx->rgi_candidates_fp[ i_idx ][ 1 ] >> 1 );
		}
	}

	if( ps_ctx->i_me_call == MECALL_LOOKAHEAD || ps_y262->i_quality_for_speed > -20 )
	{
		do
		{
			i_omv_x = i_best_mv_x;
			i_omv_y = i_best_mv_y;
			i_ter = 0;
			for( i_scale = 0; i_scale < 12; i_scale++ )
			{
				for( i_idx = 0; i_idx < 8; i_idx++ )
				{
					i_mv_x = i_omv_x + ( rgi_exhaustive_pattern[ i_idx ][ 0 ] << i_scale );
					i_mv_y = i_omv_y + ( rgi_exhaustive_pattern[ i_idx ][ 1 ] << i_scale );
					if( VALID_MV( i_mv_x, i_mv_y ) )
					{
						TRY_MV( i_mv_x, i_mv_y );
					}
				}

				i_ter++;
				if( i_omv_x != i_best_mv_x || i_omv_y != i_best_mv_y )
				{
					i_ter = 0;
				}
				else if( i_ter > 4 )
				{
					break;
				}
			}
		} while( ( i_omv_x != i_best_mv_x || i_omv_y != i_best_mv_y ) );
	}
	else
	{
		do
		{
			i_omv_x = i_best_mv_x;
			i_omv_y = i_best_mv_y;
			i_ter = 0;
			for( i_idx = 0; i_idx < 4; i_idx++ )
			{
				i_mv_x = i_omv_x + ( rgi_diamond[ i_idx ][ 0 ] );
				i_mv_y = i_omv_y + ( rgi_diamond[ i_idx ][ 1 ] );
				if( VALID_MV( i_mv_x, i_mv_y ) )
				{
					TRY_MV( i_mv_x, i_mv_y );
				}
			}
		} while( ( i_omv_x != i_best_mv_x || i_omv_y != i_best_mv_y ) );
	}

	i_best_mv_x <<= 1;
	i_best_mv_y <<= 1;
	ps_ctx->i_best_mv_x = i_best_mv_x;
	ps_ctx->i_best_mv_y = i_best_mv_y;
	ps_ctx->i_best_mv_sad = i_best_sad;

	/* hpel */
	y262_hpel_motion_search( ps_y262, ps_ctx );

	return TRUE;
}


#define TRY_HPEL_MV( x, y )																		\
do {																							\
	uint8_t *pui8_mvref;																		\
	int32_t i_hpelidx;																			\
	pui8_mvref = pui8_ref + ( ( x ) >> 1 ) + ( ( ( y ) >> 1 ) * i_ref_stride );					\
	i_hpelidx = ( ( x ) & 1 ) | ( ( ( y ) & 1 ) << 1 );											\
	if( i_hpelidx )																				\
	{																							\
		ps_y262->s_funcs.rgf_motcomp_copy[ i_mc_blk_type ][ i_hpelidx ]( pui8_mvref, i_ref_stride, rgui8_pred, 16 );	\
		i_sad = ps_y262->s_funcs.rgf_satd[ i_blk_type ]( pui8_blk, i_blk_stride, rgui8_pred, 16 );	\
	}																							\
	else																						\
	{																							\
		i_sad = ps_y262->s_funcs.rgf_satd[ i_blk_type ]( pui8_blk, i_blk_stride, pui8_mvref, i_ref_stride );	\
	}																							\
	i_sad += MV_COST( ( x ), ( y ) );															\
	if( i_sad < i_best_sad )																	\
	{																							\
		i_best_sad = i_sad;																		\
		i_best_mv_x = ( x );																	\
		i_best_mv_y = ( y );																	\
	}																							\
} while( 0 )


bool_t y262_hpel_motion_search( y262_t *ps_y262, y262_me_context_t *ps_ctx )
{
	int32_t i_blk_stride, i_ref_stride, i_blk_type, i_idx, i_best_mv_x, i_best_mv_y, i_best_sad, i_mv_x, i_mv_y, i_sad;
	int32_t i_pred_mv_x, i_pred_mv_y;
	int32_t i_lambda;
	uint8_t *pui8_blk, *pui8_ref;
	int32_t i_min_mv_x, i_min_mv_y, i_max_mv_x, i_max_mv_y;
	ALIGNED( 16 ) uint8_t rgui8_pred[ 16 * 16 ];
	uint8_t *pui8_pred;
	int32_t i_mc_blk_type;

	static const int32_t rgi_exhaustive_pattern[ 8 ][ 2 ] = {
		{ -1, -1 }, { 0, -1 }, { 1, -1 },
		{ -1,  0 },            { 1,  0 },
		{ -1,  1 }, { 0,  1 }, { 1,  1 }
	};

	pui8_blk = ps_ctx->pui8_blk;
	i_blk_stride = ps_ctx->i_blk_stride;
	i_blk_type = ps_ctx->i_blk_type;
	i_ref_stride = ps_ctx->i_ref_stride;
	i_pred_mv_x = ps_ctx->i_pred_mv_x;
	i_pred_mv_y = ps_ctx->i_pred_mv_y;
	i_lambda = ps_ctx->i_lambda;

	i_min_mv_x = MAX( ps_ctx->i_min_mv_x, -ps_ctx->i_x_offset );
	i_max_mv_x = MIN( ps_ctx->i_max_mv_x, ps_ctx->i_ref_width - rgi_y262_block_type_dims[ ps_ctx->i_blk_type ][ 0 ] - ps_ctx->i_x_offset );
	i_min_mv_y = MAX( ps_ctx->i_min_mv_y, -ps_ctx->i_y_offset );
	i_max_mv_y = MIN( ps_ctx->i_max_mv_y, ps_ctx->i_ref_height - rgi_y262_block_type_dims[ ps_ctx->i_blk_type ][ 1 ] - ps_ctx->i_y_offset );

	pui8_ref = ps_ctx->pui8_ref + ps_ctx->i_x_offset + ( ps_ctx->i_y_offset * ps_ctx->i_ref_stride );

	/* hpel */
	i_min_mv_x <<= 1;
	i_min_mv_y <<= 1;
	i_max_mv_x <<= 1;
	i_max_mv_y <<= 1;

	if( i_blk_type == BLOCK_TYPE_16x16 )
	{
		i_mc_blk_type = MC_BLOCK_16x16;
	}
	else if( i_blk_type == BLOCK_TYPE_16x8 )
	{
		i_mc_blk_type = MC_BLOCK_16x8;
	}
	else
	{
		assert( FALSE );
	}
		
	pui8_pred = pui8_ref + ( ps_ctx->i_best_mv_x >> 1 ) + ( ( ps_ctx->i_best_mv_y >> 1 ) * i_ref_stride );
	i_best_sad = MAX_COST;
	i_best_mv_x = 0;
	i_best_mv_y = 0;
	
	if( VALID_MV( ps_ctx->i_best_mv_x, ps_ctx->i_best_mv_y ) )
	{
		TRY_HPEL_MV( ps_ctx->i_best_mv_x, ps_ctx->i_best_mv_y );
	}

	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		i_mv_x = ps_ctx->i_best_mv_x + rgi_exhaustive_pattern[ i_idx ][ 0 ];
		i_mv_y = ps_ctx->i_best_mv_y + rgi_exhaustive_pattern[ i_idx ][ 1 ];

		if( VALID_MV( i_mv_x, i_mv_y ) )
		{
			TRY_HPEL_MV( i_mv_x, i_mv_y );
		}
	}
	ps_ctx->i_best_mv_x = i_best_mv_x;
	ps_ctx->i_best_mv_y = i_best_mv_y;
	ps_ctx->i_best_mv_sad = i_best_sad;

	return TRUE;
}








