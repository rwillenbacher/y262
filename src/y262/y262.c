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



int32_t y262_get_mbmode_cost( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mbmode );


void y262_error( y262_t *ps_y262, int32_t i_error_code, int8_t* pi8_format, ... )
{
	if( ps_y262->s_funcs.pf_error_callback )
	{
		va_list p_va;
		int8_t rgi8_tmpbuffer[ 0x2000 ];

		va_start( p_va, pi8_format );

		vsnprintf( rgi8_tmpbuffer, 0x2000, pi8_format, p_va );

		ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, i_error_code, rgi8_tmpbuffer );
	}
}



void y262_slice_reset_predictors_intra( y262_t *ps_y262, y262_slice_t *ps_slice )
{
	ps_slice->rgi_dc_dct_pred[ 0 ] = 1 << ( 7 + ps_y262->i_intra_dc_precision );
	ps_slice->rgi_dc_dct_pred[ 1 ] = 1 << ( 7 + ps_y262->i_intra_dc_precision );
	ps_slice->rgi_dc_dct_pred[ 2 ] = 1 << ( 7 + ps_y262->i_intra_dc_precision );
}

void y262_slice_reset_predictors_inter( y262_t *ps_y262, y262_slice_t *ps_slice )
{
	ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ] = 0;
	ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ] = 0;
	ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ] = 0;
	ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ] = 0;
	ps_slice->rgi_pmv[ 1 ][ 0 ][ 0 ] = 0;
	ps_slice->rgi_pmv[ 1 ][ 0 ][ 1 ] = 0;
	ps_slice->rgi_pmv[ 1 ][ 1 ][ 0 ] = 0;
	ps_slice->rgi_pmv[ 1 ][ 1 ][ 1 ] = 0;
}


void y262_init_macroblock( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mb_idx )
{
	int32_t i_pel_x, i_pel_y, i_chroma_pel_x, i_chroma_pel_y, i_stride_chroma;
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;
	ps_mb->i_mb_addr = i_mb_idx;

	i_pel_x = ( i_mb_idx % ( ps_y262->i_sequence_width >> 4 ) ) << 4;
	i_pel_y = ( i_mb_idx / ( ps_y262->i_sequence_width >> 4 ) ) << 4;

	ps_mb->pui8_src_luma = ps_y262->ps_input_picture->pui8_luma + i_pel_x + ( i_pel_y * ps_y262->i_sequence_width );
	ps_mb->i_src_luma_stride = ps_y262->i_sequence_width;

	ps_mb->pui8_dst_luma = ps_y262->ps_refpic_dst->pui8_luma + i_pel_x + ( i_pel_y * ps_y262->ps_refpic_dst->i_stride_luma );
	ps_mb->i_dst_luma_stride = ps_y262->ps_refpic_dst->i_stride_luma;

	ps_mb->i_mb_x = i_pel_x;
	ps_mb->i_mb_y = i_pel_y;

	switch( ps_y262->i_sequence_chroma_format )
	{
		case Y262_CHROMA_FORMAT_420:
			i_chroma_pel_x = i_pel_x >> 1;
			i_chroma_pel_y = i_pel_y >> 1;
			break;
		case Y262_CHROMA_FORMAT_422:
			i_chroma_pel_x = i_pel_x >> 1;
			i_chroma_pel_y = i_pel_y;
			break;
		case Y262_CHROMA_FORMAT_444:
			i_chroma_pel_x = i_pel_x;
			i_chroma_pel_y = i_pel_y;
			break;
	}
	i_stride_chroma = ps_y262->i_sequence_chroma_width;

	ps_mb->pui8_src_cb = ps_y262->ps_input_picture->pui8_cb + i_chroma_pel_x + ( i_chroma_pel_y * i_stride_chroma );
	ps_mb->pui8_src_cr = ps_y262->ps_input_picture->pui8_cr + i_chroma_pel_x + ( i_chroma_pel_y * i_stride_chroma );
	ps_mb->i_src_chroma_stride = i_stride_chroma;

	ps_mb->pui8_dst_cb = ps_y262->ps_refpic_dst->pui8_cb + i_chroma_pel_x + ( i_chroma_pel_y * ps_y262->ps_refpic_dst->i_stride_chroma );
	ps_mb->pui8_dst_cr = ps_y262->ps_refpic_dst->pui8_cr + i_chroma_pel_x + ( i_chroma_pel_y * ps_y262->ps_refpic_dst->i_stride_chroma );
	ps_mb->i_dst_chroma_stride = ps_y262->ps_refpic_dst->i_stride_chroma;


}


uint8_t *y262_blk_pointer_adjust( y262_t *ps_y262, uint8_t *pui8_ptr, int32_t i_stride, int32_t i_plane_idx, int32_t i_blk_idx, bool_t b_interlaced, int32_t *pi_adjusted_stride )
{
	int32_t i_blk_stride;
	uint8_t *pui8_blk;

	if( !b_interlaced )
	{
		if( i_plane_idx == 0 )
		{
			i_blk_stride = i_stride;
			pui8_blk = pui8_ptr + rgui_y262_luma_blk_offsets[ i_blk_idx ][ 0 ] + ( rgui_y262_luma_blk_offsets[ i_blk_idx ][ 1 ] * i_blk_stride );
		}
		else
		{
			i_blk_stride = i_stride;
			pui8_blk = pui8_ptr + rgui_y262_chroma_blk_offsets[ ps_y262->i_sequence_chroma_format ][ i_blk_idx ][ 0 ] + ( rgui_y262_chroma_blk_offsets[ ps_y262->i_sequence_chroma_format ][ i_blk_idx ][ 1 ] * i_blk_stride );
		}
	}
	else
	{
		if( i_plane_idx == 0 )
		{
			pui8_blk = pui8_ptr + rgui_y262_luma_il_blk_offsets[ i_blk_idx ][ 0 ] + ( rgui_y262_luma_il_blk_offsets[ i_blk_idx ][ 1 ] * i_stride );
			i_blk_stride = i_stride * 2;
		}
		else
		{
			i_blk_stride = i_stride;
			pui8_blk = pui8_ptr + rgui_y262_chroma_il_blk_offsets[ ps_y262->i_sequence_chroma_format ][ i_blk_idx ][ 0 ] + ( rgui_y262_chroma_il_blk_offsets[ ps_y262->i_sequence_chroma_format ][ i_blk_idx ][ 1 ] * i_blk_stride );
			switch( ps_y262->i_sequence_chroma_format )
			{
				case Y262_CHROMA_FORMAT_420:
					i_blk_stride = i_stride;
					break;
				case Y262_CHROMA_FORMAT_422:
				case Y262_CHROMA_FORMAT_444:
				default:
					i_blk_stride = i_stride * 2;
					break;
			}
		}
	}
	*pi_adjusted_stride = i_blk_stride;
	return pui8_blk;
}

void y262_encode_macroblock_intra( y262_t *ps_y262, y262_slice_t *ps_slice, bool_t b_interlaced )
{
	int32_t i_blk_idx, i_x, i_y, i_plane_idx, i_num_blocks, i_chroma_format;
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;
	i_chroma_format = ps_y262->i_sequence_chroma_format;

	ps_mb->i_macroblock_type = MACROBLOCK_INTRA;
	if( b_interlaced )
	{
		ps_mb->i_macroblock_type |= MACROBLOCK_INTERLACED;
	}

	ps_mb->i_cbp = 0;
	for( i_plane_idx = 0; i_plane_idx < 3; i_plane_idx++ )
	{
		i_num_blocks = i_plane_idx == 0 ? 4 : rgui_num_chroma_blk[ ps_y262->i_sequence_chroma_format ];
		for( i_blk_idx = 0; i_blk_idx < i_num_blocks; i_blk_idx++ )
		{
			uint8_t *pui8_blk;
			int32_t i_blk_stride;

			switch( i_plane_idx )
			{
				case 0:
					pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, i_plane_idx, i_blk_idx, b_interlaced, &i_blk_stride );
					break;
				case 1:
					pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_cb, ps_mb->i_src_chroma_stride, i_plane_idx, i_blk_idx, b_interlaced, &i_blk_stride );
					break;
				case 2:
					pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_cr, ps_mb->i_src_chroma_stride, i_plane_idx, i_blk_idx, b_interlaced, &i_blk_stride );
					break;
			}

			for( i_y = 0; i_y < 8; i_y++ )
			{
				for( i_x = 0; i_x < 8; i_x++ )
				{
					ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ] = pui8_blk[ i_x + i_y * i_blk_stride ];
				}
			}
			ps_y262->s_funcs.f_fdct_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ] );

			ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ 0 ] = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ 0 ] >> ( 3 - ps_y262->i_intra_dc_precision );
			if( ps_y262->i_quality_for_speed < 0 )
			{
				y262_quant8x8_intra_fw( ps_y262, ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ], 8, ps_y262->rgui16_intra_quantizer_matrices[ ps_mb->i_scaled_quantizer ], ps_y262->rgui16_intra_quantizer_matrices_bias[ ps_mb->i_scaled_quantizer ] );
			}
			else
			{
				y262_quant8x8_trellis_fw( ps_y262, ps_slice, ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ], 8, ps_mb->i_scaled_quantizer, TRUE );
			}


			ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] = TRUE;
			ps_mb->i_cbp |= 1 << ( i_plane_idx * 4 + i_blk_idx );

			for( i_y = 0; i_y < 8; i_y++ )
			{
				for( i_x = 0; i_x < 8; i_x++ )
				{
					ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ] = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ];
				}
			}

			ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ 0 ] = ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ 0 ] << ( 3 - ps_y262->i_intra_dc_precision );
			y262_quant8x8_intra_bw( ps_y262, ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], 8, ps_mb->i_scaled_quantizer, ps_y262->rgui8_intra_quantiser_matrix );

			ps_y262->s_funcs.f_idct_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ] );

			switch( i_plane_idx )
			{
				case 0:
					pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_dst_luma, ps_mb->i_dst_luma_stride, i_plane_idx, i_blk_idx, b_interlaced, &i_blk_stride );
					break;
				case 1:
					pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_dst_cb, ps_mb->i_dst_chroma_stride, i_plane_idx, i_blk_idx, b_interlaced, &i_blk_stride );
					break;
				case 2:
					pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_dst_cr, ps_mb->i_dst_chroma_stride, i_plane_idx, i_blk_idx, b_interlaced, &i_blk_stride );
					break;
			}

			for( i_y = 0; i_y < 8; i_y++ )
			{
				for( i_x = 0; i_x < 8; i_x++ )
				{
					int32_t i_rec;
					i_rec = ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ];
					pui8_blk[ i_x + i_y * i_blk_stride ] = i_rec < 0 ? 0 : ( i_rec > 255 ? 255 : i_rec );
				}
			}
		}
	}
}


int32_t y262_get_inter_block_bits( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx );

