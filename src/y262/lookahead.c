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

int32_t y262_get_free_input_frame_idx( y262_t *ps_y262 )
{
	int32_t i_idx;

	if( ps_y262->b_multithreading )
	{
		y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex );
	}

	for( i_idx = 0; i_idx < ps_y262->i_max_buffered_input_pictures; i_idx++ )
	{
		if( ps_y262->rgs_buffered_input_pictures[ i_idx ].b_used == FALSE )
		{
			if( ps_y262->b_multithreading )
			{
				y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
			}
			return i_idx;
		}
	}

	if( ps_y262->b_multithreading )
	{
		y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
	}

	assert( FALSE );
	return 0;
}

int32_t y262_get_input_frame_pon( y262_t *ps_y262, int32_t i_pon )
{
	int32_t i_idx;

	if( ps_y262->b_multithreading )
	{
		y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex );
	}

	for( i_idx = 0; i_idx < ps_y262->i_max_buffered_input_pictures; i_idx++ )
	{
		if( ps_y262->rgs_buffered_input_pictures[ i_idx ].b_used == TRUE &&
			ps_y262->rgs_buffered_input_pictures[ i_idx ].i_pon == i_pon )
		{
			if( ps_y262->b_multithreading )
			{
				y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
			}
			return i_idx;
		}
	}

	if( ps_y262->b_multithreading )
	{
		y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
	}

	assert( FALSE );
	return 0;
}

int32_t y262_get_input_frame_don( y262_t *ps_y262, int32_t i_don )
{
	int32_t i_idx;

	if( ps_y262->b_multithreading )
	{
		y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex );
	}

	for( i_idx = 0; i_idx < ps_y262->i_max_buffered_input_pictures; i_idx++ )
	{
		if( ps_y262->rgs_buffered_input_pictures[ i_idx ].b_used == TRUE &&
			ps_y262->rgs_buffered_input_pictures[ i_idx ].i_don == i_don )
		{
			if( ps_y262->b_multithreading )
			{
				y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
			}
			return i_idx;
		}
	}

	if( ps_y262->b_multithreading )
	{
		y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
	}

	assert( FALSE );
	return 0;
}


void y262_lookahead_analyze_mb_intra( y262_t *ps_y262, y262_lookahead_mb_t *ps_mb, int32_t i_mb_x, int32_t i_mb_y, y262_picture_t *ps_pic )
{
	int32_t i_satd, i_sad;
	const uint8_t rgui8_zero[ 16 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t *pui8_mb;

	pui8_mb = ps_pic->pui8_luma + ( i_mb_x << 4 ) + ( ( i_mb_y << 4 ) * ps_y262->i_sequence_width );
	i_satd = ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x16 ]( pui8_mb, ps_y262->i_sequence_width, ( uint8_t *)rgui8_zero, 0 );
	i_sad = ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x16 ]( ( uint8_t *)rgui8_zero, 0, pui8_mb, ps_y262->i_sequence_width );
	i_satd -= i_sad >> 2; /* - dc */

	i_satd = MAX( i_satd + 10, 10 );

	ps_mb->i_best_cost = ps_mb->i_intra_cost = i_satd;
	ps_mb->i_best_mode = LOOKAHEAD_MODE_INTRA;
}