bool_t y262_encode_macroblock_inter( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mbmode )
{
	int32_t i_blk_idx, i_field_idx, i_idx, i_x, i_y, i_plane_idx, i_num_blocks, i_num_preds, i_blk_type, i_chroma_blk_type;
	int32_t i_chroma_mv_x, i_chroma_mv_y;
	int32_t rgi_strides[ 3 ][ 2 ], rgi_dst_strides[ 3 ], i_shift_chroma_x, i_shift_chroma_y;
	uint8_t *rgpui8_src[ 3 ][ 2 ], *rgpui8_dst[ 3 ];
	y262_macroblock_t *ps_mb;
	y262_reference_picture_t *rgps_refs[ 2 ];
	y262_mv_t *rgps_mvs[ 2 ], s_skip_mvs[ 2 ];
	bool_t b_skip_cbf = FALSE;

	ps_mb = &ps_slice->s_macroblock;

	if( i_mbmode == Y262_MBMODE_SKIP || i_mbmode == Y262_MBMODE_FW || i_mbmode == Y262_MBMODE_BW || i_mbmode == Y262_MBMODE_BI )
	{
		i_blk_type = MC_BLOCK_16x16;
		switch( ps_y262->i_sequence_chroma_format )
		{
			case Y262_CHROMA_FORMAT_420:
				i_chroma_blk_type = MC_BLOCK_8x8;
				i_shift_chroma_x = 1;
				i_shift_chroma_y = 1;
				break;
			case Y262_CHROMA_FORMAT_422:
				i_chroma_blk_type = MC_BLOCK_8x16;
				i_shift_chroma_x = 1;
				i_shift_chroma_y = 0;
				break;
			case Y262_CHROMA_FORMAT_444:
				i_chroma_blk_type = MC_BLOCK_16x16;
				i_shift_chroma_x = 0;
				i_shift_chroma_y = 0;
				break;
		}

		rgi_dst_strides[ 0 ] = ps_y262->ps_refpic_dst->i_stride_luma;
		rgi_dst_strides[ 1 ] = ps_y262->ps_refpic_dst->i_stride_chroma;
		rgi_dst_strides[ 2 ] = ps_y262->ps_refpic_dst->i_stride_chroma;
		rgpui8_dst[ 0 ] = ps_y262->ps_refpic_dst->pui8_luma + ps_mb->i_mb_x + ( ps_mb->i_mb_y * rgi_dst_strides[ 0 ] );
		rgpui8_dst[ 1 ] = ps_y262->ps_refpic_dst->pui8_cb + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgi_dst_strides[ 1 ] );
		rgpui8_dst[ 2 ] = ps_y262->ps_refpic_dst->pui8_cr + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgi_dst_strides[ 2 ] );

		if( i_mbmode == Y262_MBMODE_FW )
		{
			ps_mb->i_macroblock_type = MACROBLOCK_MOTION_FORWARD;
			ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FRAME * MACROBLOCK_MOTION_TYPE;
			i_num_preds = 1;
			rgps_refs[ 0 ] = ps_y262->ps_refpic_forward;
			rgps_mvs[ 0 ] = &ps_slice->s_mode_decision.s_fw_mv;
			ps_mb->rgs_motion[ 0 ][ 0 ] = ps_slice->s_mode_decision.s_fw_mv;
		}
		else if( i_mbmode == Y262_MBMODE_BW )
		{
			ps_mb->i_macroblock_type = MACROBLOCK_MOTION_BACKWARD;
			ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FRAME * MACROBLOCK_MOTION_TYPE;
			i_num_preds = 1;
			rgps_refs[ 0 ] = ps_y262->ps_refpic_backward;
			rgps_mvs[ 0 ] = &ps_slice->s_mode_decision.s_bw_mv;
			ps_mb->rgs_motion[ 0 ][ 1 ] = ps_slice->s_mode_decision.s_bw_mv;
		}
		else if( i_mbmode == Y262_MBMODE_BI )
		{
			ps_mb->i_macroblock_type = MACROBLOCK_MOTION_FORWARD | MACROBLOCK_MOTION_BACKWARD;
			ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FRAME * MACROBLOCK_MOTION_TYPE;
			i_num_preds = 2;
			rgps_refs[ 0 ] = ps_y262->ps_refpic_forward;
			rgps_mvs[ 0 ] = &ps_slice->s_mode_decision.s_bi_mv[ 0 ];
			rgps_refs[ 1 ] = ps_y262->ps_refpic_backward;
			rgps_mvs[ 1 ] = &ps_slice->s_mode_decision.s_bi_mv[ 1 ];
			ps_mb->rgs_motion[ 0 ][ 0 ] = ps_slice->s_mode_decision.s_bi_mv[ 0 ];
			ps_mb->rgs_motion[ 0 ][ 1 ] = ps_slice->s_mode_decision.s_bi_mv[ 1 ];
		}
		else if( i_mbmode == Y262_MBMODE_SKIP )
		{
			if( ps_slice->i_picture_type == PICTURE_CODING_TYPE_P )
			{
				ps_mb->i_macroblock_type = MACROBLOCK_MOTION_FORWARD;
				ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FRAME * MACROBLOCK_MOTION_TYPE;
				i_num_preds = 1;
				rgps_refs[ 0 ] = ps_y262->ps_refpic_forward;
				s_skip_mvs[ 0 ].i_x = 0;
				s_skip_mvs[ 0 ].i_y = 0;
				rgps_mvs[ 0 ] = &s_skip_mvs[ 0 ];
				ps_mb->rgs_motion[ 0 ][ 0 ] = s_skip_mvs[ 0 ];
			}
			else
			{
				assert( ps_slice->i_picture_type == PICTURE_CODING_TYPE_B );
				i_num_preds = 0;
				ps_mb->i_macroblock_type = 0;
				if( ps_slice->i_last_mb_motion_flags & MACROBLOCK_MOTION_FORWARD )
				{
					ps_mb->i_macroblock_type |= MACROBLOCK_MOTION_FORWARD;
					ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FRAME * MACROBLOCK_MOTION_TYPE;
					rgps_refs[ i_num_preds ] = ps_y262->ps_refpic_forward;
					s_skip_mvs[ i_num_preds ].i_x = ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ];
					s_skip_mvs[ i_num_preds ].i_y = ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ];
					rgps_mvs[ i_num_preds ] = &s_skip_mvs[ i_num_preds ];
					ps_mb->rgs_motion[ 0 ][ 0 ] = s_skip_mvs[ i_num_preds ];
					i_num_preds++;
				}
				if( ps_slice->i_last_mb_motion_flags & MACROBLOCK_MOTION_BACKWARD )
				{
					ps_mb->i_macroblock_type |= MACROBLOCK_MOTION_BACKWARD;
					ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FRAME * MACROBLOCK_MOTION_TYPE;
					rgps_refs[ i_num_preds ] = ps_y262->ps_refpic_backward;
					s_skip_mvs[ i_num_preds ].i_x = ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ];
					s_skip_mvs[ i_num_preds ].i_y = ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ];
					rgps_mvs[ i_num_preds ] = &s_skip_mvs[ i_num_preds ];
					ps_mb->rgs_motion[ 0 ][ 1 ] = s_skip_mvs[ i_num_preds ];
					i_num_preds++;
				}
			}
		}
		else
		{
			assert( FALSE );
		}

		
	
		for( i_idx = 0; i_idx < i_num_preds; i_idx++ )
		{
			int32_t i_hpel_idx, i_chroma_hpel_idx;

			rgi_strides[ 0 ][ 0 ] = rgps_refs[ i_idx ]->i_stride_luma;
			rgi_strides[ 1 ][ 0 ] = rgps_refs[ i_idx ]->i_stride_chroma;
			rgi_strides[ 2 ][ 0 ] = rgps_refs[ i_idx ]->i_stride_chroma;
			rgpui8_src[ 0 ][ 0 ] = rgps_refs[ i_idx ]->pui8_luma + ps_mb->i_mb_x + ( ps_mb->i_mb_y * rgi_strides[ 0 ][ 0 ] );
			rgpui8_src[ 1 ][ 0 ] = rgps_refs[ i_idx ]->pui8_cb + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgi_strides[ 1 ][ 0 ] );
			rgpui8_src[ 2 ][ 0 ] = rgps_refs[ i_idx ]->pui8_cr + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgi_strides[ 2 ][ 0 ] );

			i_hpel_idx = ( rgps_mvs[ i_idx ]->i_x & 1 ) | ( ( rgps_mvs[ i_idx ]->i_y & 1 ) << 1 );
			rgpui8_src[ 0 ][ 0 ] += ( rgps_mvs[ i_idx ]->i_x >> 1 ) + ( ( rgps_mvs[ i_idx ]->i_y >> 1 ) * rgi_strides[ 0 ][ 0 ] );

			i_chroma_mv_x = ( rgps_mvs[ i_idx ]->i_x / ( 1 << i_shift_chroma_x ) );
			i_chroma_mv_y = ( rgps_mvs[ i_idx ]->i_y / ( 1 << i_shift_chroma_y ) );
			i_chroma_hpel_idx = ( i_chroma_mv_x & 1 ) | ( ( i_chroma_mv_y & 1 ) << 1 );
			rgpui8_src[ 1 ][ 0 ] += ( i_chroma_mv_x >> 1 ) + ( ( i_chroma_mv_y >> 1 ) * rgi_strides[ 1 ][ 0 ] );
			rgpui8_src[ 2 ][ 0 ] += ( i_chroma_mv_x >> 1 ) + ( ( i_chroma_mv_y >> 1 ) * rgi_strides[ 1 ][ 0 ] );

			if( i_idx == 0 )
			{
				ps_y262->s_funcs.rgf_motcomp_copy[ i_blk_type ][ i_hpel_idx ]( rgpui8_src[ 0 ][ 0 ], rgi_strides[ 0 ][ 0 ], rgpui8_dst[ 0 ], rgi_dst_strides[ 0 ] );
				ps_y262->s_funcs.rgf_motcomp_copy[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 1 ][ 0 ], rgi_strides[ 1 ][ 0 ], rgpui8_dst[ 1 ], rgi_dst_strides[ 1 ] );
				ps_y262->s_funcs.rgf_motcomp_copy[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 2 ][ 0 ], rgi_strides[ 2 ][ 0 ], rgpui8_dst[ 2 ], rgi_dst_strides[ 2 ] );
			}
			else
			{
				ps_y262->s_funcs.rgf_motcomp_avg[ i_blk_type ][ i_hpel_idx ]( rgpui8_src[ 0 ][ 0 ], rgi_strides[ 0 ][ 0 ], rgpui8_dst[ 0 ], rgi_dst_strides[ 0 ] );
				ps_y262->s_funcs.rgf_motcomp_avg[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 1 ][ 0 ], rgi_strides[ 1 ][ 0 ], rgpui8_dst[ 1 ], rgi_dst_strides[ 1 ] );
				ps_y262->s_funcs.rgf_motcomp_avg[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 2 ][ 0 ], rgi_strides[ 2 ][ 0 ], rgpui8_dst[ 2 ], rgi_dst_strides[ 2 ] );
			}
		}
		ps_mb->i_cbp = 0;
		for( i_plane_idx = 0; i_plane_idx < 3; i_plane_idx++ )
		{
			i_num_blocks = i_plane_idx == 0 ? 4 : rgui_num_chroma_blk[ ps_y262->i_sequence_chroma_format ];
			for( i_blk_idx = 0; i_blk_idx < i_num_blocks; i_blk_idx++ )
			{
				uint8_t *pui8_src_blk, *pui8_dst_blk, *pui8_pred;
				int32_t i_src_blk_stride, i_dst_blk_stride, i_pred_stride, i_zdist, i_cdist;
				uint8_t rgui8_recon[ 8 * 8 ];

				if( i_mbmode == Y262_MBMODE_SKIP && b_skip_cbf )
				{
					continue;
				}

				pui8_pred = y262_blk_pointer_adjust( ps_y262, rgpui8_dst[ i_plane_idx ], rgi_dst_strides[ i_plane_idx ], i_plane_idx, i_blk_idx, FALSE, &i_pred_stride );

				switch( i_plane_idx )
				{
					case 0:
						pui8_src_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, i_plane_idx, i_blk_idx, FALSE, &i_src_blk_stride );
						break;
					case 1:
						pui8_src_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_cb, ps_mb->i_src_chroma_stride, i_plane_idx, i_blk_idx, FALSE, &i_src_blk_stride );
						break;
					case 2:
						pui8_src_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_cr, ps_mb->i_src_chroma_stride, i_plane_idx, i_blk_idx, FALSE, &i_src_blk_stride );
						break;
				}

				if( ps_y262->i_quality_for_speed >= 8 )
				{
					i_zdist = ps_y262->s_funcs.f_ssd_8x8( pui8_src_blk, i_src_blk_stride, pui8_pred, i_pred_stride );
				}

				ps_y262->s_funcs.f_sub_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], pui8_src_blk, i_src_blk_stride, pui8_pred, i_pred_stride );

				ps_y262->s_funcs.f_fdct_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ] );
				if( ps_y262->i_quality_for_speed < 0 )
				{
					ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] = !!y262_quant8x8_inter_fw( ps_y262, ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ], 8, ps_y262->rgui16_non_intra_quantizer_matrices[ ps_mb->i_scaled_quantizer ] );
				}
				else
				{
					ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] = !!y262_quant8x8_trellis_fw( ps_y262, ps_slice, ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ], 8, ps_mb->i_scaled_quantizer, FALSE );
				}

				if( i_mbmode == Y262_MBMODE_SKIP )
				{
					if( ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] )
					{
						ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] = FALSE;
						b_skip_cbf = TRUE;
					}
					continue;
				}

				if( ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] )
				{
					for( i_y = 0; i_y < 8; i_y++ )
					{
						for( i_x = 0; i_x < 8; i_x++ )
						{
							ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ] = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ];
						}
					}

					y262_quant8x8_inter_bw( ps_y262, ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], 8, ps_mb->i_scaled_quantizer, ps_y262->rgui8_non_intra_quantiser_matrix );
					ps_y262->s_funcs.f_idct_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ] );

					pui8_dst_blk = y262_blk_pointer_adjust( ps_y262, rgpui8_dst[ i_plane_idx ], rgi_dst_strides[ i_plane_idx ], i_plane_idx, i_blk_idx, FALSE, &i_dst_blk_stride );

					if( ps_y262->i_quality_for_speed >= 8 )
					{
						ps_y262->s_funcs.f_add_8x8( rgui8_recon, 8, pui8_dst_blk, i_dst_blk_stride, ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ] );

						i_cdist = ps_y262->s_funcs.f_ssd_8x8( pui8_src_blk, i_src_blk_stride, rgui8_recon, 8 );
						i_cdist += ( ps_mb->i_lambda * ( y262_get_inter_block_bits( ps_y262, ps_slice, i_plane_idx, i_blk_idx ) + 1 ) ) >> Y262_LAMBDA_BITS;
						if( i_cdist <= i_zdist )
						{
							for( i_y = 0; i_y < 8; i_y++ )
							{
								memcpy( pui8_dst_blk + ( i_dst_blk_stride * i_y ), &rgui8_recon[ i_y * 8 ], sizeof( uint8_t ) * 8 );
							}
							ps_mb->i_cbp |= ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] << ( i_plane_idx * 4 + i_blk_idx );				
						}
						else
						{
							ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] = 0;
						}
					}
					else
					{
						ps_y262->s_funcs.f_add_8x8( pui8_dst_blk, i_dst_blk_stride, pui8_dst_blk, i_dst_blk_stride, ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ] );
						ps_mb->i_cbp |= ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] << ( i_plane_idx * 4 + i_blk_idx );				
					}
				}
			}
		}
	}
	else if( i_mbmode == Y262_MBMODE_FW_IL || i_mbmode == Y262_MBMODE_BW_IL || i_mbmode == Y262_MBMODE_BI_IL )
	{
		i_blk_type = MC_BLOCK_16x8;
		switch( ps_y262->i_sequence_chroma_format )
		{
			case Y262_CHROMA_FORMAT_420:
				i_chroma_blk_type = MC_BLOCK_8x4;
				i_shift_chroma_x = 1;
				i_shift_chroma_y = 1;
				break;
			case Y262_CHROMA_FORMAT_422:
				i_chroma_blk_type = MC_BLOCK_8x8;
				i_shift_chroma_x = 1;
				i_shift_chroma_y = 0;
				break;
			case Y262_CHROMA_FORMAT_444:
				i_chroma_blk_type = MC_BLOCK_16x8;
				i_shift_chroma_x = 0;
				i_shift_chroma_y = 0;
				break;
		}

		rgi_dst_strides[ 0 ] = ps_y262->ps_refpic_dst->i_stride_luma;
		rgi_dst_strides[ 1 ] = ps_y262->ps_refpic_dst->i_stride_chroma;
		rgi_dst_strides[ 2 ] = ps_y262->ps_refpic_dst->i_stride_chroma;
		rgpui8_dst[ 0 ] = ps_y262->ps_refpic_dst->pui8_luma + ps_mb->i_mb_x + ( ps_mb->i_mb_y * rgi_dst_strides[ 0 ] );
		rgpui8_dst[ 1 ] = ps_y262->ps_refpic_dst->pui8_cb + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgi_dst_strides[ 1 ] );
		rgpui8_dst[ 2 ] = ps_y262->ps_refpic_dst->pui8_cr + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgi_dst_strides[ 2 ] );

		for( i_field_idx = 0; i_field_idx < 2; i_field_idx++ )
		{
			if( i_mbmode == Y262_MBMODE_FW_IL )
			{
				ps_mb->i_macroblock_type = MACROBLOCK_MOTION_FORWARD | MACROBLOCK_INTERLACED;
				ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FIELD * MACROBLOCK_MOTION_TYPE;
				i_num_preds = 1;
				rgps_refs[ 0 ] = ps_y262->ps_refpic_forward;
				rgps_mvs[ 0 ] = &ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ];
				ps_mb->rgs_motion[ i_field_idx ][ 0 ] = ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ];
			}
			else if( i_mbmode == Y262_MBMODE_BW_IL )
			{
				ps_mb->i_macroblock_type = MACROBLOCK_MOTION_BACKWARD | MACROBLOCK_INTERLACED;
				ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FIELD * MACROBLOCK_MOTION_TYPE;
				i_num_preds = 1;
				rgps_refs[ 0 ] = ps_y262->ps_refpic_backward;
				rgps_mvs[ 0 ] = &ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ];
				ps_mb->rgs_motion[ i_field_idx ][ 1 ] = ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ];
			}
			else if( i_mbmode == Y262_MBMODE_BI_IL )
			{
				ps_mb->i_macroblock_type = MACROBLOCK_MOTION_FORWARD | MACROBLOCK_MOTION_BACKWARD | MACROBLOCK_INTERLACED;
				ps_mb->i_macroblock_type |= FRAME_MOTION_TYPE_FIELD * MACROBLOCK_MOTION_TYPE;
				i_num_preds = 2;
				rgps_refs[ 0 ] = ps_y262->ps_refpic_forward;
				rgps_mvs[ 0 ] = &ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 0 ];
				rgps_refs[ 1 ] = ps_y262->ps_refpic_backward;
				rgps_mvs[ 1 ] = &ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 1 ];
				ps_mb->rgs_motion[ i_field_idx ][ 0 ] = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 0 ];
				ps_mb->rgs_motion[ i_field_idx ][ 1 ] = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 1 ];
			}
			else
			{
				assert( FALSE );
			}

		
	
			for( i_idx = 0; i_idx < i_num_preds; i_idx++ )
			{
				int32_t i_hpel_idx, i_chroma_hpel_idx;

				rgi_strides[ 0 ][ 0 ] = rgps_refs[ i_idx ]->i_stride_luma << 1;
				rgi_strides[ 1 ][ 0 ] = rgps_refs[ i_idx ]->i_stride_chroma << 1;
				rgi_strides[ 2 ][ 0 ] = rgps_refs[ i_idx ]->i_stride_chroma << 1;
				rgpui8_src[ 0 ][ 0 ] = rgps_refs[ i_idx ]->pui8_luma + ps_mb->i_mb_x + ( ps_mb->i_mb_y * rgps_refs[ i_idx ]->i_stride_luma );
				rgpui8_src[ 1 ][ 0 ] = rgps_refs[ i_idx ]->pui8_cb + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgps_refs[ i_idx ]->i_stride_chroma );
				rgpui8_src[ 2 ][ 0 ] = rgps_refs[ i_idx ]->pui8_cr + ( ps_mb->i_mb_x >> i_shift_chroma_x ) + ( ( ps_mb->i_mb_y >> i_shift_chroma_y ) * rgps_refs[ i_idx ]->i_stride_chroma );

				rgpui8_src[ 0 ][ 0 ] += rgps_mvs[ i_idx ]->i_field * rgps_refs[ i_idx ]->i_stride_luma;
				rgpui8_src[ 1 ][ 0 ] += rgps_mvs[ i_idx ]->i_field * rgps_refs[ i_idx ]->i_stride_chroma;
				rgpui8_src[ 2 ][ 0 ] += rgps_mvs[ i_idx ]->i_field * rgps_refs[ i_idx ]->i_stride_chroma;

				i_hpel_idx = ( rgps_mvs[ i_idx ]->i_x & 1 ) | ( ( rgps_mvs[ i_idx ]->i_y & 1 ) << 1 );
				rgpui8_src[ 0 ][ 0 ] += ( rgps_mvs[ i_idx ]->i_x >> 1 ) + ( ( rgps_mvs[ i_idx ]->i_y >> 1 ) * rgi_strides[ 0 ][ 0 ] );

				i_chroma_mv_x = ( rgps_mvs[ i_idx ]->i_x / ( 1 << i_shift_chroma_x ) );
				i_chroma_mv_y = ( rgps_mvs[ i_idx ]->i_y / ( 1 << i_shift_chroma_y ) );
				i_chroma_hpel_idx = ( i_chroma_mv_x & 1 ) | ( ( i_chroma_mv_y & 1 ) << 1 );
				rgpui8_src[ 1 ][ 0 ] += ( i_chroma_mv_x >> 1 ) + ( ( i_chroma_mv_y >> 1 ) * rgi_strides[ 1 ][ 0 ] );
				rgpui8_src[ 2 ][ 0 ] += ( i_chroma_mv_x >> 1 ) + ( ( i_chroma_mv_y >> 1 ) * rgi_strides[ 2 ][ 0 ] );

				if( i_idx == 0 )
				{
					ps_y262->s_funcs.rgf_motcomp_copy[ i_blk_type ][ i_hpel_idx ]( rgpui8_src[ 0 ][ 0 ], rgi_strides[ 0 ][ 0 ], rgpui8_dst[ 0 ] + ( i_field_idx * rgi_dst_strides[ 0 ] ), rgi_dst_strides[ 0 ] << 1 );
					ps_y262->s_funcs.rgf_motcomp_copy[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 1 ][ 0 ], rgi_strides[ 1 ][ 0 ], rgpui8_dst[ 1 ] + ( i_field_idx * rgi_dst_strides[ 1 ] ), rgi_dst_strides[ 1 ] << 1 );
					ps_y262->s_funcs.rgf_motcomp_copy[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 2 ][ 0 ], rgi_strides[ 2 ][ 0 ], rgpui8_dst[ 2 ] + ( i_field_idx * rgi_dst_strides[ 2 ] ), rgi_dst_strides[ 2 ] << 1 );
				}
				else
				{
					ps_y262->s_funcs.rgf_motcomp_avg[ i_blk_type ][ i_hpel_idx ]( rgpui8_src[ 0 ][ 0 ], rgi_strides[ 0 ][ 0 ], rgpui8_dst[ 0 ] + ( i_field_idx * rgi_dst_strides[ 0 ] ), rgi_dst_strides[ 0 ] << 1 );
					ps_y262->s_funcs.rgf_motcomp_avg[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 1 ][ 0 ], rgi_strides[ 1 ][ 0 ], rgpui8_dst[ 1 ] + ( i_field_idx * rgi_dst_strides[ 1 ] ), rgi_dst_strides[ 1 ] << 1 );
					ps_y262->s_funcs.rgf_motcomp_avg[ i_chroma_blk_type ][ i_chroma_hpel_idx ]( rgpui8_src[ 2 ][ 0 ], rgi_strides[ 2 ][ 0 ], rgpui8_dst[ 2 ] + ( i_field_idx * rgi_dst_strides[ 2 ] ), rgi_dst_strides[ 2 ] << 1 );
				}
			}
		}
		ps_mb->i_cbp = 0;
		for( i_plane_idx = 0; i_plane_idx < 3; i_plane_idx++ )
		{
			i_num_blocks = i_plane_idx == 0 ? 4 : rgui_num_chroma_blk[ ps_y262->i_sequence_chroma_format ];
			for( i_blk_idx = 0; i_blk_idx < i_num_blocks; i_blk_idx++ )
			{
				uint8_t *pui8_blk, *pui8_pred;
				int32_t i_blk_stride, i_pred_stride;

				pui8_pred = y262_blk_pointer_adjust( ps_y262, rgpui8_dst[ i_plane_idx ], rgi_dst_strides[ i_plane_idx ], i_plane_idx, i_blk_idx, TRUE, &i_pred_stride );

				switch( i_plane_idx )
				{
					case 0:
						pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, i_plane_idx, i_blk_idx, TRUE, &i_blk_stride );
						break;
					case 1:
						pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_cb, ps_mb->i_src_chroma_stride, i_plane_idx, i_blk_idx, TRUE, &i_blk_stride );
						break;
					case 2:
						pui8_blk = y262_blk_pointer_adjust( ps_y262, ps_mb->pui8_src_cr, ps_mb->i_src_chroma_stride, i_plane_idx, i_blk_idx, TRUE, &i_blk_stride );
						break;
				}

				ps_y262->s_funcs.f_sub_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], pui8_blk, i_blk_stride, pui8_pred, i_pred_stride );

				ps_y262->s_funcs.f_fdct_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ] );
				if( ps_y262->i_quality_for_speed < 0 )
				{
					ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] = !!y262_quant8x8_inter_fw( ps_y262, ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ], 8, ps_y262->rgui16_non_intra_quantizer_matrices[ ps_mb->i_scaled_quantizer ] );
				}
				else
				{
					ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] = !!y262_quant8x8_trellis_fw( ps_y262, ps_slice, ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ], 8, ps_mb->i_scaled_quantizer, FALSE );
				}
				ps_mb->i_cbp |= ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] << ( i_plane_idx * 4 + i_blk_idx );

				if( ps_mb->rgb_cbp[ i_plane_idx ][ i_blk_idx ] )
				{
					for( i_y = 0; i_y < 8; i_y++ )
					{
						for( i_x = 0; i_x < 8; i_x++ )
						{
							ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ] = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ];
						}
					}

					y262_quant8x8_inter_bw( ps_y262, ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], 8, ps_mb->i_scaled_quantizer, ps_y262->rgui8_non_intra_quantiser_matrix );
					ps_y262->s_funcs.f_idct_8x8( ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ], ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ] );

					pui8_blk = y262_blk_pointer_adjust( ps_y262, rgpui8_dst[ i_plane_idx ], rgi_dst_strides[ i_plane_idx ], i_plane_idx, i_blk_idx, TRUE, &i_blk_stride );

					/*
					for( i_y = 0; i_y < 8; i_y++ )
					{
						for( i_x = 0; i_x < 8; i_x++ )
						{
							int32_t i_rec;
							i_rec = pui8_blk[ i_x + i_y * i_blk_stride ] + ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ][ i_x + i_y * 8 ];
							pui8_blk[ i_x + i_y * i_blk_stride ] = i_rec < 0 ? 0 : ( i_rec > 255 ? 255 : i_rec );
						}
					}
					*/
					ps_y262->s_funcs.f_add_8x8( pui8_blk, i_blk_stride, pui8_blk, i_blk_stride, ps_mb->rgi16_residual[ i_plane_idx ][ i_blk_idx ] );
				}
			}
		}
	}
	else
	{
		assert( FALSE );
	}
	return b_skip_cbf;
}


void y262_get_mbmode_motion( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mbmode )
{
	int32_t i_blk_type, i_fcode_x, i_fcode_y;
	y262_reference_picture_t *ps_ref;
	y262_me_context_t s_me;
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;

	if( i_mbmode == Y262_MBMODE_FW || i_mbmode == Y262_MBMODE_BW )
	{
		if( i_mbmode == Y262_MBMODE_FW )
		{
			ps_ref = ps_y262->ps_refpic_forward;
			i_blk_type = BLOCK_TYPE_16x16;
			i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 0 ];
			i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 1 ];
			s_me.i_pred_mv_x = ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ];
			s_me.i_pred_mv_y = ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ];
			s_me.i_num_candidates_fp = 1;
			s_me.rgi_candidates_fp[ 0 ][ 0 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 0 ][ 0 ];
			s_me.rgi_candidates_fp[ 0 ][ 1 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 0 ][ 1 ];
		}
		else if( i_mbmode == Y262_MBMODE_BW )
		{
			ps_ref = ps_y262->ps_refpic_backward;
			i_blk_type = BLOCK_TYPE_16x16;
			i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 0 ];
			i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 1 ];
			s_me.i_pred_mv_x = ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ];
			s_me.i_pred_mv_y = ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ];
			s_me.i_num_candidates_fp = 1;
			s_me.rgi_candidates_fp[ 0 ][ 0 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 1 ][ 0 ];
			s_me.rgi_candidates_fp[ 0 ][ 1 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 1 ][ 1 ];
		}
		else
		{
			assert( FALSE );
		}

		s_me.pui8_blk = ps_mb->pui8_src_luma;
		s_me.i_blk_stride = ps_mb->i_src_luma_stride;
		s_me.i_blk_type = i_blk_type;
		s_me.i_min_mv_x = -( 1 << ( 3 + i_fcode_x - 1 ) );
		s_me.i_min_mv_y = -( 1 << ( 3 + i_fcode_y - 1 ) );
		s_me.i_max_mv_x =  ( 1 << ( 3 + i_fcode_x - 1 ) ) - 1;
		s_me.i_max_mv_y =  ( 1 << ( 3 + i_fcode_y - 1 ) ) - 1;
		s_me.i_x_offset = ps_mb->i_mb_x;
		s_me.i_y_offset = ps_mb->i_mb_y;
		s_me.i_lambda = ps_mb->i_lambda_sqr;
		s_me.i_ref_width = ps_y262->i_sequence_width;
		s_me.i_ref_height = ps_y262->i_sequence_height;
		s_me.i_ref_stride = ps_ref->i_stride_luma;
		s_me.pui8_ref = ps_ref->pui8_luma;
		s_me.i_me_call = MECALL_MAIN;

		y262_motion_search( ps_y262, &s_me );

		if( i_mbmode == Y262_MBMODE_FW )
		{
			ps_slice->s_mode_decision.s_fw_mv.i_x = s_me.i_best_mv_x;
			ps_slice->s_mode_decision.s_fw_mv.i_y = s_me.i_best_mv_y;
			ps_slice->s_mode_decision.s_fw_mv.i_field = Y262_MV_FRAME_FIELD;
			ps_slice->s_mode_decision.s_fw_mv.i_cost = s_me.i_best_mv_sad;
			ps_slice->s_mode_decision.i_fw_cost = s_me.i_best_mv_sad;
		}
		else if( i_mbmode == Y262_MBMODE_BW )
		{
			ps_slice->s_mode_decision.s_bw_mv.i_x = s_me.i_best_mv_x;
			ps_slice->s_mode_decision.s_bw_mv.i_y = s_me.i_best_mv_y;
			ps_slice->s_mode_decision.s_bw_mv.i_field = Y262_MV_FRAME_FIELD;
			ps_slice->s_mode_decision.s_bw_mv.i_cost = s_me.i_best_mv_sad;
			ps_slice->s_mode_decision.i_bw_cost = s_me.i_best_mv_sad;
		}
		else
		{
			assert( FALSE );
		}
	}
	else if( i_mbmode == Y262_MBMODE_FW_IL || i_mbmode == Y262_MBMODE_BW_IL )
	{
		int32_t i_field_idx, i_search_field_idx, i_cost = 0;
		int32_t i_top_field_sad;

		for( i_field_idx = 0; i_field_idx < 2; i_field_idx++ )
		{
			if( i_mbmode == Y262_MBMODE_FW_IL )
			{
				ps_ref = ps_y262->ps_refpic_forward;
				i_blk_type = BLOCK_TYPE_16x8;
				i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 0 ];
				i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 1 ] - 1;
				s_me.i_pred_mv_x = ps_slice->rgi_pmv[ i_field_idx ][ 0 ][ 0 ];
				s_me.i_pred_mv_y = ps_slice->rgi_pmv[ i_field_idx ][ 0 ][ 1 ];
				s_me.i_num_candidates_fp = 1;
				s_me.rgi_candidates_fp[ 0 ][ 0 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 0 ][ 0 ];
				s_me.rgi_candidates_fp[ 0 ][ 1 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 0 ][ 1 ] >> 1;
			}
			else if( i_mbmode == Y262_MBMODE_BW_IL )
			{
				ps_ref = ps_y262->ps_refpic_backward;
				i_blk_type = BLOCK_TYPE_16x8;
				i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 0 ];
				i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 1 ] - 1;
				s_me.i_pred_mv_x = ps_slice->rgi_pmv[ i_field_idx ][ 1 ][ 0 ];
				s_me.i_pred_mv_y = ps_slice->rgi_pmv[ i_field_idx ][ 1 ][ 1 ];
				s_me.i_num_candidates_fp = 1;
				s_me.rgi_candidates_fp[ 0 ][ 0 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 1 ][ 0 ];
				s_me.rgi_candidates_fp[ 0 ][ 1 ] = ps_y262->ps_input_picture->ps_lookahead[ ps_mb->i_mb_addr ].rgi_mvs[ 1 ][ 1 ] >> 1;
			}
			else
			{
				assert( FALSE );
			}

			s_me.pui8_blk = ps_mb->pui8_src_luma + ( ps_mb->i_src_luma_stride * i_field_idx );
			s_me.i_blk_stride = ps_mb->i_src_luma_stride << 1;
			s_me.i_blk_type = i_blk_type;
			s_me.i_min_mv_x = -( 1 << ( 3 + i_fcode_x - 1 ) );
			s_me.i_min_mv_y = -( 1 << ( 3 + i_fcode_y - 1 ) );
			s_me.i_max_mv_x =  ( 1 << ( 3 + i_fcode_x - 1 ) ) - 1;
			s_me.i_max_mv_y =  ( 1 << ( 3 + i_fcode_y - 1 ) ) - 1;
			s_me.i_x_offset = ps_mb->i_mb_x;
			s_me.i_y_offset = ps_mb->i_mb_y >> 1;
			s_me.i_lambda = ps_mb->i_lambda_sqr;
			s_me.i_me_call = MECALL_MAIN;

			for( i_search_field_idx = 0; i_search_field_idx < 2; i_search_field_idx++ )
			{

				s_me.i_ref_width = ps_y262->i_sequence_width;
				s_me.i_ref_height = ps_y262->i_sequence_height >> 1;
				s_me.i_ref_stride = ps_ref->i_stride_luma << 1;
				s_me.pui8_ref = ps_ref->pui8_luma + ( i_search_field_idx * ps_ref->i_stride_luma );

				y262_motion_search( ps_y262, &s_me );

				if( i_mbmode == Y262_MBMODE_FW_IL && ( i_search_field_idx == 0 || s_me.i_best_mv_sad < i_top_field_sad ) )
				{
					ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ].i_x = s_me.i_best_mv_x;
					ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ].i_y = s_me.i_best_mv_y;
					ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ].i_field = i_search_field_idx == 0 ? Y262_MV_TOP_FIELD : Y262_MV_BOTTOM_FIELD;
					i_top_field_sad = s_me.i_best_mv_sad;
				}
				else if( i_mbmode == Y262_MBMODE_BW_IL && ( i_search_field_idx == 0 || s_me.i_best_mv_sad < i_top_field_sad ) )
				{
					ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ].i_x = s_me.i_best_mv_x;
					ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ].i_y = s_me.i_best_mv_y;
					ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ].i_field = i_search_field_idx == 0 ? Y262_MV_TOP_FIELD : Y262_MV_BOTTOM_FIELD;
					i_top_field_sad = s_me.i_best_mv_sad;
				}
			}
			i_cost += i_top_field_sad;
		}
		if( i_mbmode == Y262_MBMODE_FW_IL )
		{
			ps_slice->s_mode_decision.s_fw_il_mv[ 0 ].i_cost = i_cost;
			ps_slice->s_mode_decision.s_fw_il_mv[ 1 ].i_cost = i_cost;
			ps_slice->s_mode_decision.i_fw_il_cost = i_cost;
		}
		else
		{
			ps_slice->s_mode_decision.s_bw_il_mv[ 0 ].i_cost = i_cost;
			ps_slice->s_mode_decision.s_bw_il_mv[ 1 ].i_cost = i_cost;
			ps_slice->s_mode_decision.i_bw_il_cost = i_cost;
		}
	}
}

void y262_get_mbmode_satd_cost( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mbmode )
{
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;

	if( i_mbmode == Y262_MBMODE_SKIP )
	{
		int32_t i_idx, i_num_preds, i_blk_type;
		int32_t rgi_strides[ 2 ];
		uint8_t *rgpui8_src[ 2 ];
		y262_reference_picture_t *rgps_refs[ 2 ];
		y262_mv_t *rgps_mvs[ 2 ], s_skip_mvs[ 2 ];
		ALIGNED( 16 ) uint8_t rgui8_pred[ 16 * 16 ];

		i_blk_type = MC_BLOCK_16x16;

		ps_mb = &ps_slice->s_macroblock;

		if( ps_slice->i_picture_type == PICTURE_CODING_TYPE_P )
		{
			rgps_refs[ 0 ] = ps_y262->ps_refpic_forward;
			s_skip_mvs[ 0 ].i_x = 0;
			s_skip_mvs[ 0 ].i_y = 0;
			rgps_mvs[ 0 ] = &s_skip_mvs[ 0 ];
			i_num_preds = 1;
		}
		else
		{
			assert( ps_slice->i_picture_type == PICTURE_CODING_TYPE_B );
			i_num_preds = 0;
			if( ps_slice->i_last_mb_motion_flags & MACROBLOCK_MOTION_FORWARD )
			{
				rgps_refs[ i_num_preds ] = ps_y262->ps_refpic_forward;
				s_skip_mvs[ i_num_preds ].i_x = ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ];
				s_skip_mvs[ i_num_preds ].i_y = ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ];
				rgps_mvs[ i_num_preds ] = &s_skip_mvs[ i_num_preds ];
				i_num_preds++;
			}
			if( ps_slice->i_last_mb_motion_flags & MACROBLOCK_MOTION_BACKWARD )
			{
				rgps_refs[ i_num_preds ] = ps_y262->ps_refpic_backward;
				s_skip_mvs[ i_num_preds ].i_x = ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ];
				s_skip_mvs[ i_num_preds ].i_y = ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ];
				rgps_mvs[ i_num_preds ] = &s_skip_mvs[ i_num_preds ];
				i_num_preds++;
			}
		}

		for( i_idx = 0; i_idx < i_num_preds; i_idx++ )
		{
			int32_t i_hpel_idx;

			rgi_strides[ 0 ] = rgps_refs[ i_idx ]->i_stride_luma;
			rgpui8_src[ 0 ] = rgps_refs[ i_idx ]->pui8_luma + ps_mb->i_mb_x + ( ps_mb->i_mb_y * rgi_strides[ 0 ] );

			i_hpel_idx = ( rgps_mvs[ i_idx ]->i_x & 1 ) | ( ( rgps_mvs[ i_idx ]->i_y & 1 ) << 1 );
			rgpui8_src[ 0 ] += ( rgps_mvs[ i_idx ]->i_x >> 1 ) + ( ( rgps_mvs[ i_idx ]->i_y >> 1 ) * rgi_strides[ 0 ] );

			if( i_idx == 0 )
			{
				ps_y262->s_funcs.rgf_motcomp_copy[ i_blk_type ][ i_hpel_idx ]( rgpui8_src[ 0 ], rgi_strides[ 0 ], rgui8_pred, 16 );
			}
			else
			{
				ps_y262->s_funcs.rgf_motcomp_avg[ i_blk_type ][ i_hpel_idx ]( rgpui8_src[ 0 ], rgi_strides[ 0 ], rgui8_pred, 16 );
			}
		}
		ps_slice->s_mode_decision.i_skip_cost = ps_y262->s_funcs.rgf_satd[ i_blk_type ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, rgui8_pred, 16 );
		
	}
	else if( i_mbmode == Y262_MBMODE_INTRA )
	{
		ALIGNED( 16 ) static const uint8_t rgui8_zero[ 16 ] = { 0 };
		int32_t i_sad;
		ps_slice->s_mode_decision.i_intra_cost = ps_y262->s_funcs.rgf_satd[ MC_BLOCK_16x16 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, ( uint8_t *)rgui8_zero, 0 );
		i_sad = ps_y262->s_funcs.rgf_sad[ MC_BLOCK_16x16 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, ( uint8_t *)rgui8_zero, 0 );
		ps_slice->s_mode_decision.i_intra_cost -= i_sad >> 2;
		ps_slice->s_mode_decision.i_intra_cost += ps_mb->i_lambda_sqr * 13;
	}
	else if( i_mbmode == Y262_MBMODE_INTRA_IL )
	{
		ALIGNED( 16 ) static const uint8_t rgui8_zero[ 16 ] = { 0 };
		int32_t i_satd, i_sad;

		i_satd = ps_y262->s_funcs.rgf_satd[ MC_BLOCK_16x8 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
		i_sad = ps_y262->s_funcs.rgf_sad[ MC_BLOCK_16x8 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
		ps_slice->s_mode_decision.i_intra_il_cost = i_satd - ( i_sad >> 2 );

		i_satd = ps_y262->s_funcs.rgf_satd[ MC_BLOCK_16x8 ]( ps_mb->pui8_src_luma + ps_mb->i_src_luma_stride, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
		i_sad = ps_y262->s_funcs.rgf_sad[ MC_BLOCK_16x8 ]( ps_mb->pui8_src_luma + ps_mb->i_src_luma_stride, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
		ps_slice->s_mode_decision.i_intra_il_cost += i_satd - ( i_sad >> 2 );
		ps_slice->s_mode_decision.i_intra_il_cost += ps_mb->i_lambda_sqr * 13;
	}
}


void y262_get_mbmode_motion_bi( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mbmode )
{
	uint8_t *pui8_ref, *pui8_deltasrc;
	int32_t i_blk_type, i_iter, i_fcode_x, i_fcode_y, i_mv_x, i_mv_y, i_hpelidx, i_deltasrc_stride, i_max_iter, i_cost = 0;
	y262_reference_picture_t *ps_ref, *ps_otherref;
	y262_me_context_t s_me;
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;

	i_max_iter = MAX( 0, ps_y262->i_quality_for_speed / 5 );

	if( i_mbmode == Y262_MBMODE_BI )
	{
		ALIGNED( 16 ) uint8_t rgui8_delta[ 16 * 16 ];
		ps_slice->s_mode_decision.s_bi_mv[ 0 ].i_x = ps_slice->s_mode_decision.s_fw_mv.i_x;
		ps_slice->s_mode_decision.s_bi_mv[ 0 ].i_y = ps_slice->s_mode_decision.s_fw_mv.i_y;
		ps_slice->s_mode_decision.s_bi_mv[ 0 ].i_field = Y262_MV_FRAME_FIELD;
		ps_slice->s_mode_decision.s_bi_mv[ 1 ].i_x = ps_slice->s_mode_decision.s_bw_mv.i_x;
		ps_slice->s_mode_decision.s_bi_mv[ 1 ].i_y = ps_slice->s_mode_decision.s_bw_mv.i_y;
		ps_slice->s_mode_decision.s_bi_mv[ 1 ].i_field = Y262_MV_FRAME_FIELD;

		for( i_iter = 0; i_iter < i_max_iter; i_iter++ )
		{
			int32_t i_dir, i_best_mv_x, i_best_mv_y;

			i_dir = i_iter & 1;
			i_blk_type = BLOCK_TYPE_16x16;
			s_me.pui8_blk = rgui8_delta;
			s_me.i_blk_stride = 16;
			s_me.i_blk_type = i_blk_type;
			s_me.i_x_offset = ps_mb->i_mb_x;
			s_me.i_y_offset = ps_mb->i_mb_y;
			s_me.i_num_candidates_fp = 0;
			s_me.i_lambda = ps_mb->i_lambda_sqr;

			if( i_dir == 0 )
			{
				ps_ref = ps_y262->ps_refpic_forward;
				ps_otherref = ps_y262->ps_refpic_backward;
				i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 0 ];
				i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 1 ];
				s_me.i_pred_mv_x = ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ];
				s_me.i_pred_mv_y = ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ];
			}
			else
			{
				ps_ref = ps_y262->ps_refpic_backward;
				ps_otherref = ps_y262->ps_refpic_forward;
				i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 0 ];
				i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 1 ];
				s_me.i_pred_mv_x = ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ];
				s_me.i_pred_mv_y = ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ];
			}

			s_me.i_min_mv_x = -( 1 << ( 3 + i_fcode_x - 1 ) );
			s_me.i_min_mv_y = -( 1 << ( 3 + i_fcode_y - 1 ) );
			s_me.i_max_mv_x =  ( 1 << ( 3 + i_fcode_x - 1 ) ) - 1;
			s_me.i_max_mv_y =  ( 1 << ( 3 + i_fcode_y - 1 ) ) - 1;
			s_me.i_ref_width = ps_y262->i_sequence_width;
			s_me.i_ref_height = ps_y262->i_sequence_height;
			s_me.i_ref_stride = ps_ref->i_stride_luma;
			s_me.pui8_ref = ps_ref->pui8_luma;
			i_best_mv_x = s_me.i_best_mv_x = ps_slice->s_mode_decision.s_bi_mv[ i_dir ].i_x;
			i_best_mv_y = s_me.i_best_mv_y = ps_slice->s_mode_decision.s_bi_mv[ i_dir ].i_y;

			i_mv_x = ps_slice->s_mode_decision.s_bi_mv[ !i_dir ].i_x;
			i_mv_y = ps_slice->s_mode_decision.s_bi_mv[ !i_dir ].i_y;
			pui8_ref = ps_otherref->pui8_luma + ( s_me.i_x_offset + ( i_mv_x >> 1 ) ) + ( ( s_me.i_y_offset + ( i_mv_y >> 1 ) ) * ps_otherref->i_stride_luma );
			i_hpelidx = ( ( i_mv_x ) & 1 ) | ( ( ( i_mv_y ) & 1 ) << 1 );
			if( i_hpelidx )
			{
				ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ i_hpelidx ]( pui8_ref, ps_otherref->i_stride_luma, rgui8_delta, 16 );
				pui8_deltasrc = rgui8_delta;
				i_deltasrc_stride = 16;
			}
			else
			{
				pui8_deltasrc = pui8_ref;
				i_deltasrc_stride = ps_otherref->i_stride_luma;
			}
			for( i_mv_y = 0; i_mv_y < 16; i_mv_y++ )
			{
				for( i_mv_x = 0; i_mv_x < 16; i_mv_x++ )
				{
					rgui8_delta[ i_mv_x + i_mv_y * 16 ] = MIN( 255, MAX( 0, ( ps_mb->pui8_src_luma[ i_mv_x + i_mv_y * ps_mb->i_src_luma_stride ] << 1 ) - pui8_deltasrc[ i_mv_x + i_mv_y * i_deltasrc_stride ] ) );
				}
			}

			y262_hpel_motion_search( ps_y262, &s_me );

			ps_slice->s_mode_decision.s_bi_mv[ i_dir ].i_x = s_me.i_best_mv_x;
			ps_slice->s_mode_decision.s_bi_mv[ i_dir ].i_y = s_me.i_best_mv_y;
			ps_slice->s_mode_decision.s_bi_mv[ i_dir ].i_field = Y262_MV_FRAME_FIELD;
			if( s_me.i_best_mv_x == i_best_mv_x && s_me.i_best_mv_y == i_best_mv_y )
			{
				break;
			}
		}
		if( 1 )
		{
			int32_t i_dir, i_bits = 0;
			for( i_dir = 0; i_dir < 2; i_dir++ )
			{
				if( i_dir == 0 )
				{
					ps_ref = ps_y262->ps_refpic_forward;
				}
				else
				{
					ps_ref = ps_y262->ps_refpic_backward;
				}
				i_mv_x = ps_slice->s_mode_decision.s_bi_mv[ i_dir ].i_x;
				i_mv_y = ps_slice->s_mode_decision.s_bi_mv[ i_dir ].i_y;
				pui8_ref = ps_ref->pui8_luma + ( ps_mb->i_mb_x + ( i_mv_x >> 1 ) ) + ( ( ps_mb->i_mb_y + ( i_mv_y >> 1 ) ) * ps_ref->i_stride_luma );
				i_hpelidx = ( ( i_mv_x ) & 1 ) | ( ( ( i_mv_y ) & 1 ) << 1 );
				if( i_dir == 0 )
				{
					ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x16 ][ i_hpelidx ]( pui8_ref, ps_ref->i_stride_luma, rgui8_delta, 16 );
				}
				else
				{
					ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x16 ][ i_hpelidx ]( pui8_ref, ps_ref->i_stride_luma, rgui8_delta, 16 );
				}
				i_bits += ps_y262->rgi_y262_motion_bits_x[ 2048 + i_mv_x - ps_slice->rgi_pmv[ 0 ][ i_dir ][ 0 ] ];
				i_bits += ps_y262->rgi_y262_motion_bits_y[ 2048 + i_mv_y - ps_slice->rgi_pmv[ 0 ][ i_dir ][ 1 ] ];
			}
			i_cost = ps_y262->s_funcs.rgf_satd[ MC_BLOCK_16x16 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, rgui8_delta, 16 );
			i_cost += i_bits * ps_mb->i_lambda_sqr;
		}
		ps_slice->s_mode_decision.i_bi_cost = i_cost;
	}
	else if( i_mbmode == Y262_MBMODE_BI_IL  )
	{
		int32_t i_field_idx, i_search_field_idx;

		for( i_field_idx = 0; i_field_idx < 2; i_field_idx++ )
		{
			int32_t i_dir, i_best_mv_x, i_best_mv_y;

			ALIGNED( 16 ) uint8_t rgui8_delta[ 16 * 8 ];
			ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 0 ].i_x = ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ].i_x;
			ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 0 ].i_y = ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ].i_y;
			ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 0 ].i_field = ps_slice->s_mode_decision.s_fw_il_mv[ i_field_idx ].i_field;
			ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 1 ].i_x = ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ].i_x;
			ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 1 ].i_y = ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ].i_y;
			ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ 1 ].i_field = ps_slice->s_mode_decision.s_bw_il_mv[ i_field_idx ].i_field;

			for( i_iter = 0; i_iter < i_max_iter; i_iter++ )
			{
				i_dir = i_iter & 1;

				if( i_dir == 0 )
				{
					ps_ref = ps_y262->ps_refpic_forward;
					ps_otherref = ps_y262->ps_refpic_backward;
					i_blk_type = BLOCK_TYPE_16x8;
					i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 0 ];
					i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 1 ] - 1;
					s_me.i_pred_mv_x = ps_slice->rgi_pmv[ i_field_idx ][ 0 ][ 0 ];
					s_me.i_pred_mv_y = ps_slice->rgi_pmv[ i_field_idx ][ 0 ][ 1 ];
				}
				else if( i_dir == 1 )
				{
					ps_ref = ps_y262->ps_refpic_backward;
					ps_otherref = ps_y262->ps_refpic_forward;
					i_blk_type = BLOCK_TYPE_16x8;
					i_fcode_x = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 0 ];
					i_fcode_y = ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 1 ] - 1;
					s_me.i_pred_mv_x = ps_slice->rgi_pmv[ i_field_idx ][ 1 ][ 0 ];
					s_me.i_pred_mv_y = ps_slice->rgi_pmv[ i_field_idx ][ 1 ][ 1 ];
				}
				else
				{
					assert( FALSE );
				}

				s_me.pui8_blk = rgui8_delta;
				s_me.i_blk_stride = 16;
				s_me.i_blk_type = i_blk_type;
				s_me.i_min_mv_x = -( 1 << ( 3 + i_fcode_x - 1 ) );
				s_me.i_min_mv_y = -( 1 << ( 3 + i_fcode_y - 1 ) );
				s_me.i_max_mv_x =  ( 1 << ( 3 + i_fcode_x - 1 ) ) - 1;
				s_me.i_max_mv_y =  ( 1 << ( 3 + i_fcode_y - 1 ) ) - 1;
				s_me.i_x_offset = ps_mb->i_mb_x;
				s_me.i_y_offset = ps_mb->i_mb_y >> 1;
				s_me.i_num_candidates_fp = 0;
				s_me.i_lambda = ps_mb->i_lambda_sqr;

				i_mv_x = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ !i_dir ].i_x;
				i_mv_y = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ !i_dir ].i_y;
				pui8_ref = ps_otherref->pui8_luma + ( s_me.i_x_offset + ( i_mv_x >> 1 ) ) + ( ( s_me.i_y_offset + ( i_mv_y >> 1 ) ) * ps_otherref->i_stride_luma * 2 );
				if( ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ !i_dir ].i_field == Y262_MV_BOTTOM_FIELD )
				{
					pui8_ref += ps_otherref->i_stride_luma;
				}
				i_hpelidx = ( ( i_mv_x ) & 1 ) | ( ( ( i_mv_y ) & 1 ) << 1 );
				if( i_hpelidx )
				{
					ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ i_hpelidx ]( pui8_ref, ps_otherref->i_stride_luma * 2, rgui8_delta, 16 );
					pui8_deltasrc = rgui8_delta;
					i_deltasrc_stride = 16;
				}
				else
				{
					pui8_deltasrc = pui8_ref;
					i_deltasrc_stride = ps_otherref->i_stride_luma * 2;
				}
				for( i_mv_y = 0; i_mv_y < 8; i_mv_y++ )
				{
					for( i_mv_x = 0; i_mv_x < 16; i_mv_x++ )
					{
						rgui8_delta[ i_mv_x + i_mv_y * 16 ] = MIN( 255, MAX( 0, ( ps_mb->pui8_src_luma[ i_mv_x + i_mv_y * ps_mb->i_src_luma_stride * 2 + ( ps_mb->i_src_luma_stride * i_field_idx ) ] << 1 ) - pui8_deltasrc[ i_mv_x + i_mv_y * i_deltasrc_stride ] ) );
					}
				}

				i_best_mv_x = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_x;
				i_best_mv_y = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_y;
				i_search_field_idx = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_field == Y262_MV_TOP_FIELD ? 0 : 1;

				s_me.i_ref_width = ps_y262->i_sequence_width;
				s_me.i_ref_height = ps_y262->i_sequence_height >> 1;
				s_me.i_ref_stride = ps_ref->i_stride_luma << 1;
				s_me.pui8_ref = ps_ref->pui8_luma + ( i_search_field_idx * ps_ref->i_stride_luma );
				s_me.i_best_mv_x = i_best_mv_x;
				s_me.i_best_mv_y = i_best_mv_y;

				y262_hpel_motion_search( ps_y262, &s_me );

				ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_x = s_me.i_best_mv_x;
				ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_y = s_me.i_best_mv_y;
				ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_field = i_search_field_idx == 0 ? Y262_MV_TOP_FIELD : Y262_MV_BOTTOM_FIELD;

				if( s_me.i_best_mv_x == i_best_mv_x && s_me.i_best_mv_y == i_best_mv_y )
				{
					break;
				}
			}
			if( 1 )
			{
				int32_t i_bits = 0;
				for( i_dir = 0; i_dir < 2; i_dir++ )
				{
					if( i_dir == 0 )
					{
						ps_ref = ps_y262->ps_refpic_forward;
					}
					else
					{
						ps_ref = ps_y262->ps_refpic_backward;
					}
					i_mv_x = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_x;
					i_mv_y = ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_y;
					pui8_ref = ps_ref->pui8_luma + ( ps_mb->i_mb_x + ( i_mv_x >> 1 ) ) + ( ( ( ps_mb->i_mb_y >> 1 ) + ( i_mv_y >> 1 ) ) * ps_ref->i_stride_luma * 2 );
					if( ps_slice->s_mode_decision.s_bi_il_mv[ i_field_idx ][ i_dir ].i_field == Y262_MV_BOTTOM_FIELD )
					{
						pui8_ref += ps_ref->i_stride_luma;
					}
					i_hpelidx = ( ( i_mv_x ) & 1 ) | ( ( ( i_mv_y ) & 1 ) << 1 );
					if( i_dir == 0 )
					{
						ps_y262->s_funcs.rgf_motcomp_copy[ MC_BLOCK_16x8 ][ i_hpelidx ]( pui8_ref, ps_ref->i_stride_luma * 2, rgui8_delta, 16 );
					}
					else
					{
						ps_y262->s_funcs.rgf_motcomp_avg[ MC_BLOCK_16x8 ][ i_hpelidx ]( pui8_ref, ps_ref->i_stride_luma * 2, rgui8_delta, 16 );
					}
					i_bits += ps_y262->rgi_y262_motion_bits_x[ 2048 + i_mv_x - ps_slice->rgi_pmv[ i_field_idx ][ i_dir ][ 0 ] ];
					i_bits += ps_y262->rgi_y262_motion_bits_y[ 2048 + i_mv_y - ps_slice->rgi_pmv[ i_field_idx ][ i_dir ][ 1 ] ];
				}
				i_cost += ps_y262->s_funcs.rgf_satd[ MC_BLOCK_16x8 ]( ps_mb->pui8_src_luma + ( ps_mb->i_src_luma_stride * i_field_idx ), ps_mb->i_src_luma_stride * 2, rgui8_delta, 16 );
				i_cost += i_bits * ps_mb->i_lambda_sqr;
			}
		}
		ps_slice->s_mode_decision.i_bi_il_cost = i_cost;
	}
	else
	{
		assert( FALSE );
	}
}