void y262_lookahead_analyze_mb_inter( y262_t *ps_y262, y262_lookahead_mb_t *ps_mb, int32_t i_mb_x, int32_t i_mb_y, int32_t i_s, y262_picture_t *ps_pic, y262_picture_t *ps_ref )
{
	int32_t i_fcode_x, i_fcode_y;
	y262_me_context_t s_me;

	i_fcode_x = ps_y262->rgi_fcode[ i_s ][ 0 ];
	i_fcode_y = ps_y262->rgi_fcode[ i_s ][ 1 ];
	s_me.pui8_blk = ps_pic->pui8_luma + ( i_mb_x << 4 ) + ( ( i_mb_y << 4 ) * ps_y262->i_sequence_width );
	s_me.i_blk_stride = ps_y262->i_sequence_width;
	s_me.i_blk_type = BLOCK_TYPE_16x16;
	s_me.i_min_mv_x = -( 1 << ( 3 + i_fcode_x - 1 ) );
	s_me.i_min_mv_y = -( 1 << ( 3 + i_fcode_y - 1 ) );
	s_me.i_max_mv_x =  ( 1 << ( 3 + i_fcode_x - 1 ) ) - 1;
	s_me.i_max_mv_y =  ( 1 << ( 3 + i_fcode_y - 1 ) ) - 1;
	s_me.i_x_offset = i_mb_x << 4;
	s_me.i_y_offset = i_mb_y << 4;
	s_me.i_num_candidates_fp = 0;
	s_me.i_lambda = 1;
	s_me.i_ref_width = ps_y262->i_sequence_width;
	s_me.i_ref_height = ps_y262->i_sequence_height;
	s_me.i_ref_stride = ps_y262->i_sequence_width;
	s_me.pui8_ref = ps_ref->pui8_luma;
	if( i_mb_x > 0 )
	{
		s_me.i_pred_mv_x = ps_mb[ -1 ].rgi_mvs[ i_s ][ 0 ];
		s_me.i_pred_mv_y = ps_mb[ -1 ].rgi_mvs[ i_s ][ 1 ];
	}
	else
	{
		s_me.i_pred_mv_x = 0;
		s_me.i_pred_mv_y = 0;
	}
	s_me.i_me_call = MECALL_LOOKAHEAD;
	y262_motion_search( ps_y262, &s_me );

	ps_mb->rgi_mvs[ i_s ][ 0 ] = s_me.i_best_mv_x;
	ps_mb->rgi_mvs[ i_s ][ 1 ] = s_me.i_best_mv_y;
	ps_mb->rgi_mv_costs[ i_s ] = s_me.i_best_mv_sad + 4;

	if( ps_mb->rgi_mv_costs[ i_s ] < ps_mb->i_best_cost )
	{
		ps_mb->i_best_cost = ps_mb->rgi_mv_costs[ i_s ];
		if( i_s == 0 )
		{
			ps_mb->i_best_mode = LOOKAHEAD_MODE_INTER_FW;
		}
		else
		{
			ps_mb->i_best_mode = LOOKAHEAD_MODE_INTER_BW;
		}
	}
}

void y262_lookahead_analyze_slice( y262_t *ps_y262, y262_picture_t *ps_pic, y262_picture_t *ps_fw_ref, y262_picture_t *ps_bw_ref, int32_t i_mb_y_first, int32_t i_mb_y_last )
{
	int32_t i_lookahead_size_x, i_lookahead_size_y, i_mb_x, i_mb_y, i_mb_idx;
	y262_lookahead_mb_t *ps_mb;

	i_lookahead_size_x = ps_y262->i_sequence_width >> 4;
	i_lookahead_size_y = ps_y262->i_sequence_height >> 4;

	for( i_mb_y = i_mb_y_first; i_mb_y <= i_mb_y_last; i_mb_y++ )
	{
		for( i_mb_x = 0; i_mb_x < i_lookahead_size_x; i_mb_x++ )
		{
			int32_t i_variance_frame;
			i_mb_idx = i_mb_x + ( i_lookahead_size_x * i_mb_y );

			ps_mb = &ps_pic->ps_lookahead[ i_mb_idx ];
			ps_mb->i_quantizer_scale = ( 1 << 12 );

			if( ps_y262->b_variance_aq )
			{
				i_variance_frame = ps_y262->s_funcs.f_variance_16x16( ps_pic->pui8_luma + ( i_mb_x << 4 ) + ( ( i_mb_y << 4 ) * ps_y262->i_sequence_width ), ps_y262->i_sequence_width );
				i_variance_frame += ps_y262->s_funcs.f_variance_8x8( ps_pic->pui8_cb + ( i_mb_x << 3 ) + ( ( i_mb_y << 3 ) * ( ps_y262->i_sequence_width >> 1 ) ), ps_y262->i_sequence_width >> 1 );
				i_variance_frame += ps_y262->s_funcs.f_variance_8x8( ps_pic->pui8_cr + ( i_mb_x << 3 ) + ( ( i_mb_y << 3 ) * ( ps_y262->i_sequence_width >> 1 ) ), ps_y262->i_sequence_width >> 1 );
				ps_mb->i_quantizer_aq_scale = MAX( 1 << 10,( int32_t )( ( pow( i_variance_frame, 1.0/6.0 ) / 4.0 ) * ( 1 << 12 ) ) );
			}
			else
			{
				ps_mb->i_quantizer_aq_scale = ( 1 << 12 );
			}

			y262_lookahead_analyze_mb_intra( ps_y262, ps_mb, i_mb_x, i_mb_y, ps_pic );
			if( ps_fw_ref )
			{
				y262_lookahead_analyze_mb_inter( ps_y262, ps_mb, i_mb_x, i_mb_y, 0, ps_pic, ps_fw_ref );
			}
			if( ps_bw_ref )
			{
				y262_lookahead_analyze_mb_inter( ps_y262, ps_mb, i_mb_x, i_mb_y, 1, ps_pic, ps_bw_ref );
			}
			/* fixme: bi ? */
		}
	}
}