static int32_t rgi_quantizer_delta[ 64 ] = { 0 };
void y262_qprd_mbmode( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mbmode, int32_t i_mbmode_cost )
{
	int32_t i_original_quantizer, i_quantizer, i_best_quantizer, i_best_cost, i_cost, i_dir, i_down, i_up;
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;

	i_original_quantizer = i_best_quantizer = ps_mb->i_quantizer;
	i_best_cost = i_mbmode_cost;

	if( i_original_quantizer > 1 )
	{
		ps_mb->i_quantizer = i_original_quantizer - 1;
		ps_mb->i_scaled_quantizer = rgi8_y262_quantiser_scale_table[ ps_y262->b_qscale_type ][ ps_mb->i_quantizer ];
		i_down = y262_get_mbmode_cost( ps_y262, ps_slice, i_mbmode );
		if( i_down < i_best_cost )
		{
			i_best_cost = i_down;
			i_best_quantizer = ps_mb->i_quantizer;
		}
	}
	else
	{
		i_down = MAX_COST;
	}

	if( i_original_quantizer < 31 )
	{
		ps_mb->i_quantizer = i_original_quantizer + 1;
		ps_mb->i_scaled_quantizer = rgi8_y262_quantiser_scale_table[ ps_y262->b_qscale_type ][ ps_mb->i_quantizer ];
		i_up = y262_get_mbmode_cost( ps_y262, ps_slice, i_mbmode );
		if( i_up < i_best_cost )
		{
			i_best_cost = i_up;
			i_best_quantizer = ps_mb->i_quantizer;
		}
	}
	else
	{
		i_up = MAX_COST;
	}

	if( i_best_quantizer != i_original_quantizer )
	{
		if( i_down < i_up )
		{
			i_dir = -1;
		}
		else
		{
			i_dir = 1;
		}
		while( 1 )
		{
			i_quantizer = i_best_quantizer + i_dir;
			if( i_quantizer > 0 && i_quantizer < 32 )
			{
				ps_mb->i_quantizer = i_quantizer;
				ps_mb->i_scaled_quantizer = rgi8_y262_quantiser_scale_table[ ps_y262->b_qscale_type ][ ps_mb->i_quantizer ];
				i_cost = y262_get_mbmode_cost( ps_y262, ps_slice, i_mbmode );
				if( i_cost < i_best_cost )
				{
					i_best_cost = i_cost;
					i_best_quantizer = i_quantizer;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	ps_mb->i_quantizer = i_best_quantizer;
	ps_mb->i_scaled_quantizer = rgi8_y262_quantiser_scale_table[ ps_y262->b_qscale_type ][ ps_mb->i_quantizer ];
	rgi_quantizer_delta[ i_best_quantizer - i_original_quantizer + 32 ]++;
}

bool_t y262_bskip_valid( y262_t *ps_y262, y262_slice_t *ps_slice )
{
	bool_t b_allow_skip;
	y262_macroblock_t *ps_mb;
	ps_mb = &ps_slice->s_macroblock;

	b_allow_skip = TRUE;
	if( ps_slice->i_last_mb_motion_flags & MACROBLOCK_MOTION_FORWARD )
	{
		if( ( ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ] + ( ps_mb->i_mb_x << 1 ) ) < 0 ||
			( ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ] + ( ( 16 + ps_mb->i_mb_x ) << 1 ) ) > ( ps_y262->i_sequence_width << 1 ) )
		{
			b_allow_skip = FALSE;
		}
		if( ( ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ] + ( ps_mb->i_mb_y << 1 ) ) < 0 ||
			( ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ] + ( ( 16 + ps_mb->i_mb_y ) << 1 ) ) > ( ps_y262->i_sequence_height << 1 ) )
		{
			b_allow_skip = FALSE;
		}
	}
	if( ps_slice->i_last_mb_motion_flags & MACROBLOCK_MOTION_BACKWARD )
	{
		if( ( ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ] + ( ps_mb->i_mb_x << 1 ) ) < 0 ||
			( ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ] + ( ( 16 + ps_mb->i_mb_x ) << 1 ) ) > ( ps_y262->i_sequence_width << 1 ) )
		{
			b_allow_skip = FALSE;
		}
		if( ( ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ] + ( ps_mb->i_mb_y << 1 ) ) < 0 ||
			( ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ] + ( ( 16 + ps_mb->i_mb_y ) << 1 ) ) > ( ps_y262->i_sequence_height << 1 ) )
		{
			b_allow_skip = FALSE;
		}
	}
	return b_allow_skip;
}


void y262_encode_macroblock( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mb_idx, bool_t *pb_skip )
{
	int32_t i_best_mbmode, i_best_cost, i_best_satd_cost;
	bool_t b_no_frame, b_no_field;
	y262_macroblock_t *ps_mb;

	*pb_skip = FALSE;
	ps_mb = &ps_slice->s_macroblock;

	b_no_frame = b_no_field = FALSE;
	if( !ps_y262->b_frame_pred_frame_dct && ps_y262->i_quality_for_speed < -20 )
	{
		b_no_frame = y262_16x16_frame_field_pel_decision( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride );
		b_no_field = !b_no_frame;
	}

	if( ps_slice->i_picture_type == PICTURE_CODING_TYPE_I )
	{
		if( ps_y262->b_frame_pred_frame_dct == FALSE )
		{
			i_best_satd_cost = MAX_COST;
			i_best_mbmode = Y262_MBMODE_INTRA;

			if( !b_no_frame )
			{
				y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA );
				if( ps_slice->s_mode_decision.i_intra_cost < i_best_satd_cost )
				{
					i_best_satd_cost = ps_slice->s_mode_decision.i_intra_cost;
					i_best_mbmode = Y262_MBMODE_INTRA;
				}
			}
			else
			{
				ps_slice->s_mode_decision.i_intra_cost = MAX_COST;
			}

			if( !b_no_field )
			{
				y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA_IL );
				if( ps_slice->s_mode_decision.i_intra_il_cost < i_best_satd_cost )
				{
					i_best_satd_cost = ps_slice->s_mode_decision.i_intra_il_cost;
					i_best_mbmode = Y262_MBMODE_INTRA_IL;
				}
			}
			else
			{
				ps_slice->s_mode_decision.i_intra_il_cost = MAX_COST;
			}

			if( ps_y262->i_quality_for_speed >= -25 )
			{
				i_best_cost = MAX_COST;
				if( ps_slice->s_mode_decision.i_intra_cost <= ( i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( 40 + ps_y262->i_quality_for_speed ) ) ) )
				{
					y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_intra_cost < i_best_cost )
					{
						i_best_cost = ps_slice->s_mode_decision.i_intra_cost;
						i_best_mbmode = Y262_MBMODE_INTRA;
					}
				}
				if( ps_slice->s_mode_decision.i_intra_il_cost <= ( i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( 40 + ps_y262->i_quality_for_speed ) ) ) )
				{
					y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA_IL );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_intra_il_cost < i_best_cost )
					{
						i_best_cost = ps_slice->s_mode_decision.i_intra_il_cost;
						i_best_mbmode = Y262_MBMODE_INTRA_IL;
					}
				}
			}

			if( ps_y262->i_quality_for_speed > 10 )
			{
				y262_qprd_mbmode( ps_y262, ps_slice, i_best_mbmode, i_best_cost );
			}

			if( i_best_mbmode == Y262_MBMODE_INTRA )
			{
				y262_encode_macroblock_intra( ps_y262, ps_slice, FALSE );
			}
			else
			{
				y262_encode_macroblock_intra( ps_y262, ps_slice, TRUE );
			}
		}
		else
		{
			/* qprd ? */
			y262_encode_macroblock_intra( ps_y262, ps_slice, FALSE );
		}
	}
	else if( ps_slice->i_picture_type == PICTURE_CODING_TYPE_P )
	{
		if( !b_no_frame )
		{
			y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_FW );
			i_best_satd_cost = ps_slice->s_mode_decision.i_fw_cost;
			i_best_mbmode = Y262_MBMODE_FW;
		}
		else
		{
			ps_slice->s_mode_decision.i_fw_cost = MAX_COST;

			y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_FW_IL );
			i_best_satd_cost = ps_slice->s_mode_decision.i_fw_il_cost;
			i_best_mbmode = Y262_MBMODE_FW_IL;
		}

		if( ps_slice->b_allow_skip )
		{
			y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_SKIP );
			if( i_best_satd_cost > ps_slice->s_mode_decision.i_skip_cost )
			{
				if( !y262_encode_macroblock_inter( ps_y262, ps_slice, Y262_MBMODE_SKIP ) )
				{
					//i_best_satd_cost = ps_slice->s_mode_decision.i_skip_cost;
					i_best_mbmode = Y262_MBMODE_SKIP;
					if( ps_y262->i_quality_for_speed < -5 )
					{
						goto fast_skip_p;
					}
				}
			}
		}

		if( !b_no_frame )
		{
			y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA );
			if( ps_slice->s_mode_decision.i_intra_cost < i_best_satd_cost )
			{
				i_best_satd_cost = ps_slice->s_mode_decision.i_intra_cost;
				i_best_mbmode = Y262_MBMODE_INTRA;
			}
		}
		else
		{
			ps_slice->s_mode_decision.i_intra_cost = MAX_COST;
		}

		if( ps_y262->b_frame_pred_frame_dct == FALSE && !b_no_field )
		{
			if( !b_no_frame )
			{
				y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_FW_IL );
				if( ps_slice->s_mode_decision.i_fw_il_cost < i_best_satd_cost )
				{
					i_best_satd_cost = ps_slice->s_mode_decision.i_fw_il_cost;
					i_best_mbmode = Y262_MBMODE_FW_IL;
				}
			}
			y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA_IL );
			if( ps_slice->s_mode_decision.i_intra_il_cost < i_best_satd_cost )
			{
				i_best_satd_cost = ps_slice->s_mode_decision.i_intra_il_cost;
				i_best_mbmode = Y262_MBMODE_INTRA_IL;
			}
		}
		else
		{
			ps_slice->s_mode_decision.i_fw_il_cost = MAX_COST;
			ps_slice->s_mode_decision.i_intra_il_cost = MAX_COST;
		}

		if( ps_y262->i_quality_for_speed >= -30 )
		{
			i_best_cost = MAX_COST;
			if( ps_slice->s_mode_decision.i_fw_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
			{
				ps_slice->s_mode_decision.i_fw_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_FW );
				if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_fw_cost < i_best_cost )
				{
					i_best_mbmode = Y262_MBMODE_FW;
					i_best_cost = ps_slice->s_mode_decision.i_fw_cost;
				}
			}

			if( ps_slice->s_mode_decision.i_intra_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 50 + ps_y262->i_quality_for_speed ) ) ) ) )
			{
				ps_slice->s_mode_decision.i_intra_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA );
				if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_intra_cost < i_best_cost )
				{
					i_best_mbmode = Y262_MBMODE_INTRA;
					i_best_cost = ps_slice->s_mode_decision.i_intra_cost;
				}
			}

			if( ps_y262->b_frame_pred_frame_dct == FALSE )
			{
				if( ps_slice->s_mode_decision.i_fw_il_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
				{
					ps_slice->s_mode_decision.i_fw_il_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_FW_IL );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_fw_il_cost < i_best_cost )
					{
						i_best_mbmode = Y262_MBMODE_FW_IL;
						i_best_cost = ps_slice->s_mode_decision.i_fw_il_cost;
					}
				}

				if( ps_slice->s_mode_decision.i_intra_il_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) )
				{
					ps_slice->s_mode_decision.i_intra_il_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA_IL );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_intra_il_cost < i_best_cost )
					{
						i_best_mbmode = Y262_MBMODE_INTRA_IL;
						i_best_cost = ps_slice->s_mode_decision.i_intra_il_cost;
					}
				}
			}

			if( ps_slice->b_allow_skip )
			{
				ps_slice->s_mode_decision.i_skip_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_SKIP );
				if( ps_slice->s_mode_decision.i_skip_cost < i_best_cost )
				{
					i_best_cost = ps_slice->s_mode_decision.i_skip_cost;
					i_best_mbmode = Y262_MBMODE_SKIP;
				}
			}
		}

fast_skip_p:
		if( i_best_mbmode != Y262_MBMODE_INTRA && i_best_mbmode != Y262_MBMODE_INTRA_IL )
		{
			y262_encode_macroblock_inter( ps_y262, ps_slice, i_best_mbmode );

			if( i_best_mbmode != Y262_MBMODE_SKIP && ps_mb->i_cbp && ps_y262->i_quality_for_speed > 10 )
			{
				y262_qprd_mbmode( ps_y262, ps_slice, i_best_mbmode, i_best_cost );
				y262_encode_macroblock_inter( ps_y262, ps_slice, i_best_mbmode );
			}

			if( i_best_mbmode == Y262_MBMODE_SKIP )
			{
				*pb_skip = TRUE;
			}
		}
		else
		{
			if( ps_y262->i_quality_for_speed > 10 )
			{
				y262_qprd_mbmode( ps_y262, ps_slice, i_best_mbmode, i_best_cost );
			}
			y262_encode_macroblock_intra( ps_y262, ps_slice, i_best_mbmode == Y262_MBMODE_INTRA_IL );
		}
	}
	else
	{
		bool_t b_allow_skip, b_backward_pred_only;
		assert( ps_slice->i_picture_type == PICTURE_CODING_TYPE_B );

		b_backward_pred_only = ps_y262->ps_input_picture->b_backward_pred_only;

		ps_slice->s_mode_decision.i_fw_cost = MAX_COST;
		ps_slice->s_mode_decision.i_bw_cost = MAX_COST;
		ps_slice->s_mode_decision.i_bi_cost = MAX_COST;
		ps_slice->s_mode_decision.i_fw_il_cost = MAX_COST;
		ps_slice->s_mode_decision.i_bw_il_cost = MAX_COST;
		ps_slice->s_mode_decision.i_bi_il_cost = MAX_COST;
		ps_slice->s_mode_decision.i_intra_il_cost = MAX_COST;
		ps_slice->s_mode_decision.i_skip_cost = MAX_COST;


		if( !b_no_frame )
		{ 
			y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_BW );
			i_best_satd_cost = ps_slice->s_mode_decision.i_bw_cost;
			i_best_mbmode = Y262_MBMODE_BW;

			if( !b_backward_pred_only )
			{
				y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_FW );
				if( ps_slice->s_mode_decision.i_fw_cost < i_best_satd_cost )
				{
					i_best_satd_cost = ps_slice->s_mode_decision.i_fw_cost;
					i_best_mbmode = Y262_MBMODE_FW;
				}

				y262_get_mbmode_motion_bi( ps_y262, ps_slice, Y262_MBMODE_BI );
				if( ps_slice->s_mode_decision.i_bi_cost < i_best_satd_cost )
				{
					i_best_satd_cost = ps_slice->s_mode_decision.i_bi_cost;
					i_best_mbmode = Y262_MBMODE_BI;
				}
			}
		}
		else
		{
			if( b_backward_pred_only )
			{
				y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_BW_IL );
				i_best_satd_cost = ps_slice->s_mode_decision.i_bw_il_cost;
				i_best_mbmode = Y262_MBMODE_BW_IL;
			}
			else
			{
				y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_FW_IL );
				i_best_satd_cost = ps_slice->s_mode_decision.i_fw_il_cost;
				i_best_mbmode = Y262_MBMODE_FW_IL;
			}
		}

		if( ps_slice->b_allow_skip )
		{
			b_allow_skip = y262_bskip_valid( ps_y262, ps_slice );
		}
		else
		{
			b_allow_skip = FALSE;
		}

		if( ps_slice->b_allow_skip & b_allow_skip )
		{
			y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_SKIP );
			ps_slice->s_mode_decision.i_skip_cost -= ps_mb->i_lambda_sqr * 3; /* wink wink nudge nudge */
			if( i_best_satd_cost > ps_slice->s_mode_decision.i_skip_cost )
			{
				if( !y262_encode_macroblock_inter( ps_y262, ps_slice, Y262_MBMODE_SKIP ) )
				{
					//i_best_satd_cost = ps_slice->s_mode_decision.i_skip_cost;
					i_best_mbmode = Y262_MBMODE_SKIP;
					if( ps_y262->i_quality_for_speed < -5 )
					{
						goto fast_skip_b;
					}
				}
			}
			else
			{
				i_best_satd_cost = i_best_satd_cost;
			}
		}

		if( !b_no_frame )
		{
			y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA );
			if( ps_slice->s_mode_decision.i_intra_cost < i_best_satd_cost )
			{
				i_best_satd_cost = ps_slice->s_mode_decision.i_intra_cost;
				i_best_mbmode = Y262_MBMODE_INTRA;
			}
		}
		else
		{
			ps_slice->s_mode_decision.i_intra_cost = MAX_COST;
		}

		if( ps_y262->b_frame_pred_frame_dct == FALSE && !b_no_field )
		{
			if( !b_no_frame )
			{
				if( !b_backward_pred_only )
				{
					y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_FW_IL );
					if( ps_slice->s_mode_decision.i_fw_il_cost < i_best_satd_cost )
					{
						i_best_satd_cost = ps_slice->s_mode_decision.i_fw_il_cost;
						i_best_mbmode = Y262_MBMODE_FW_IL;
					}
				}
			}

			y262_get_mbmode_motion( ps_y262, ps_slice, Y262_MBMODE_BW_IL );
			if( ps_slice->s_mode_decision.i_bw_il_cost < i_best_satd_cost )
			{
				i_best_satd_cost = ps_slice->s_mode_decision.i_bw_il_cost;
				i_best_mbmode = Y262_MBMODE_BW_IL;
			}

			if( !b_backward_pred_only )
			{
				y262_get_mbmode_motion_bi( ps_y262, ps_slice, Y262_MBMODE_BI_IL );
				if( ps_slice->s_mode_decision.i_bi_il_cost < i_best_satd_cost )
				{
					i_best_satd_cost = ps_slice->s_mode_decision.i_bi_il_cost;
					i_best_mbmode = Y262_MBMODE_BI_IL;
				}
			}

			y262_get_mbmode_satd_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA_IL );
			if( ps_slice->s_mode_decision.i_intra_il_cost < i_best_satd_cost )
			{
				i_best_satd_cost = ps_slice->s_mode_decision.i_intra_il_cost;
				i_best_mbmode = Y262_MBMODE_INTRA_IL;
			}
		}

		if( ps_y262->i_quality_for_speed >= -30 )
		{
			i_best_cost = MAX_COST;
			if( ps_slice->s_mode_decision.i_fw_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
			{
				ps_slice->s_mode_decision.i_fw_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_FW );
				if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_fw_cost < i_best_cost )
				{
					i_best_mbmode = Y262_MBMODE_FW;
					i_best_cost = ps_slice->s_mode_decision.i_fw_cost;
				}
			}

			if( ps_slice->s_mode_decision.i_bw_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
			{
				ps_slice->s_mode_decision.i_bw_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_BW );
				if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_bw_cost < i_best_cost )
				{
					i_best_cost = ps_slice->s_mode_decision.i_bw_cost;
					i_best_mbmode = Y262_MBMODE_BW;
				}
			}

			if( ps_slice->s_mode_decision.i_bi_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
			{
				ps_slice->s_mode_decision.i_bi_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_BI );
				if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_bi_cost < i_best_cost )
				{
					i_best_cost = ps_slice->s_mode_decision.i_bi_cost;
					i_best_mbmode = Y262_MBMODE_BI;
				}
			}

			if( ps_slice->s_mode_decision.i_intra_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 50 + ps_y262->i_quality_for_speed ) ) ) ) )
			{
				ps_slice->s_mode_decision.i_intra_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA );
				if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_intra_cost < i_best_cost )
				{
					i_best_cost = ps_slice->s_mode_decision.i_intra_cost;
					i_best_mbmode = Y262_MBMODE_INTRA;
				}
			}

			if( ps_y262->b_frame_pred_frame_dct == FALSE )
			{
				bool_t b_try_bi = FALSE;
				if( ps_slice->s_mode_decision.i_fw_il_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
				{
					b_try_bi = TRUE;
					ps_slice->s_mode_decision.i_fw_il_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_FW_IL );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_fw_il_cost < i_best_cost )
					{
						i_best_cost = ps_slice->s_mode_decision.i_fw_il_cost;
						i_best_mbmode = Y262_MBMODE_FW_IL;
					}
				}

				if( ps_slice->s_mode_decision.i_bw_il_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
				{
					b_try_bi = TRUE;
					ps_slice->s_mode_decision.i_bw_il_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_BW_IL );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_bw_il_cost < i_best_cost )
					{
						i_best_cost = ps_slice->s_mode_decision.i_bw_il_cost;
						i_best_mbmode = Y262_MBMODE_BW_IL;
					}
				}

				if( ps_slice->s_mode_decision.i_bi_il_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 20 + ps_y262->i_quality_for_speed ) ) ) ) )
				{
					ps_slice->s_mode_decision.i_bi_il_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_BI_IL );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_bi_il_cost < i_best_cost )
					{
						i_best_cost = ps_slice->s_mode_decision.i_bi_il_cost;
						i_best_mbmode = Y262_MBMODE_BI_IL;
					}
				}
			
				if( ps_slice->s_mode_decision.i_intra_il_cost <= ( ( ps_mb->i_lambda_sqr * 5 ) + i_best_satd_cost + ( ( i_best_satd_cost / 100 ) * ( MAX( 0, 50 + ps_y262->i_quality_for_speed ) ) ) ) )
				{
					ps_slice->s_mode_decision.i_intra_il_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_INTRA_IL );
					if( i_best_cost == MAX_COST || ps_slice->s_mode_decision.i_intra_il_cost < i_best_cost )
					{
						i_best_cost = ps_slice->s_mode_decision.i_intra_il_cost;
						i_best_mbmode = Y262_MBMODE_INTRA_IL;
					}
				}
			}

			if( ps_slice->b_allow_skip & b_allow_skip )
			{
				ps_slice->s_mode_decision.i_skip_cost = y262_get_mbmode_cost( ps_y262, ps_slice, Y262_MBMODE_SKIP );
				if( ps_slice->s_mode_decision.i_skip_cost < i_best_cost )
				{
					i_best_cost = ps_slice->s_mode_decision.i_skip_cost;
					i_best_mbmode = Y262_MBMODE_SKIP;
				}
			}
		}
		
fast_skip_b:
		if( i_best_mbmode != Y262_MBMODE_INTRA && i_best_mbmode != Y262_MBMODE_INTRA_IL )
		{
			y262_encode_macroblock_inter( ps_y262, ps_slice, i_best_mbmode );

			if( i_best_mbmode != Y262_MBMODE_SKIP && ps_mb->i_cbp && ps_y262->i_quality_for_speed > 10 )
			{
				y262_qprd_mbmode( ps_y262, ps_slice, i_best_mbmode, i_best_cost );
				y262_encode_macroblock_inter( ps_y262, ps_slice, i_best_mbmode );
			}

			if( b_backward_pred_only )
			{
				if( ps_mb->i_macroblock_type & MACROBLOCK_MOTION_FORWARD )
				{
					int32_t *pi_null = NULL; /* fatal */
					*pi_null = 0;
				}
			}
			
			if( i_best_mbmode == Y262_MBMODE_SKIP )
			{
				*pb_skip = TRUE;
			}
		}
		else
		{
			if( ps_y262->i_quality_for_speed > 10 )
			{
				y262_qprd_mbmode( ps_y262, ps_slice, i_best_mbmode, i_best_cost );
			}
			y262_encode_macroblock_intra( ps_y262, ps_slice, i_best_mbmode == Y262_MBMODE_INTRA_IL );
		}
	}
}


int32_t y262_write_intra_block_get_vlc0_idx( y262_t *ps_y262, int32_t i_run, int32_t i_level )
{
	int32_t i_rl_idx;
	for( i_rl_idx = 0; rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_code != VLC_SENTINEL; i_rl_idx++ )
	{
		if( rgs_y262_dct_coefficients_lookup_table_zero[ i_rl_idx ].i_level == i_level &&
			rgs_y262_dct_coefficients_lookup_table_zero[ i_rl_idx ].i_run == i_run )
		{
			return i_rl_idx;
		}
	}
	return -1;
}

void y262_write_intra_block_mpeg2( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx )
{
	int32_t i_dct_differential, i_dct_co, i_sign, i_dc_size, i_delta, i_idx;
	y262_bitstream_t *ps_bitstream;
	y262_macroblock_t *ps_mb;

	ps_bitstream = &ps_y262->s_bitstream;
	ps_mb = &ps_slice->s_macroblock;

	i_dct_co = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ 0 ];
	i_delta = i_dct_co - ps_slice->rgi_dc_dct_pred[ i_plane_idx ];
	ps_slice->rgi_dc_dct_pred[ i_plane_idx ] = i_dct_co;

	i_sign = i_delta < 0;
	i_delta = i_delta < 0 ? ( -i_delta ) : i_delta;

	i_dc_size = 0;
	while( ( i_delta ) >= ( 1 << i_dc_size ) )
	{
		i_dc_size++;
	}
	i_dct_differential = i_delta;
	if( i_sign )
	{
		i_dct_differential = ( -i_dct_differential - 1 ) + ( 1 << i_dc_size );
	}

	if( i_plane_idx == 0 )
	{
		for( i_idx = 0; rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgi_y262_dct_dc_size_luminance_lookup_table[ i_idx ] == i_dc_size )
			{
				break;
			}
		}
		if( rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code, rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_length );
		if( i_dc_size > 0 )
		{
			y262_bitstream_write( ps_bitstream, i_dct_differential, i_dc_size );
		}
	}
	else
	{
		for( i_idx = 0; rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgi_y262_dct_dc_size_chrominance_lookup_table[ i_idx ] == i_dc_size )
			{
				break;
			}
		}
		if( rgs_y262_dct_dc_size_chrominance_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_dct_dc_size_chrominance_table[ i_idx ].i_code, rgs_y262_dct_dc_size_chrominance_table[ i_idx ].i_length );
		if( i_dc_size > 0 )
		{
			y262_bitstream_write( ps_bitstream, i_dct_differential, i_dc_size );
		}
	}

	if( ps_y262->b_intra_vlc_format )
	{
		assert( FALSE );
	}
	else
	{
		int32_t i_run, i_level, i_level_sign, i_sign, i_rl_idx, i_escape, i_eob;

		i_escape = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_ESCAPE, RUN_LEVEL_ESCAPE );
		i_eob = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_END_OF_BLOCK, RUN_LEVEL_END_OF_BLOCK );

		assert( i_escape >= 0 );
		assert( i_eob >= 0 );
		i_run = 0;
		for( i_idx = 1; i_idx < 64; i_idx++ )
		{
			i_level = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ rgui8_y262_scan_0_table[ i_idx ] ];
			if( i_level != 0 )
			{
				i_level_sign = i_level;

				if( i_level < 0 )
				{
					i_sign = 1;
					i_level = -i_level;
				}
				else
				{
					i_sign = 0;
				}

				i_rl_idx = y262_write_intra_block_get_vlc0_idx( ps_y262, i_run, i_level );

				if( i_rl_idx < 0 )
				{
					y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_code, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_length );
					y262_bitstream_write( ps_bitstream, i_run, 6 );

					y262_bitstream_write( ps_bitstream, i_level_sign & 0xfff, 12 );
				}
				else
				{
					y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_code, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_length );
					y262_bitstream_write( ps_bitstream, i_sign, 1 );
				}

				i_run = 0;
			}
			else
			{
				i_run++;
			}
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_code, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_length );
	}
}