void y262_lookahead_analyze_frame( y262_t *ps_y262, y262_picture_t *ps_pic, y262_picture_t *ps_fw_ref, y262_picture_t *ps_bw_ref )
{
	int32_t i_lookahead_size_x, i_lookahead_size_y, i_mb_x, i_mb_y, i_mb_idx, i_frame_cost, i_frame_intra_cost, i_slice_encoder;
	y262_lookahead_mb_t *ps_mb;

	i_lookahead_size_x = ps_y262->i_sequence_width >> 4;
	i_lookahead_size_y = ps_y262->i_sequence_height >> 4;

	if( ps_fw_ref )
	{
		ps_pic->i_forward_pon = ps_fw_ref->i_pon;
	}
	else
	{
		ps_pic->i_forward_pon = -1;
	}
	if( ps_bw_ref )
	{
		ps_pic->i_backward_pon = ps_bw_ref->i_pon;
	}
	else
	{
		ps_pic->i_backward_pon = -1;
	}

	i_frame_cost = 0;
	i_frame_intra_cost = 0;

	if( ps_y262->b_multithreading )
	{
		y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex ); /* need to lock this to shut up helgrind */
		for( i_slice_encoder = 0; i_slice_encoder < ps_y262->i_num_lookahead_encoders; i_slice_encoder++ )
		{
			ps_y262->rgs_lookahead_threads[ i_slice_encoder ].i_command = Y262_SLICE_THREAD_CMD_LOOKAHEAD;
			ps_y262->rgs_lookahead_threads[ i_slice_encoder ].i_first_slice_row = ( i_lookahead_size_y * i_slice_encoder ) / ps_y262->i_num_lookahead_encoders;
			ps_y262->rgs_lookahead_threads[ i_slice_encoder ].i_last_slice_row = ( ( i_lookahead_size_y * ( i_slice_encoder + 1 ) ) / ps_y262->i_num_lookahead_encoders ) - 1;
			ps_y262->rgs_lookahead_threads[ i_slice_encoder ].ps_pic = ps_pic;
			ps_y262->rgs_lookahead_threads[ i_slice_encoder ].ps_fw_ref = ps_fw_ref;
			ps_y262->rgs_lookahead_threads[ i_slice_encoder ].ps_bw_ref = ps_bw_ref;
		}
		y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );

		for( i_slice_encoder = 0; i_slice_encoder < ps_y262->i_num_lookahead_encoders; i_slice_encoder++ )
		{
			y262_event_set_g( ps_y262, ps_y262->rgs_lookahead_threads[ i_slice_encoder ].p_start_event );
		}
		for( i_slice_encoder = 0; i_slice_encoder < ps_y262->i_num_lookahead_encoders; i_slice_encoder++ )
		{
			y262_event_wait_g( ps_y262, ps_y262->rgs_lookahead_threads[ i_slice_encoder ].p_finished_event );
		}
	}
	else
	{
		for( i_slice_encoder = 0; i_slice_encoder < ps_y262->i_num_lookahead_encoders; i_slice_encoder++ )
		{
			y262_lookahead_analyze_slice( ps_y262, ps_pic, ps_fw_ref, ps_bw_ref, ( i_lookahead_size_y * i_slice_encoder ) /
				ps_y262->i_num_lookahead_encoders, ( ( i_lookahead_size_y * ( i_slice_encoder + 1 ) ) / ps_y262->i_num_lookahead_encoders ) - 1 );
		}
	}

	for( i_mb_y = 0; i_mb_y < i_lookahead_size_y; i_mb_y++ )
	{
		for( i_mb_x = 0; i_mb_x < i_lookahead_size_x; i_mb_x++ )
		{
			i_mb_idx = i_mb_x + ( i_lookahead_size_x * i_mb_y );
			ps_mb = &ps_pic->ps_lookahead[ i_mb_idx ];

			i_frame_intra_cost += ps_mb->i_intra_cost;
			if( ps_pic->i_frame_type == PICTURE_CODING_TYPE_I )
			{
				i_frame_cost += ps_mb->i_intra_cost; /* need to special case now that its doing mbtree past i frames */
			}
			else
			{
				i_frame_cost += ps_mb->i_best_cost;
			}
		}
	}

	if( ps_y262->b_multithreading )
	{
		y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex ); /* need to lock this to shut up helgrind */
	}
	ps_pic->i_frame_intra_cost = i_frame_intra_cost;
	ps_pic->i_frame_cost = i_frame_cost;
	if( ps_y262->b_multithreading )
	{
		y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
	}
}