void y262_write_intra_block_mpeg1( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx )
{
	int32_t i_dct_differential, i_dct_co, i_sign, i_dc_size, i_delta, i_idx;
	y262_bitstream_t *ps_bitstream;
	y262_macroblock_t *ps_mb;

	ps_bitstream = &ps_y262->s_bitstream;
	ps_mb = &ps_slice->s_macroblock;

	i_dct_co = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ 0 ];
	i_delta = i_dct_co - ps_slice->rgi_dc_dct_pred[ i_plane_idx ];
	ps_slice->rgi_dc_dct_pred[ i_plane_idx ] = i_dct_co;

	i_sign = i_delta < 0;
	i_delta = i_delta < 0 ? ( -i_delta ) : i_delta;

	i_dc_size = 0;
	while( ( i_delta ) >= ( 1 << i_dc_size ) )
	{
		i_dc_size++;
	}
	i_dct_differential = i_delta;
	if( i_sign )
	{
		i_dct_differential = ( -i_dct_differential - 1 ) + ( 1 << i_dc_size );
	}

	if( i_plane_idx == 0 )
	{
		for( i_idx = 0; rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgi_y262_dct_dc_size_luminance_lookup_table[ i_idx ] == i_dc_size )
			{
				break;
			}
		}
		if( rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code, rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_length );
		if( i_dc_size > 0 )
		{
			y262_bitstream_write( ps_bitstream, i_dct_differential, i_dc_size );
		}
	}
	else
	{
		for( i_idx = 0; rgs_y262_dct_dc_size_luminance_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgi_y262_dct_dc_size_chrominance_lookup_table[ i_idx ] == i_dc_size )
			{
				break;
			}
		}
		if( rgs_y262_dct_dc_size_chrominance_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_dct_dc_size_chrominance_table[ i_idx ].i_code, rgs_y262_dct_dc_size_chrominance_table[ i_idx ].i_length );
		if( i_dc_size > 0 )
		{
			y262_bitstream_write( ps_bitstream, i_dct_differential, i_dc_size );
		}
	}

	if( ps_y262->b_intra_vlc_format )
	{
		assert( FALSE );
	}
	else
	{
		int32_t i_run, i_level, i_level_sign, i_sign, i_rl_idx, i_escape, i_eob;

		i_escape = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_ESCAPE, RUN_LEVEL_ESCAPE );
		i_eob = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_END_OF_BLOCK, RUN_LEVEL_END_OF_BLOCK );

		assert( i_escape >= 0 );
		assert( i_eob >= 0 );
		i_run = 0;
		for( i_idx = 1; i_idx < 64; i_idx++ )
		{
			i_level = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ rgui8_y262_scan_0_table[ i_idx ] ];
			if( i_level != 0 )
			{
				i_level_sign = i_level;

				if( i_level < 0 )
				{
					i_sign = 1;
					i_level = -i_level;
				}
				else
				{
					i_sign = 0;
				}

				i_rl_idx = y262_write_intra_block_get_vlc0_idx( ps_y262, i_run, i_level );

				if( i_rl_idx < 0 )
				{
					y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_code, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_length );
					y262_bitstream_write( ps_bitstream, i_run, 6 );

					if( i_level_sign < -127 )
					{
						y262_bitstream_write( ps_bitstream, ( -127 & 0xff ), 8 );
						y262_bitstream_write( ps_bitstream, i_level_sign & 0xff, 8 );
					}
					else if( i_level_sign < 128 )
					{
						y262_bitstream_write( ps_bitstream, i_level_sign & 0xff, 8 );
					}
					else
					{
						y262_bitstream_write( ps_bitstream, 0, 8 );
						y262_bitstream_write( ps_bitstream, i_level_sign & 0xff, 8 );
					}
				}
				else
				{
					y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_code, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_length );
					y262_bitstream_write( ps_bitstream, i_sign, 1 );
				}

				i_run = 0;
			}
			else
			{
				i_run++;
			}
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_code, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_length );
	}
}

void y262_write_intra_block( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx )
{
	if( !ps_y262->b_sequence_mpeg1 )
	{
		y262_write_intra_block_mpeg2( ps_y262, ps_slice, i_plane_idx, i_blk_idx );
	}
	else
	{
		y262_write_intra_block_mpeg1( ps_y262, ps_slice, i_plane_idx, i_blk_idx );
	}
}


void y262_write_inter_block_mpeg2( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx )
{
	int32_t i_sign, i_idx;
	int32_t i_run, i_level, i_level_sign, i_rl_idx, i_escape, i_eob;
	y262_bitstream_t *ps_bitstream;
	y262_macroblock_t *ps_mb;

	ps_bitstream = &ps_y262->s_bitstream;
	ps_mb = &ps_slice->s_macroblock;

	i_escape = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_ESCAPE, RUN_LEVEL_ESCAPE );
	i_eob = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_END_OF_BLOCK, RUN_LEVEL_END_OF_BLOCK );

	assert( i_escape >= 0 );
	assert( i_eob >= 0 );
	i_run = 0;
	for( i_idx = 0; i_idx < 64; i_idx++ )
	{
		i_level = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ rgui8_y262_scan_0_table[ i_idx ] ];
		if( i_level != 0 )
		{
			i_level_sign = i_level;

			if( i_level < 0 )
			{
				i_sign = 1;
				i_level = -i_level;
			}
			else
			{
				i_sign = 0;
			}

			i_rl_idx = y262_write_intra_block_get_vlc0_idx( ps_y262, i_run, i_level );

			if( i_rl_idx < 0 )
			{
				y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_code, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_length );
				y262_bitstream_write( ps_bitstream, i_run, 6 );

				y262_bitstream_write( ps_bitstream, i_level_sign & 0xfff, 12 );
			}
			else
			{
				if( i_idx == 0 && i_level == 1 )
				{
					y262_bitstream_write( ps_bitstream, 1, 1 ); /* special case */
				}
				else
				{
					y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_code, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_length );
				}

				y262_bitstream_write( ps_bitstream, i_sign, 1 );
			}

			i_run = 0;
		}
		else
		{
			i_run++;
		}
	}
	y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_code, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_length );
}

void y262_write_inter_block_mpeg1( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx )
{
	int32_t i_sign, i_idx;
	int32_t i_run, i_level, i_level_sign, i_rl_idx, i_escape, i_eob;
	y262_bitstream_t *ps_bitstream;
	y262_macroblock_t *ps_mb;

	ps_bitstream = &ps_y262->s_bitstream;
	ps_mb = &ps_slice->s_macroblock;

	i_escape = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_ESCAPE, RUN_LEVEL_ESCAPE );
	i_eob = y262_write_intra_block_get_vlc0_idx( ps_y262, RUN_LEVEL_END_OF_BLOCK, RUN_LEVEL_END_OF_BLOCK );

	assert( i_escape >= 0 );
	assert( i_eob >= 0 );
	i_run = 0;
	for( i_idx = 0; i_idx < 64; i_idx++ )
	{
		i_level = ps_mb->rgi16_coeffs[ i_plane_idx ][ i_blk_idx ][ rgui8_y262_scan_0_table[ i_idx ] ];
		if( i_level != 0 )
		{
			i_level_sign = i_level;

			if( i_level < 0 )
			{
				i_sign = 1;
				i_level = -i_level;
			}
			else
			{
				i_sign = 0;
			}

			i_rl_idx = y262_write_intra_block_get_vlc0_idx( ps_y262, i_run, i_level );

			if( i_rl_idx < 0 )
			{
				y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_code, rgs_y262_dct_coefficients_table_zero[ i_escape ].i_length );
				y262_bitstream_write( ps_bitstream, i_run, 6 );

				if( i_level_sign < -127 )
				{
					y262_bitstream_write( ps_bitstream, ( -127 & 0xff ), 8 );
					y262_bitstream_write( ps_bitstream, i_level_sign & 0xff, 8 );
				}
				else if( i_level_sign < 128 )
				{
					y262_bitstream_write( ps_bitstream, i_level_sign & 0xff, 8 );
				}
				else
				{
					y262_bitstream_write( ps_bitstream, 0, 8 );
					y262_bitstream_write( ps_bitstream, i_level_sign & 0xff, 8 );
				}
			}
			else
			{
				if( i_idx == 0 && i_level == 1 )
				{
					y262_bitstream_write( ps_bitstream, 1, 1 ); /* special case */
				}
				else
				{
					y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_code, rgs_y262_dct_coefficients_table_zero[ i_rl_idx ].i_length );
				}

				y262_bitstream_write( ps_bitstream, i_sign, 1 );
			}

			i_run = 0;
		}
		else
		{
			i_run++;
		}
	}
	y262_bitstream_write( ps_bitstream, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_code, rgs_y262_dct_coefficients_table_zero[ i_eob ].i_length );
}


void y262_write_inter_block( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx )
{
	if( !ps_y262->b_sequence_mpeg1 )
	{
		y262_write_inter_block_mpeg2( ps_y262, ps_slice, i_plane_idx, i_blk_idx );
	}
	else
	{
		y262_write_inter_block_mpeg1( ps_y262, ps_slice, i_plane_idx, i_blk_idx );
	}
}



void y262_write_motion_vector_delta( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_fcode, int32_t i_mv )
{
	int32_t i_idx, i_mv_delta, i_residual, i_sign, i_fcode_minus_one;
	y262_bitstream_t *ps_bitstream;
	y262_macroblock_t *ps_mb;

	ps_bitstream = &ps_y262->s_bitstream;
	ps_mb = &ps_slice->s_macroblock;

	i_fcode_minus_one = i_fcode - 1;

	if( i_mv == 0 )
	{
		i_mv_delta = 0;
	}
	else
	{
		if( i_mv < -( 1 << ( 4 + i_fcode_minus_one ) ) )
		{
			i_mv += ( 1 << ( 5 + i_fcode_minus_one ) );
			assert( i_mv >= -( 1 << ( 4 + i_fcode_minus_one ) ) );
		}
		else if( i_mv >= ( 1 << ( 4 + i_fcode_minus_one ) ) )
		{
			i_mv -= ( 1 << ( 5 + i_fcode_minus_one ) );
			assert( i_mv < ( 1 << ( 4 + i_fcode_minus_one ) ) );
		}

		if( i_mv < 0 )
		{
			i_sign = 1;
			i_mv = -i_mv;
		}
		else
		{
			i_sign = 0;
		}
		i_residual = ( i_mv - 1 ) & ( ( 1 << i_fcode_minus_one ) - 1 );
		i_mv_delta = 1 + ( ( i_mv - 1 ) >> i_fcode_minus_one );
		if( i_sign )
		{
			i_mv_delta = -i_mv_delta;
		}
	}
	for( i_idx = 0; rgs_y262_motion_code_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
	{
		if( rgi_y262_motion_delta_lookup_table[ i_idx ] == i_mv_delta )
		{
			break;
		}
	}
	if( rgs_y262_motion_code_table[ i_idx ].i_code == VLC_SENTINEL )
	{
		assert( FALSE );
	}
	y262_bitstream_write( ps_bitstream, rgs_y262_motion_code_table[ i_idx ].i_code, rgs_y262_motion_code_table[ i_idx ].i_length );
	if( i_mv_delta != 0 )
	{
		y262_bitstream_write( ps_bitstream, i_residual, i_fcode_minus_one );
	}
}


int32_t y262_write_macroblock( y262_t *ps_y262, y262_slice_t *ps_slice )
{
	int32_t i_idx, i_bits_start, i_num_mv;
	int32_t i_motion_type;
	uint32_t ui_mb_type;
	y262_bitstream_t *ps_bitstream;
	y262_macroblock_t *ps_mb;

	ps_bitstream = &ps_y262->s_bitstream;
	ps_mb = &ps_slice->s_macroblock;

	i_bits_start = ( int32_t )( ( ( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream ) * 8 ) + ps_bitstream->i_codeword_fill );

	while( ps_slice->i_skip_run >= 0 )
	{
		int32_t i_mb_inc;

		if( ps_slice->i_skip_run > 33 )
		{
			i_mb_inc = 33;
		}
		else
		{
			i_mb_inc = ps_slice->i_skip_run;
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_macroblock_address_increment_table[ i_mb_inc ].i_code, rgs_y262_macroblock_address_increment_table[ i_mb_inc ].i_length );

		ps_slice->i_skip_run -= i_mb_inc;
		if( i_mb_inc < 33 )
		{
			break;
		}
	}

	ui_mb_type = ps_mb->i_macroblock_type & 0x1f;
	i_motion_type = ps_mb->i_macroblock_type / MACROBLOCK_MOTION_TYPE;
	if( ps_mb->i_quantizer != ps_slice->i_quantizer && ps_mb->i_cbp )
	{
		ui_mb_type |= MACROBLOCK_QUANT;
	}
	if( !( ui_mb_type & MACROBLOCK_INTRA ) && ps_mb->i_cbp )
	{
		ui_mb_type |= MACROBLOCK_PATTERN;
	}
	if( ps_slice->i_picture_type == PICTURE_CODING_TYPE_I )
	{
		for( i_idx = 0; rgs_y262_macroblock_type_i_picture_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgui_y262_macroblock_type_i_picture_flags_table[ i_idx ] == ui_mb_type )
			{
				break;
			}
		}
		if( rgs_y262_macroblock_type_i_picture_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_macroblock_type_i_picture_table[ i_idx ].i_code, rgs_y262_macroblock_type_i_picture_table[ i_idx ].i_length );
		if( !ps_y262->b_frame_pred_frame_dct && 1 /* PICTURE_CODING_STRUCTURE_FRAME */ )
		{
			y262_bitstream_write( ps_bitstream, ( ps_mb->i_macroblock_type & MACROBLOCK_INTERLACED ) ? 1 : 0, 1 );
		}
	}
	else if( ps_slice->i_picture_type == PICTURE_CODING_TYPE_P )
	{
		for( i_idx = 0; rgs_y262_macroblock_type_p_picture_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgui_y262_macroblock_type_p_picture_flags_table[ i_idx ] == ui_mb_type )
			{
				break;
			}
		}
		if( rgs_y262_macroblock_type_p_picture_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_macroblock_type_p_picture_table[ i_idx ].i_code, rgs_y262_macroblock_type_p_picture_table[ i_idx ].i_length );
		if( !ps_y262->b_frame_pred_frame_dct && 1 /* PICTURE_CODING_STRUCTURE_FRAME */ )
		{
			if( ui_mb_type & MACROBLOCK_MOTION_FORWARD )
			{
				y262_bitstream_write( ps_bitstream, i_motion_type, 2 );
			}
			if( ui_mb_type & ( MACROBLOCK_INTRA | MACROBLOCK_PATTERN ) )
			{
				y262_bitstream_write( ps_bitstream, ( ps_mb->i_macroblock_type & MACROBLOCK_INTERLACED ) ? 1 : 0, 1 );
			}
			i_num_mv = i_motion_type == FRAME_MOTION_TYPE_FRAME ? 1 : 2;
		}
		else
		{
			i_num_mv = 1;
		}
	}
	else
	{
		assert( ps_slice->i_picture_type == PICTURE_CODING_TYPE_B );

		for( i_idx = 0; rgs_y262_macroblock_type_b_picture_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgui_y262_macroblock_type_b_picture_flags_table[ i_idx ] == ui_mb_type )
			{
				break;
			}
		}
		if( rgs_y262_macroblock_type_b_picture_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_macroblock_type_b_picture_table[ i_idx ].i_code, rgs_y262_macroblock_type_b_picture_table[ i_idx ].i_length );
		if( !ps_y262->b_frame_pred_frame_dct && 1 /* PICTURE_CODING_STRUCTURE_FRAME */ )
		{
			if( !( ui_mb_type & MACROBLOCK_INTRA ) )
			{
				y262_bitstream_write( ps_bitstream, i_motion_type, 2 );
			}
			if( ui_mb_type & ( MACROBLOCK_INTRA | MACROBLOCK_PATTERN ) )
			{
				y262_bitstream_write( ps_bitstream, ( ps_mb->i_macroblock_type & MACROBLOCK_INTERLACED ) ? 1 : 0, 1 );
			}
			i_num_mv = i_motion_type == FRAME_MOTION_TYPE_FRAME ? 1 : 2;
		}
		else
		{
			i_num_mv = 1;
		}
	}

	if( ui_mb_type & MACROBLOCK_QUANT )
	{
		y262_bitstream_write( ps_bitstream, ps_mb->i_quantizer, 5 );
		ps_slice->i_quantizer = ps_mb->i_quantizer;
	}

	if( ui_mb_type & MACROBLOCK_MOTION_FORWARD )
	{
		if( i_num_mv == 1 )
		{
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 0 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_x - ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 0 ] );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 1 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_y - ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 1 ] );
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 0 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_x;
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 1 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_y;
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_FORWARD ][ 0 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_x;
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_FORWARD ][ 1 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_y;
		}
		else
		{
			y262_bitstream_write( ps_bitstream, ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_field, 1 );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 0 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_x - ( ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 0 ] ) );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 1 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_y - ( ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 1 ] >> 1 ) );
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 0 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_x;
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_FORWARD ][ 1 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_FORWARD ].i_y << 1;
			y262_bitstream_write( ps_bitstream, ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_FORWARD ].i_field, 1 );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 0 ], ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_FORWARD ].i_x - ( ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_FORWARD ][ 0 ] ) );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_FORWARD ][ 1 ], ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_FORWARD ].i_y - ( ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_FORWARD ][ 1 ] >> 1 ) );
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_FORWARD ][ 0 ] = ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_FORWARD ].i_x;
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_FORWARD ][ 1 ] = ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_FORWARD ].i_y << 1;
		}
	}
	if( ui_mb_type & MACROBLOCK_MOTION_BACKWARD )
	{
		if( i_num_mv == 1 )
		{
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 0 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_x - ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 0 ] );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 1 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_y - ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 1 ] );
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 0 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_x;
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 1 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_y;
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_BACKWARD ][ 0 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_x;
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_BACKWARD ][ 1 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_y;
		}
		else
		{
			y262_bitstream_write( ps_bitstream, ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_field, 1 );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 0 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_x - ( ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 0 ] ) );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 1 ], ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_y - ( ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 1 ] >> 1 ) );
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 0 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_x;
			ps_slice->rgi_pmv[ 0 ][ PICTURE_CODING_BACKWARD ][ 1 ] = ps_mb->rgs_motion[ 0 ][ PICTURE_CODING_BACKWARD ].i_y << 1;
			y262_bitstream_write( ps_bitstream, ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_BACKWARD ].i_field, 1 );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 0 ], ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_BACKWARD ].i_x - ( ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_BACKWARD ][ 0 ] ) );
			y262_write_motion_vector_delta( ps_y262, ps_slice, ps_y262->rgi_fcode[ PICTURE_CODING_BACKWARD ][ 1 ], ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_BACKWARD ].i_y - ( ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_BACKWARD ][ 1 ] >> 1 ) );
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_BACKWARD ][ 0 ] = ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_BACKWARD ].i_x;
			ps_slice->rgi_pmv[ 1 ][ PICTURE_CODING_BACKWARD ][ 1 ] = ps_mb->rgs_motion[ 1 ][ PICTURE_CODING_BACKWARD ].i_y << 1;
		}
	}

	if( ui_mb_type & MACROBLOCK_INTRA )
	{
		for( i_idx = 0; i_idx < 4; i_idx++ )
		{
			y262_write_intra_block( ps_y262, ps_slice, 0, i_idx );
		}
		switch( ps_y262->i_sequence_chroma_format )
		{
			case Y262_CHROMA_FORMAT_420:
				y262_write_intra_block( ps_y262, ps_slice, 1, 0 );
				y262_write_intra_block( ps_y262, ps_slice, 2, 0 );
				break;
			case Y262_CHROMA_FORMAT_422:
				y262_write_intra_block( ps_y262, ps_slice, 1, 0 );
				y262_write_intra_block( ps_y262, ps_slice, 2, 0 );
				y262_write_intra_block( ps_y262, ps_slice, 1, 1 );
				y262_write_intra_block( ps_y262, ps_slice, 2, 1 );
				break;
			case Y262_CHROMA_FORMAT_444:
				y262_write_intra_block( ps_y262, ps_slice, 1, 0 );
				y262_write_intra_block( ps_y262, ps_slice, 2, 0 );
				y262_write_intra_block( ps_y262, ps_slice, 1, 2 );
				y262_write_intra_block( ps_y262, ps_slice, 2, 2 );
				y262_write_intra_block( ps_y262, ps_slice, 1, 1 );
				y262_write_intra_block( ps_y262, ps_slice, 2, 1 );
				y262_write_intra_block( ps_y262, ps_slice, 1, 3 );
				y262_write_intra_block( ps_y262, ps_slice, 2, 3 );
				break;
		}
	}
	else if( ui_mb_type & MACROBLOCK_PATTERN )
	{
		uint32_t ui_pattern, ui_chroma_extra, ui_chroma_extra_len;

		ui_pattern = ps_mb->rgb_cbp[ 0 ][ 0 ] << 5;
		ui_pattern |= ps_mb->rgb_cbp[ 0 ][ 1 ] << 4;
		ui_pattern |= ps_mb->rgb_cbp[ 0 ][ 2 ] << 3;
		ui_pattern |= ps_mb->rgb_cbp[ 0 ][ 3 ] << 2;
		switch( ps_y262->i_sequence_chroma_format )
		{
			case Y262_CHROMA_FORMAT_420:
				ui_pattern |= ps_mb->rgb_cbp[ 1 ][ 0 ] << 1;
				ui_pattern |= ps_mb->rgb_cbp[ 2 ][ 0 ] << 0;
				ui_chroma_extra = ui_chroma_extra_len = 0;
				break;
			case Y262_CHROMA_FORMAT_422:
				ui_pattern |= ps_mb->rgb_cbp[ 1 ][ 0 ] << 1;
				ui_pattern |= ps_mb->rgb_cbp[ 2 ][ 0 ] << 0;
				ui_chroma_extra = 0;
				ui_chroma_extra_len = 2;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 1 ][ 1 ] << 1;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 2 ][ 1 ] << 0;
				break;
			case Y262_CHROMA_FORMAT_444:
				ui_pattern |= ps_mb->rgb_cbp[ 1 ][ 0 ] << 1;
				ui_pattern |= ps_mb->rgb_cbp[ 2 ][ 0 ] << 0;
				ui_chroma_extra = 0;
				ui_chroma_extra_len = 6;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 1 ][ 2 ] << 5;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 2 ][ 2 ] << 4;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 1 ][ 1 ] << 3;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 2 ][ 1 ] << 2;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 1 ][ 3 ] << 1;
				ui_chroma_extra |= ps_mb->rgb_cbp[ 2 ][ 3 ] << 0;
				break;
		}
		for( i_idx = 0; rgs_y262_coded_block_pattern_table[ i_idx ].i_code != VLC_SENTINEL; i_idx++ )
		{
			if( rgi_y262_coded_block_pattern_lookup_table[ i_idx ] == ui_pattern )
			{
				break;
			}
		}
		if( rgs_y262_coded_block_pattern_table[ i_idx ].i_code == VLC_SENTINEL )
		{
			assert( FALSE );
		}
		y262_bitstream_write( ps_bitstream, rgs_y262_coded_block_pattern_table[ i_idx ].i_code, rgs_y262_coded_block_pattern_table[ i_idx ].i_length );
		if( ui_chroma_extra_len > 0 )
		{
			y262_bitstream_write( ps_bitstream, ui_chroma_extra, ui_chroma_extra_len );
		}

		for( i_idx = 0; i_idx < 4; i_idx++ )
		{
			if( ps_mb->rgb_cbp[ 0 ][ i_idx ] )
			{
				y262_write_inter_block( ps_y262, ps_slice, 0, i_idx );
			}
		}
		switch( ps_y262->i_sequence_chroma_format )
		{
			case Y262_CHROMA_FORMAT_420:
				if( ps_mb->rgb_cbp[ 1 ][ 0 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 1, 0 );
				}
				if( ps_mb->rgb_cbp[ 2 ][ 0 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 2, 0 );
				}
				break;
			case Y262_CHROMA_FORMAT_422:
				if( ps_mb->rgb_cbp[ 1 ][ 0 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 1, 0 );
				}
				if( ps_mb->rgb_cbp[ 2 ][ 0 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 2, 0 );
				}
				if( ps_mb->rgb_cbp[ 1 ][ 1 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 1, 1 );
				}
				if( ps_mb->rgb_cbp[ 2 ][ 1 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 2, 1 );
				}
				break;
			case Y262_CHROMA_FORMAT_444:
				if( ps_mb->rgb_cbp[ 1 ][ 0 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 1, 0 );
				}
				if( ps_mb->rgb_cbp[ 2 ][ 0 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 2, 0 );
				}
				if( ps_mb->rgb_cbp[ 1 ][ 2 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 1, 2 );
				}
				if( ps_mb->rgb_cbp[ 2 ][ 2 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 2, 2 );
				}
				if( ps_mb->rgb_cbp[ 1 ][ 1 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 1, 1 );
				}
				if( ps_mb->rgb_cbp[ 2 ][ 1 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 2, 1 );
				}
				if( ps_mb->rgb_cbp[ 1 ][ 3 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 1, 3 );
				}
				if( ps_mb->rgb_cbp[ 2 ][ 3 ] )
				{
					y262_write_inter_block( ps_y262, ps_slice, 2, 3 );
				}
				break;
		}
	}

	if( ui_mb_type & MACROBLOCK_INTRA || !( ui_mb_type & ( MACROBLOCK_MOTION_FORWARD | MACROBLOCK_MOTION_BACKWARD ) ) )
	{
		y262_slice_reset_predictors_inter( ps_y262, ps_slice );
	}
	if( !( ui_mb_type & MACROBLOCK_INTRA ) )
	{
		y262_slice_reset_predictors_intra( ps_y262, ps_slice );
	}
	ps_slice->i_last_mb_motion_flags = ( ui_mb_type & ( MACROBLOCK_MOTION_FORWARD | MACROBLOCK_MOTION_BACKWARD ) );


	return ( int32_t )( ( ( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream ) * 8 ) + ps_bitstream->i_codeword_fill - i_bits_start );
}

int32_t y262_get_inter_block_bits( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_plane_idx, int32_t i_blk_idx )
{
	int32_t i_bits_start, i_bits;
	y262_bitstream_t s_saved_bitstream, *ps_bitstream;
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;
	s_saved_bitstream = ps_y262->s_bitstream;
	ps_bitstream = &ps_y262->s_bitstream;

	i_bits_start = ( int32_t )( ( ( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream ) * 8 ) + ps_bitstream->i_codeword_fill );

	y262_write_inter_block( ps_y262, ps_slice, i_plane_idx, i_blk_idx );

	i_bits = ( int32_t )( ( ( ( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream ) * 8 ) + ps_bitstream->i_codeword_fill ) - i_bits_start );

	ps_y262->s_bitstream = s_saved_bitstream;

	return i_bits;
}

int32_t y262_get_mbmode_cost( y262_t *ps_y262, y262_slice_t *ps_slice, int32_t i_mbmode )
{
	int32_t i_saved_quantizer, i_bits, i_ssd, i_saved_skip_run, i_saved_mb_motion;
	int32_t rgi_saved_pmv[ 8 ];
	int32_t rgi_saved_intra_dc[ 3 ];
	y262_bitstream_t s_saved_bitstream;
	y262_macroblock_t *ps_mb;

	ps_mb = &ps_slice->s_macroblock;

	i_saved_quantizer = ps_slice->i_quantizer;
	rgi_saved_pmv[ 0 ] = ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ];
	rgi_saved_pmv[ 1 ] = ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ];
	rgi_saved_pmv[ 2 ] = ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ];
	rgi_saved_pmv[ 3 ] = ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ];
	rgi_saved_pmv[ 4 ] = ps_slice->rgi_pmv[ 1 ][ 0 ][ 0 ];
	rgi_saved_pmv[ 5 ] = ps_slice->rgi_pmv[ 1 ][ 0 ][ 1 ];
	rgi_saved_pmv[ 6 ] = ps_slice->rgi_pmv[ 1 ][ 1 ][ 0 ];
	rgi_saved_pmv[ 7 ] = ps_slice->rgi_pmv[ 1 ][ 1 ][ 1 ];
	rgi_saved_intra_dc[ 0 ] = ps_slice->rgi_dc_dct_pred[ 0 ];
	rgi_saved_intra_dc[ 1 ] = ps_slice->rgi_dc_dct_pred[ 1 ];
	rgi_saved_intra_dc[ 2 ] = ps_slice->rgi_dc_dct_pred[ 2 ];
	s_saved_bitstream = ps_y262->s_bitstream;
	i_saved_skip_run = ps_slice->i_skip_run;
	i_saved_mb_motion = ps_slice->i_last_mb_motion_flags;

	if( i_mbmode == Y262_MBMODE_INTRA || i_mbmode == Y262_MBMODE_INTRA_IL )
	{
		y262_encode_macroblock_intra( ps_y262, ps_slice, i_mbmode == Y262_MBMODE_INTRA_IL );
	}
	else
	{
		y262_encode_macroblock_inter( ps_y262, ps_slice, i_mbmode );
	}
	i_ssd = ps_y262->s_funcs.f_ssd_16x16( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, ps_mb->pui8_dst_luma, ps_mb->i_dst_luma_stride );
	if( ps_y262->i_psyrd_strength > 0 )
	{
		int32_t i_src_cplx, i_rec_cplx, i_cost;
		bool_t b_interlaced;
		static const uint8_t rgui8_zero[ 16 ] = { 0, };

		b_interlaced = i_mbmode == Y262_MBMODE_INTRA_IL || i_mbmode == Y262_MBMODE_FW_IL || i_mbmode == Y262_MBMODE_BW_IL || i_mbmode == Y262_MBMODE_BI_IL;
		if( !b_interlaced )
		{
			i_src_cplx = ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x16 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, ( uint8_t *)rgui8_zero, 0 );
			i_src_cplx -= ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x16 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride, ( uint8_t *)rgui8_zero, 0 ) >> 2;
			i_rec_cplx = ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x16 ]( ps_mb->pui8_dst_luma, ps_mb->i_dst_luma_stride, ( uint8_t *)rgui8_zero, 0 );
			i_rec_cplx -= ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x16 ]( ps_mb->pui8_dst_luma, ps_mb->i_dst_luma_stride, ( uint8_t *)rgui8_zero, 0 ) >> 2;
			i_cost = abs( i_src_cplx - i_rec_cplx );
		}
		else
		{
			i_src_cplx = ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
			i_src_cplx -= ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_src_luma, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 ) >> 2;
			i_rec_cplx = ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_dst_luma, ps_mb->i_dst_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
			i_rec_cplx -= ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_dst_luma, ps_mb->i_dst_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 ) >> 2;
			i_cost = abs( i_src_cplx - i_rec_cplx );
			i_src_cplx = ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_src_luma + ps_mb->i_src_luma_stride, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
			i_src_cplx -= ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_src_luma + ps_mb->i_src_luma_stride, ps_mb->i_src_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 ) >> 2;
			i_rec_cplx = ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_dst_luma + ps_mb->i_dst_luma_stride, ps_mb->i_dst_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 );
			i_rec_cplx -= ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x8 ]( ps_mb->pui8_dst_luma + ps_mb->i_dst_luma_stride, ps_mb->i_dst_luma_stride * 2, ( uint8_t *)rgui8_zero, 0 ) >> 2;
			i_cost += abs( i_src_cplx - i_rec_cplx );
		}
		i_cost = ( i_cost * ps_y262->i_psyrd_strength * ps_mb->i_lambda_sqr ) >> 8;
		i_ssd += i_cost;
	}
	i_ssd += ps_y262->s_funcs.f_ssd_8x8( ps_mb->pui8_src_cb, ps_mb->i_src_chroma_stride, ps_mb->pui8_dst_cb, ps_mb->i_dst_chroma_stride );
	i_ssd += ps_y262->s_funcs.f_ssd_8x8( ps_mb->pui8_src_cr, ps_mb->i_src_chroma_stride, ps_mb->pui8_dst_cr, ps_mb->i_dst_chroma_stride );
	if( i_mbmode != Y262_MBMODE_SKIP )
	{
		i_bits = y262_write_macroblock( ps_y262, ps_slice );
	}
	else
	{
		i_bits = 1;
	}

	ps_slice->i_quantizer = i_saved_quantizer;
	ps_slice->rgi_pmv[ 0 ][ 0 ][ 0 ] = rgi_saved_pmv[ 0 ];
	ps_slice->rgi_pmv[ 0 ][ 0 ][ 1 ] = rgi_saved_pmv[ 1 ];
	ps_slice->rgi_pmv[ 0 ][ 1 ][ 0 ] = rgi_saved_pmv[ 2 ];
	ps_slice->rgi_pmv[ 0 ][ 1 ][ 1 ] = rgi_saved_pmv[ 3 ];
	ps_slice->rgi_pmv[ 1 ][ 0 ][ 0 ] = rgi_saved_pmv[ 4 ];
	ps_slice->rgi_pmv[ 1 ][ 0 ][ 1 ] = rgi_saved_pmv[ 5 ];
	ps_slice->rgi_pmv[ 1 ][ 1 ][ 0 ] = rgi_saved_pmv[ 6 ];
	ps_slice->rgi_pmv[ 1 ][ 1 ][ 1 ] = rgi_saved_pmv[ 7 ];
	ps_slice->rgi_dc_dct_pred[ 0 ] = rgi_saved_intra_dc[ 0 ];
	ps_slice->rgi_dc_dct_pred[ 1 ] = rgi_saved_intra_dc[ 1 ];
	ps_slice->rgi_dc_dct_pred[ 2 ] = rgi_saved_intra_dc[ 2 ];
	ps_y262->s_bitstream = s_saved_bitstream;
	ps_slice->i_skip_run = i_saved_skip_run;
	ps_slice->i_last_mb_motion_flags = i_saved_mb_motion;

	return i_ssd + ( ( i_bits * ( ps_mb->i_lambda ) ) >> Y262_LAMBDA_BITS );
}


int32_t y262_get_quantizer_from_quantizer_f8( y262_t *ps_y262, int32_t i_quantizer_f8, int32_t *pi_lambda_quantizer_f8 )
{
	int32_t i_scale, i_idx, i_best_delta, i_best_idx, i_linear_int_quantizer;

	if( ps_y262->b_qscale_type )
	{
		i_scale = ( 128 << 8 ) / 32;
	}
	else
	{
		i_scale = ( 64 << 8 ) / 32;
	}

	i_quantizer_f8 = ( ( i_quantizer_f8 * i_scale ) + 128 ) >> 8;
	*pi_lambda_quantizer_f8 = i_quantizer_f8;

	i_linear_int_quantizer = ( i_quantizer_f8 + 128 ) >> 8;

	i_best_delta = 256;
	i_best_idx = 1;

	for( i_idx = 1; i_idx < 32; i_idx++ )
	{
		int32_t i_delta;

		i_delta = abs( i_linear_int_quantizer - rgi8_y262_quantiser_scale_table[ ps_y262->b_qscale_type ][ i_idx ] );
		if( i_best_delta > i_delta )
		{
			i_best_delta = i_delta;
			i_best_idx = i_idx;
		}
	}

	return i_best_idx;
}

void y262_encode_slice( y262_t *ps_y262, y262_slice_t *ps_slice )
{
	int32_t i_mb_idx, i_bits;
	bool_t b_skip;

	y262_slice_reset_predictors_intra( ps_y262, ps_slice );
	y262_slice_reset_predictors_inter( ps_y262, ps_slice );
	ps_slice->i_skip_run = 0;

	for( i_mb_idx = ps_slice->i_start_mb_addr; i_mb_idx <= ps_slice->i_end_mb_addr; i_mb_idx++ )
	{
		if( i_mb_idx == 71 )
		{
			i_mb_idx = i_mb_idx;
		}
		
		ps_slice->b_allow_skip = i_mb_idx != ps_slice->i_start_mb_addr && i_mb_idx != ps_slice->i_end_mb_addr;
		ps_slice->b_allow_skip = ps_slice->b_allow_skip && ( ( ps_slice->i_last_mb_motion_flags & ( MACROBLOCK_MOTION_FORWARD | MACROBLOCK_MOTION_BACKWARD ) ) || ( ps_slice->i_picture_type == PICTURE_CODING_TYPE_P ) );

		y262_init_macroblock( ps_y262, ps_slice, i_mb_idx );


		if( ps_slice->s_macroblock.i_mb_x == 160 && ps_slice->s_macroblock.i_mb_y == 48 )
		{
			ps_slice = ps_slice;
		}
		/* dumped quantizer here */
		{
			float f_lambda;
			int32_t i_quantizer_f8, i_lambda_quantizer_f8;
			y262_macroblock_t *ps_mb;

			ps_mb = &ps_slice->s_macroblock;
			
			i_quantizer_f8 = y262_ratectrl_get_slice_mb_quantizer( ps_y262, &ps_y262->s_slice_encoder_ratectrl, i_mb_idx );
			
			/* mbtree */
			i_quantizer_f8 = ( ( i_quantizer_f8 ) * ( ( ps_y262->ps_input_picture->ps_lookahead[ i_mb_idx ].i_quantizer_scale ) ) ) / ( 1 << 12 );

			/* variance aq */
			if( ps_y262->b_variance_aq )
			{
				i_quantizer_f8 = ( i_quantizer_f8 * ps_y262->ps_input_picture->ps_lookahead[ i_mb_idx ].i_quantizer_aq_scale ) / ( 1 << 12 );
			}
			
			ps_mb->i_quantizer = y262_get_quantizer_from_quantizer_f8( ps_y262, i_quantizer_f8, &i_lambda_quantizer_f8 );
			ps_mb->i_quantizer = MIN( 31, MAX( 1, ps_mb->i_quantizer ) );
			i_lambda_quantizer_f8 = MIN( 128 << 8, MAX( 1 << 8, i_lambda_quantizer_f8 ) );
			ps_mb->i_scaled_quantizer = rgi8_y262_quantiser_scale_table[ ps_y262->b_qscale_type ][ ps_mb->i_quantizer ];
			f_lambda = ( ( ps_mb->i_scaled_quantizer * ps_mb->i_scaled_quantizer ) * ( 0.4f * 0.4f ) );
			/*ps_mb->i_lambda = ( int32_t ) ( ( ( ( i_lambda_quantizer_f8 * i_lambda_quantizer_f8 ) + ( 1 << 15 ) ) >> 16 ) * ( 0.4 * 0.4 ) );*/
			ps_mb->i_lambda = ( int32_t ) ( floorf( MAX( 1.0f, f_lambda + 0.5f ) * ( 1 << Y262_LAMBDA_BITS ) ) );
			ps_mb->i_lambda_sqr = ( int32_t ) floorf( MAX( 1.0f, sqrtf( f_lambda ) + 0.5f ) );

		}

		y262_encode_macroblock( ps_y262, ps_slice, i_mb_idx, &b_skip );

		
		if( !b_skip )
		{
			i_bits = y262_write_macroblock( ps_y262, ps_slice );
		}
		else
		{
			assert( ps_slice->b_allow_skip );

			if( ps_slice->i_picture_type == PICTURE_CODING_TYPE_P )
			{
				y262_slice_reset_predictors_inter( ps_y262, ps_slice );
			}
			y262_slice_reset_predictors_intra( ps_y262, ps_slice );

			ps_slice->i_skip_run++;

			i_bits = 1;
		}
		y262_ratectrl_update_slice_mb( ps_y262, &ps_y262->s_slice_encoder_ratectrl, i_mb_idx, i_bits );
	}
}