void y262_process_lookahead_internal( y262_t *ps_y262 )
{
	int32_t i_idx, i_next_ref, i_next_pon, i_forward_ref_pon, i_backward_ref_pon, i_fw_ref_idx, i_bw_ref_idx;
	y262_picture_t *ps_pic, *ps_fw_ref, *ps_bw_ref;
	bool_t b_backward_pred_only = FALSE;

	i_next_pon = ps_y262->i_lookahead_next_pon;
	i_next_ref = ps_y262->i_lookahead_next_ref;

	ps_pic = &ps_y262->rgs_buffered_input_pictures[ y262_get_input_frame_pon( ps_y262, i_next_ref ) ];
	ps_pic->i_don = ps_y262->i_leading_lookahead_don++;
	if( ps_y262->i_keyframe_countdown <= 0 )
	{
		ps_pic->i_frame_type = PICTURE_CODING_TYPE_I;
		ps_y262->i_keyframe_countdown = ps_y262->i_sequence_keyframe_distance;
		b_backward_pred_only = ps_y262->b_closed_gop;
	}
	else
	{
		ps_pic->i_frame_type = PICTURE_CODING_TYPE_P;
		ps_y262->i_keyframe_countdown--;
	}
	ps_pic->b_backward_pred_only = FALSE;

	if( ps_pic->i_frame_type == PICTURE_CODING_TYPE_I )
	{
		ps_y262->i_last_keyframe_temporal_reference = i_next_pon;
	}

	ps_pic->i_temporal_reference = ps_pic->i_pon - ps_y262->i_last_keyframe_temporal_reference;
	if( ps_pic->i_temporal_reference < 0 )
	{
		int32_t *pi_null = NULL;
		*pi_null = 0;
	}
	ps_pic->i_temporal_reference = MAX( 0, ps_pic->i_temporal_reference ); /* if stream ends in a keyframe i_next_pon points past end behind the keyframe */

	i_forward_ref_pon = ps_pic->i_don - 1;
	i_backward_ref_pon = i_next_ref;

	if( i_forward_ref_pon >= 0 )
	{
		i_fw_ref_idx = y262_get_input_frame_pon( ps_y262, i_forward_ref_pon );
		assert( i_fw_ref_idx >= 0 );
		ps_fw_ref = &ps_y262->rgs_buffered_input_pictures[ i_fw_ref_idx ];
	}
	else
	{
		ps_fw_ref = NULL;
	}
	i_bw_ref_idx = y262_get_input_frame_pon( ps_y262, i_backward_ref_pon );
	assert( i_bw_ref_idx >= 0 );
	ps_bw_ref = &ps_y262->rgs_buffered_input_pictures[ i_bw_ref_idx ];

	y262_lookahead_analyze_frame( ps_y262, ps_pic, /*ps_pic->i_frame_type == PICTURE_CODING_TYPE_I ? NULL : */ ps_fw_ref, NULL );

	for( i_idx = i_next_pon; i_idx < i_next_ref; i_idx++ )
	{
		ps_pic = &ps_y262->rgs_buffered_input_pictures[ y262_get_input_frame_pon( ps_y262, i_idx ) ];
		ps_pic->i_don = ps_y262->i_leading_lookahead_don++;
		ps_pic->i_frame_type = PICTURE_CODING_TYPE_B;
		ps_pic->i_temporal_reference = ps_pic->i_pon - ps_y262->i_last_keyframe_temporal_reference;
		ps_pic->b_backward_pred_only = b_backward_pred_only;
		assert( ps_pic->i_temporal_reference >= 0 );

		if( !b_backward_pred_only )
		{
			y262_lookahead_analyze_frame( ps_y262, ps_pic, ps_fw_ref, ps_bw_ref );
		}
		else
		{
			y262_lookahead_analyze_frame( ps_y262, ps_pic, NULL, ps_bw_ref );
		}
	}
}