void y262_write_slice( y262_t *ps_y262, int32_t i_picture_type, int32_t i_slice_row, y262_slice_t *ps_slice )
{
	y262_bitstream_t *ps_bitstream;

	ps_bitstream = &ps_y262->s_bitstream;

	y262_bitstream_write( ps_bitstream, 1, 24 );

	ps_slice->i_quantizer_f8 = ps_y262->s_ratectrl.i_quantizer;
	ps_slice->s_slice_header.i_quantizer_scale_code = MAX( 1, MIN( 31, ( ps_slice->i_quantizer_f8 >> 8 ) ) );
	ps_slice->s_slice_header.b_intra_slice_flag = FALSE;
	ps_slice->s_slice_header.b_intra_slice = ps_slice->s_slice_header.b_intra_slice_flag;
	ps_slice->i_picture_type = i_picture_type;
	ps_slice->i_mb_addr = i_slice_row * ( ps_y262->i_sequence_width >> 4 );
	ps_slice->i_start_mb_addr = ps_slice->i_mb_addr;
	ps_slice->i_end_mb_addr = ps_slice->i_start_mb_addr + ( ps_y262->i_sequence_width >> 4 ) - 1;
	ps_slice->i_quantizer = ps_slice->s_slice_header.i_quantizer_scale_code;

	if( ps_y262->i_sequence_height > 2800 )
	{
		int32_t i_slice_vertical_msb;

		i_slice_vertical_msb = ( i_slice_row >> 7 ) & 3;

		y262_bitstream_write( ps_bitstream, STARTCODE_SLICE_START + i_slice_row & 0x7f, 8 );
		y262_bitstream_write( ps_bitstream, i_slice_vertical_msb, 3 );
	}
	else
	{
		y262_bitstream_write( ps_bitstream, STARTCODE_SLICE_START + i_slice_row, 8 );
	}

	y262_bitstream_write( ps_bitstream, ps_slice->s_slice_header.i_quantizer_scale_code, 5 );
	y262_bitstream_write( ps_bitstream, ps_slice->s_slice_header.b_intra_slice_flag, 1 );

	if( ps_slice->s_slice_header.b_intra_slice_flag )
	{
		y262_bitstream_write( ps_bitstream, ps_slice->s_slice_header.b_intra_slice, 1 );
		y262_bitstream_write( ps_bitstream, 0x7f, 7 );
		y262_bitstream_write( ps_bitstream, 0, 1 ); /* no more extra slice data */
	}

	y262_encode_slice( ps_y262, ps_slice );
}


int32_t y262_encode_unit( y262_t *ps_y262, int32_t i_unit_startcode, int32_t i_unit_extension_startcode, int32_t i_picture_type, int32_t i_slice_row )
{
	y262_bitstream_reset( &ps_y262->s_bitstream );
	if( i_unit_startcode == STARTCODE_SEQUENCE_HEADER )
	{
		y262_write_sequence_header( ps_y262 );
	}
	else if( i_unit_startcode == STARTCODE_EXTENSION && i_unit_extension_startcode == H262_SEQUENCE_EXTENSION_ID )
	{
		y262_write_sequence_extension( ps_y262 );
	}
	else if( i_unit_startcode == STARTCODE_EXTENSION && i_unit_extension_startcode == H262_SEQUENCE_DISPLAY_EXTENSION_ID )
	{
		y262_write_sequence_display_extension( ps_y262 );
	}
	else if( i_unit_startcode == STARTCODE_GROUP )
	{
		y262_write_group_of_pictures_header( ps_y262 );
	}
	else if( i_unit_startcode == STARTCODE_PICTURE )
	{
		y262_write_picture_header( ps_y262, i_picture_type );
	}
	else if( i_unit_startcode == STARTCODE_EXTENSION && i_unit_extension_startcode == H262_PICTURE_CODING_EXTENSION_ID )
	{
		y262_write_picture_coding_extension( ps_y262 );
	}
	else if( i_unit_startcode == STARTCODE_SLICE_START )
	{
		/* deprecated */
	}
	else if( i_unit_startcode == STARTCODE_USER_DATA )
	{
		y262_write_user_data( ps_y262, i_slice_row );
	}
	else if( i_unit_startcode == STARTCODE_STUFFING )
	{
		y262_write_zero_stuffing( ps_y262, i_slice_row );
	}

	{
		uint8_t *pui8_bs;
		uint32_t ui_len;
		y262_result_t s_res;

		y262_bitstream_flush( &ps_y262->s_bitstream, &pui8_bs, &ui_len );

		if( ps_y262->s_funcs.pf_result_callback )
		{
			s_res.bitstream_unit.i_type = Y262_RESULT_BITSTREAM;
			s_res.bitstream_unit.i_don = ps_y262->ps_input_picture->i_don;
			s_res.bitstream_unit.i_pon = ps_y262->ps_input_picture->i_pon;
			s_res.bitstream_unit.pui8_unit = pui8_bs;
			s_res.bitstream_unit.i_unit_length = ui_len;
			s_res.bitstream_unit.i_unit_type = i_unit_startcode;
			ps_y262->s_funcs.pf_result_callback( ps_y262->p_cb_handle, Y262_RESULT_BITSTREAM, &s_res );
		}
		return ui_len * 8;
	}
}

/* called by slice thread or directly */
void y262_encode_unit_slice( y262_t *ps_y262, int32_t i_picture_type, int32_t i_slice_row )
{
	y262_slice_t s_slice;
	y262_write_slice( ps_y262, i_picture_type, i_slice_row, &s_slice );
}

void y262_start_and_encode_unit_slices( y262_t *ps_y262, int32_t i_slice_encoder_idx, int32_t i_picture_type, int32_t i_start_mb_row, int32_t i_end_mb_row )
{
	int32_t i_idx, i_slices_start_mb, i_slices_end_mb;

	i_slices_start_mb = i_start_mb_row * ( ps_y262->i_sequence_width >> 4 );
	i_slices_end_mb = ( ( i_end_mb_row + 1 ) * ( ps_y262->i_sequence_width >> 4 ) ) - 1;

	y262_ratectrl_start_slice_encoder( ps_y262, &ps_y262->rgps_slice_encoders[ i_slice_encoder_idx ]->s_slice_encoder_ratectrl, i_slices_start_mb, i_slices_end_mb );

	if( !ps_y262->b_multithreading )
	{
		y262_bitstream_reset( &ps_y262->rgps_slice_encoders[ i_slice_encoder_idx ]->s_bitstream );
		for( i_idx = i_start_mb_row; i_idx <= i_end_mb_row; i_idx++ )
		{
			y262_encode_unit_slice( ps_y262->rgps_slice_encoders[ i_slice_encoder_idx ], i_picture_type, i_idx );
			y262_bitstream_bytealign( &ps_y262->rgps_slice_encoders[ i_slice_encoder_idx ]->s_bitstream );
		}
	}
	else
	{
		y262_slice_thread_t *ps_slice_thread;

		ps_slice_thread = &ps_y262->rgs_slice_threads[ i_slice_encoder_idx ];
		ps_slice_thread->i_command = Y262_SLICE_THREAD_CMD_ENCODE;
		ps_slice_thread->i_picture_type = i_picture_type;
		ps_slice_thread->i_first_slice_row = i_start_mb_row;
		ps_slice_thread->i_last_slice_row = i_end_mb_row;

		y262_event_set_g( ps_y262, ps_slice_thread->p_start_event );
	}
}


int32_t y262_finish_encode_unit_slices( y262_t *ps_y262, int32_t i_slice_encoder_idx )
{
	y262_t *ps_y262_i;
	uint8_t *pui8_bs;
	uint32_t ui_len;

	if( ps_y262->b_multithreading )
	{
		y262_slice_thread_t *ps_slice_thread;
		ps_slice_thread = &ps_y262->rgs_slice_threads[ i_slice_encoder_idx ];
		y262_event_wait_g( ps_y262, ps_slice_thread->p_finished_event );
	}

	ps_y262_i = ps_y262->rgps_slice_encoders[ i_slice_encoder_idx ];

	y262_ratectrl_end_slice_encoder( ps_y262, &ps_y262_i->s_slice_encoder_ratectrl );

	y262_bitstream_flush( &ps_y262_i->s_bitstream, &pui8_bs, &ui_len );

	return ui_len * 8;
}


int32_t y262_encode_slices( y262_t *ps_y262, int32_t i_picture_type )
{
	int32_t i_idx, i_num_mb_rows, i_num_used_slice_encoders, i_enc_start_row, i_enc_end_row;
	y262_bitstream_t s_save_bs;
	int32_t i_accumulated_bits;

	if( ps_y262->b_multithreading )
	{
		y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex ); /* need to lock this to shut up helgrind */
	}
	for( i_idx = 0; i_idx < ps_y262->i_num_slice_encoders; i_idx++ )
	{
		y262_t *ps_y262_i;
		ps_y262_i = ps_y262->rgps_slice_encoders[ i_idx ];
		s_save_bs = ps_y262_i->s_bitstream;
		*ps_y262_i = *ps_y262;
		ps_y262_i->s_bitstream = s_save_bs;
	}
	if( ps_y262->b_multithreading )
	{
		y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex ); /* need to lock this to shut up helgrind */
	}

	i_accumulated_bits = 0;
	i_num_mb_rows = ( ps_y262->i_sequence_height >> 4 );
	i_num_used_slice_encoders = MIN( ps_y262->i_num_slice_encoders, i_num_mb_rows );
	for( i_idx = 0; i_idx < i_num_used_slice_encoders; i_idx++ )
	{
		i_enc_start_row = ( i_num_mb_rows * i_idx ) / i_num_used_slice_encoders;
		i_enc_end_row = ( ( i_num_mb_rows * ( i_idx + 1 ) ) / i_num_used_slice_encoders ) - 1;
		y262_start_and_encode_unit_slices( ps_y262, i_idx, i_picture_type, i_enc_start_row, i_enc_end_row );
	}
	for( i_idx = 0; i_idx < i_num_used_slice_encoders; i_idx++ )
	{
		i_accumulated_bits += y262_finish_encode_unit_slices( ps_y262, i_idx );
	}
	return i_accumulated_bits;
}


int32_t y262_output_slices( y262_t *ps_y262 )
{
	y262_t *ps_y262_i;
	int32_t i_num_mb_rows, i_idx, i_num_used_slice_encoders, i_accumulated_bits;
	uint8_t *pui8_bs;
	uint32_t ui_len;
	y262_result_t s_res;

	i_num_mb_rows = ( ps_y262->i_sequence_height >> 4 );
	i_num_used_slice_encoders = MIN( ps_y262->i_num_slice_encoders, i_num_mb_rows );

	i_accumulated_bits = 0;
	for( i_idx = 0; i_idx < i_num_used_slice_encoders; i_idx++ )
	{
		ps_y262_i = ps_y262->rgps_slice_encoders[ i_idx ];

		y262_bitstream_get( &ps_y262_i->s_bitstream, &pui8_bs, &ui_len );

		if( ps_y262->s_funcs.pf_result_callback )
		{
			s_res.bitstream_unit.i_type = Y262_RESULT_BITSTREAM;
			s_res.bitstream_unit.i_don = ps_y262->ps_input_picture->i_don;
			s_res.bitstream_unit.i_pon = ps_y262->ps_input_picture->i_pon;
			s_res.bitstream_unit.pui8_unit = pui8_bs;
			s_res.bitstream_unit.i_unit_length = ui_len;
			s_res.bitstream_unit.i_unit_type = STARTCODE_SLICE_START;
			ps_y262->s_funcs.pf_result_callback( ps_y262->p_cb_handle, Y262_RESULT_BITSTREAM, &s_res );
		}

		i_accumulated_bits += ui_len * 8;
	}
	return i_accumulated_bits;
}


void y262_get_frame_psnr( y262_t *ps_y262, y262_picture_t *ps_original, y262_reference_picture_t *ps_recon, double *pd_psnr )
{
	int32_t i_x, i_y, i_num_pel_x, i_num_pel_y, i_pel_stride, i_idx;
	int64_t ui64_ssd;
	uint8_t *pui8_ref, *pui8_recon;

	for( i_idx = 0; i_idx < 3; i_idx++ )
	{
		if( i_idx == 0 )
		{
			pui8_ref = ps_original->pui8_luma;
			pui8_recon = ps_recon->pui8_luma;
			i_num_pel_x = ps_y262->i_sequence_width;
			i_num_pel_y = ps_y262->i_sequence_height;
			i_pel_stride = ps_y262->i_sequence_width;
		}
		else
		{
			if( i_idx == 1 )
			{
				pui8_ref = ps_original->pui8_cb;
				pui8_recon = ps_recon->pui8_cb;
			}
			else
			{
				pui8_ref = ps_original->pui8_cr;
				pui8_recon = ps_recon->pui8_cr;
			}
			i_num_pel_x = ps_y262->i_sequence_width / 2;
			i_num_pel_y = ps_y262->i_sequence_height / 2;
			i_pel_stride = ps_y262->i_sequence_width / 2;
		}
		ui64_ssd = 0;
		for( i_y = 0; i_y < i_num_pel_y; i_y += 8 )
		{
			for( i_x = 0; i_x < i_num_pel_x; i_x += 8 )
			{
				ui64_ssd += ps_y262->s_funcs.f_ssd_8x8( pui8_ref + i_x, i_pel_stride, pui8_recon + i_x, i_pel_stride );
			}
			pui8_ref += i_pel_stride * 8;
			pui8_recon += i_pel_stride * 8;
		}

	    if( 0 == ui64_ssd )
	    {
			pd_psnr[ i_idx ] = 100.0;
		}
		else 
		{
	        pd_psnr[ i_idx ] = 10 * log10( 255.0 * 255.0 * ( i_num_pel_x * i_num_pel_y ) / ( double )( ui64_ssd ) );
	    }
	}
}


int32_t y262_encode_possible_user_data_unit( y262_t *ps_y262, int32_t i_where )
{
	int32_t i_idx, i_accumulated_bits;

	i_accumulated_bits = 0;

	for( i_idx = 0; i_idx < ps_y262->ps_input_picture->i_num_user_data; i_idx++ )
	{
		if( i_where == ps_y262->ps_input_picture->rgps_user_data[ i_idx ]->i_where )
		{
			i_accumulated_bits += y262_encode_unit( ps_y262, STARTCODE_USER_DATA, 0, 0, i_idx );
		}
	}

	return i_accumulated_bits;
}


void y262_encode_picture( y262_t *ps_y262, y262_picture_t *ps_picture, int32_t i_picture_type, int32_t i_pon )
{
	int32_t i_idx, i_accumulated_bits, i_number_of_encodes, i_stuffing_bits;
	bool_t b_reencode_if_possible;

	ps_y262->ps_input_picture = ps_picture;

	i_accumulated_bits = 0;

	if( i_picture_type == PICTURE_CODING_TYPE_I )
	{
		i_accumulated_bits += y262_encode_unit( ps_y262, STARTCODE_SEQUENCE_HEADER, 0, 0, 0 );
		if( !ps_y262->b_sequence_mpeg1 )
		{
			i_accumulated_bits += y262_encode_unit( ps_y262, STARTCODE_EXTENSION, H262_SEQUENCE_EXTENSION_ID, 0, 0 );
			i_accumulated_bits += y262_encode_unit( ps_y262, STARTCODE_EXTENSION, H262_SEQUENCE_DISPLAY_EXTENSION_ID, 0, 0 );
		}
		i_accumulated_bits += y262_encode_possible_user_data_unit( ps_y262, Y262_USER_DATA_AFTER_SEQUENCE );
		i_accumulated_bits += y262_encode_unit( ps_y262, STARTCODE_GROUP, 0, 0, 0 );
		i_accumulated_bits += y262_encode_possible_user_data_unit( ps_y262, Y262_USER_DATA_AFTER_GOP );
	}

	if( i_picture_type != PICTURE_CODING_TYPE_B )
	{
		ps_y262->b_next_reference_picture_toggle = !ps_y262->b_next_reference_picture_toggle;
		ps_y262->ps_refpic_forward = &ps_y262->rgs_frame_buffer[ !ps_y262->b_next_reference_picture_toggle ];
		ps_y262->ps_refpic_dst = &ps_y262->rgs_frame_buffer[ ps_y262->b_next_reference_picture_toggle ];
	}
	else
	{
		ps_y262->ps_refpic_forward = &ps_y262->rgs_frame_buffer[ !ps_y262->b_next_reference_picture_toggle ];
		ps_y262->ps_refpic_backward = &ps_y262->rgs_frame_buffer[ ps_y262->b_next_reference_picture_toggle ];
		ps_y262->ps_refpic_dst = &ps_y262->rgs_frame_buffer[ 2 ];
	}
	y262_lookahead_mbtree( ps_y262, ps_picture );
	y262_lookahead_fill_ratectrl_vars( ps_y262, ps_picture );
	y262_ratectrl_start_picture( ps_y262, i_accumulated_bits );

	i_accumulated_bits += y262_encode_unit( ps_y262, STARTCODE_PICTURE, 0, i_picture_type, 0 );
	if( !ps_y262->b_sequence_mpeg1 )
	{
		i_accumulated_bits += y262_encode_unit( ps_y262, STARTCODE_EXTENSION, H262_PICTURE_CODING_EXTENSION_ID, PICTURE_CODING_TYPE_I, 0 );
	}
	i_accumulated_bits += y262_encode_possible_user_data_unit( ps_y262, Y262_USER_DATA_BEFORE_SLICES );


	if( i_pon == 1 )
	{
		i_pon = i_pon;
	}

	i_number_of_encodes = 0;
	do
	{
		int32_t i_picture_bits;

		i_picture_bits = i_accumulated_bits;
		i_picture_bits += y262_encode_slices( ps_y262, i_picture_type );

		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			/*printf("%d: %d\n", i_idx - 32, rgi_quantizer_delta[ i_idx ] );*/
		}

		b_reencode_if_possible = FALSE;
		if( i_number_of_encodes < 3 )
		{
			b_reencode_if_possible = y262_ratectrl_check_for_reencode( ps_y262, i_picture_bits );
			if( b_reencode_if_possible )
			{
				/*fprintf( stderr, "reenc\n" );*/
			}
		}

		i_number_of_encodes++;
	} while( b_reencode_if_possible );

	i_accumulated_bits += y262_output_slices( ps_y262 );

	y262_ratectrl_end_picture( ps_y262, i_accumulated_bits );

	if( ps_y262->b_sequence_cbr )
	{
		int32_t i_commit_bits;
		i_stuffing_bits = y262_ratectrl_stuffing_bits_needed( ps_y262 );
		if( i_stuffing_bits > 0 )
		{
			i_commit_bits = y262_encode_unit( ps_y262, STARTCODE_STUFFING, 0, i_picture_type, ( i_stuffing_bits + 7 ) / 8 );
			y262_ratectrl_commit_stuffing_bits( ps_y262, i_commit_bits );
		}
	}

	if( ps_y262->s_funcs.pf_result_callback )
	{
		y262_result_t s_res;

		s_res.recon.i_type = Y262_RESULT_RECON;
		s_res.recon.i_pon = ps_y262->ps_input_picture->i_pon;
		s_res.recon.i_don = ps_y262->ps_input_picture->i_don;
		y262_get_frame_psnr( ps_y262, ps_picture, ps_y262->ps_refpic_dst, &s_res.recon.f64_psnr[ 0 ] );
		s_res.recon.pui8_luma = ps_y262->ps_refpic_dst->pui8_luma;
		s_res.recon.pui8_cb = ps_y262->ps_refpic_dst->pui8_cb;
		s_res.recon.pui8_cr = ps_y262->ps_refpic_dst->pui8_cr;
		s_res.recon.i_frame_type = i_picture_type;

		ps_y262->s_funcs.pf_result_callback( ps_y262->p_cb_handle, Y262_RESULT_RECON, &s_res );
	}
}