void y262_start_lookahead( y262_t *ps_y262 )
{
	if( ps_y262->b_multithreading )
	{
		assert( !ps_y262->b_lookahead_running );
		ps_y262->s_lookahead_thread.i_command = Y262_LOOKAHEAD_THREAD_CMD_LOOKAHEAD;
		y262_event_set_g( ps_y262, ps_y262->s_lookahead_thread.p_start_event );
	}
	else
	{
		y262_process_lookahead_internal( ps_y262 );
	}
}

void y262_setup_lookahead_next_and_start_lookahead( y262_t *ps_y262 )
{
	int32_t i_pon, i_buf_idx;

	ps_y262->i_lookahead_next_pon = ps_y262->i_leading_lookahead_don;
	ps_y262->i_lookahead_next_ref = ps_y262->i_current_input_pon - 1;

	for( i_pon = ps_y262->i_lookahead_next_pon; i_pon <= ps_y262->i_lookahead_next_ref; i_pon++ )
	{
		i_buf_idx = y262_get_input_frame_pon( ps_y262, i_pon );
		if( ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].b_force_new_gop )
		{
			ps_y262->i_lookahead_next_ref = i_pon;
			ps_y262->i_keyframe_countdown = 0;
		}
	}

	y262_start_lookahead( ps_y262 );
}



void y262_finish_lookahead( y262_t *ps_y262 )
{
	if( ps_y262->b_multithreading )
	{
		assert( ps_y262->b_lookahead_running );
		y262_event_wait_g( ps_y262, ps_y262->s_lookahead_thread.p_finished_event );
	}
}


void y262_lookahead_mbtree( y262_t *ps_y262, y262_picture_t *ps_pic )
{
	volatile int32_t i_ridx;
	int32_t i_idx, i_lookahead_size_x, i_lookahead_size_y, i_mb_x, i_mb_y, i_mb_idx, i_don;
	y262_lookahead_mb_t *ps_mb;
	int32_t *rgpi_references[ 4 ], rgi_references_don_map[ 4 ];

	i_lookahead_size_x = ps_y262->i_sequence_width >> 4;
	i_lookahead_size_y = ps_y262->i_sequence_height >> 4;

	if( ps_pic->i_frame_type == PICTURE_CODING_TYPE_B )
	{
		for( i_idx = 0; i_idx < i_lookahead_size_x * i_lookahead_size_y; i_idx++ )
		{
			ps_mb = &ps_pic->ps_lookahead[ i_idx ];
			ps_mb->i_quantizer_scale = ( 1 << 12 );
		}
		return;
	}

	for( i_ridx = 0; i_ridx < 4; i_ridx++ )
	{
		rgpi_references[ i_ridx ] = ps_y262->rgpi_mbtree_references[ i_ridx ];
		rgi_references_don_map[ i_ridx ] = 0x7fffffff;
	}

	for( i_don = ps_pic->i_don + ps_y262->i_num_lookahead_pictures; i_don > ps_pic->i_don; i_don-- )
	{
		int32_t i_fpic_idx, i_fwpic_idx, i_bwpic_idx;
		y262_picture_t *ps_fpic;

		if( ps_y262->i_current_eof_don >= 0 && i_don >= ps_y262->i_current_eof_don )
		{
			continue;
		}

		i_fpic_idx = y262_get_input_frame_don( ps_y262, i_don );
		assert( i_fpic_idx >= 0 );
		ps_fpic = &ps_y262->rgs_buffered_input_pictures[ i_fpic_idx ];

		if( ps_fpic->i_frame_type != PICTURE_CODING_TYPE_B )
		{
			int32_t i_oldest;
			i_oldest = 0;
			for( i_idx = 0; i_idx < 4; i_idx++ )
			{
				if( rgi_references_don_map[ i_idx ] == i_don )
				{
					break;
				}
				else if( rgi_references_don_map[ i_idx ] > rgi_references_don_map[ i_oldest ] )
				{
					i_oldest = i_idx;
				}
			}
			if( i_idx == 4 )
			{
				rgi_references_don_map[ i_oldest ] = i_don;
				i_fpic_idx = i_oldest;
				memset( rgpi_references[ i_fpic_idx ], 0, sizeof( int32_t ) * i_lookahead_size_x * i_lookahead_size_y );
			}
			else
			{
				i_fpic_idx = i_idx;
			}
		}
		else
		{
			i_fpic_idx = -1;
		}

		if( ps_fpic->i_forward_pon >= 0 && ps_pic->i_pon <= ps_fpic->i_forward_pon )
		{
			int32_t i_oldest;
			i_fwpic_idx = y262_get_input_frame_pon( ps_y262, ps_fpic->i_forward_pon );
			assert( i_fwpic_idx >= 0 );

			i_oldest = 0;
			for( i_idx = 0; i_idx < 4; i_idx++ )
			{
				if( rgi_references_don_map[ i_idx ] == ps_y262->rgs_buffered_input_pictures[ i_fwpic_idx ].i_don )
				{
					break;
				}
				else if( rgi_references_don_map[ i_idx ] > rgi_references_don_map[ i_oldest ] )
				{
					i_oldest = i_idx;
				}
			}
			if( i_idx == 4 )
			{
				rgi_references_don_map[ i_oldest ] = ps_y262->rgs_buffered_input_pictures[ i_fwpic_idx ].i_don;
				i_fwpic_idx = i_oldest;
				memset( rgpi_references[ i_fwpic_idx ], 0, sizeof( int32_t ) * i_lookahead_size_x * i_lookahead_size_y );
			}
			else
			{
				i_fwpic_idx = i_idx;
			}
		}
		else
		{
			i_fwpic_idx = -1;
		}

		if( ps_fpic->i_backward_pon >= 0 )
		{
			int32_t i_oldest;
			i_bwpic_idx = y262_get_input_frame_pon( ps_y262, ps_fpic->i_backward_pon );
			assert( i_bwpic_idx >= 0 );

			i_oldest = 0;
			for( i_idx = 0; i_idx < 4; i_idx++ )
			{
				if( rgi_references_don_map[ i_idx ] == ps_y262->rgs_buffered_input_pictures[ i_bwpic_idx ].i_don )
				{
					break;
				}
				else if( rgi_references_don_map[ i_idx ] > rgi_references_don_map[ i_oldest ] )
				{
					i_oldest = i_idx;
				}
			}
			if( i_idx == 4 )
			{
				rgi_references_don_map[ i_oldest ] = ps_y262->rgs_buffered_input_pictures[ i_bwpic_idx ].i_don;
				i_bwpic_idx = i_oldest;
				memset( rgpi_references[ i_bwpic_idx ], 0, sizeof( int32_t ) * i_lookahead_size_x * i_lookahead_size_y );
			}
			else
			{
				i_bwpic_idx = i_idx;
			}
		}
		else
		{
			i_bwpic_idx = -1;
		}

		for( i_mb_y = 0; i_mb_y < i_lookahead_size_y; i_mb_y++ )
		{
			for( i_mb_x = 0; i_mb_x < i_lookahead_size_x; i_mb_x++ )
			{
				int64_t i_refweight, i_ref;
				const int32_t rgi_weights[ 4 ][ 2 ] = { { 32, 32 }, { 0, 32 }, { 32, 0 }, { 0, 0 } };
				const int32_t rgi_signs[ 4 ][ 2 ] = { { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } };
				const int32_t rgi_offsets[ 4 ][ 2 ] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
				int32_t *pi_reference;

				i_mb_idx = i_mb_x + ( i_lookahead_size_x * i_mb_y );
				ps_mb = &ps_fpic->ps_lookahead[ i_mb_idx ];

				i_refweight = ( ps_mb->i_intra_cost * ( ( int64_t )ps_mb->i_intra_cost - ps_mb->i_best_cost ) ) / ps_mb->i_intra_cost;
				if( i_fpic_idx >= 0 )
				{
					i_refweight += ( rgpi_references[ i_fpic_idx ][ i_mb_idx ] * ( ( int64_t )ps_mb->i_intra_cost - ps_mb->i_best_cost ) ) / ps_mb->i_intra_cost;	
				}

				for( i_ref = 0; i_ref < 2; i_ref++ )
				{
					if( i_ref == 0 && ps_mb->i_best_mode == LOOKAHEAD_MODE_INTER_FW && i_fwpic_idx >= 0 )
					{
						pi_reference = rgpi_references[ i_fwpic_idx ];
					}
					else if( i_ref == 1 && ps_mb->i_best_mode == LOOKAHEAD_MODE_INTER_BW && i_bwpic_idx >= 0 )
					{
						pi_reference = rgpi_references[ i_bwpic_idx ];
					}
					else
					{
						pi_reference = NULL;
					}
					if( pi_reference )
					{
						int32_t i_tmb_x, i_tmb_y, i_frac_x, i_frac_y, i_tmb_weight;

						i_tmb_x = ( i_mb_x << 5 ) + ps_mb->rgi_mvs[ i_ref ][ 0 ];
						i_tmb_y = ( i_mb_y << 5 ) + ps_mb->rgi_mvs[ i_ref ][ 1 ];
						i_frac_x = i_tmb_x & 0x1f;
						i_frac_y = i_tmb_y & 0x1f;
						i_tmb_x >>= 5;
						i_tmb_y >>= 5;

						for( i_idx = 0; i_idx < 4; i_idx++ )
						{
							int32_t i_ttmb_x, i_ttmb_y;
							i_tmb_weight = ( rgi_weights[ i_idx ][ 0 ] + rgi_signs[ i_idx ][ 0 ] * i_frac_x ) * ( rgi_weights[ i_idx ][ 1 ] + rgi_signs[ i_idx ][ 1 ] * i_frac_y );
							i_ttmb_x = i_tmb_x + rgi_offsets[ i_idx ][ 0 ];
							i_ttmb_y = i_tmb_y + rgi_offsets[ i_idx ][ 1 ];
							if( i_ttmb_x >= 0 && i_ttmb_x < i_lookahead_size_x &&
								i_ttmb_y >= 0 && i_ttmb_y < i_lookahead_size_y )
							{
								pi_reference[ i_ttmb_x + i_ttmb_y * i_lookahead_size_x ] += ( int32_t )( ( i_refweight * i_tmb_weight ) >> 10 );
							}
						}
					}
				}
			}
		}
	}

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		if( rgi_references_don_map[ i_idx ] == ps_pic->i_don )
		{
			break;
		}
	}
	if( i_idx == 4 )
	{
		/* fixme: most likely indicates that this is the last frame in sequence and is not referenced */
		for( i_idx = 0; i_idx < i_lookahead_size_x * i_lookahead_size_y; i_idx++ )
		{
			ps_mb = &ps_pic->ps_lookahead[ i_idx ];
			ps_mb->i_quantizer_scale = ( 1 << 12 );
		}
		return;
	}

	for( i_mb_y = 0; i_mb_y < i_lookahead_size_y; i_mb_y++ )
	{
		for( i_mb_x = 0; i_mb_x < i_lookahead_size_x; i_mb_x++ )
		{
			double d_quantizer_scale;
			i_mb_idx = i_mb_x + ( i_lookahead_size_x * i_mb_y );
			ps_mb = &ps_pic->ps_lookahead[ i_mb_idx ];

			d_quantizer_scale = ( ( double )( rgpi_references[ i_idx ][ i_mb_idx ] + ps_mb->i_intra_cost ) ) / ps_mb->i_intra_cost;
			d_quantizer_scale = 1.0 / ( ( ( sqrt( d_quantizer_scale ) - 1.0 ) * 0.43 ) + 1.0 );
			d_quantizer_scale = MAX( d_quantizer_scale, 0.5 );
			ps_mb->i_quantizer_scale = ( int32_t ) ( ( 1 << 12 ) * d_quantizer_scale );
		}
	}
}



void y262_lookahead_fill_ratectrl_vars( y262_t *ps_y262, y262_picture_t *ps_pic )
{
	int32_t i_don, i_counter = 0;
	for( i_don = ps_pic->i_don; i_don < ps_pic->i_don + ps_y262->i_num_lookahead_pictures; i_don++ )
	{
		int32_t i_fpic_idx;
		y262_picture_t *ps_lpic;

		if( ps_y262->i_current_eof_don >= 0 && i_don >= ps_y262->i_current_eof_don )
		{
			break;
		}

		if( i_don - ps_pic->i_don >= MAX_BITRATE_CONTROL_LOOKAHEAD_PICTURES )
		{
			break;
		}

		i_fpic_idx = y262_get_input_frame_don( ps_y262, i_don );
		ps_lpic = &ps_y262->rgs_buffered_input_pictures[ i_fpic_idx ];

		ps_pic->rgi_lookahead_picture_costs[ i_counter ] = ps_lpic->i_frame_cost;
		ps_pic->rgi_lookahead_picture_types[ i_counter ] = ps_lpic->i_frame_type;
		i_counter++;
	}
	ps_pic->i_num_lookahead_pictures = i_counter;
}


