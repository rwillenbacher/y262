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



bool_t y262_ratectrl_init( y262_t *ps_y262 )
{
	int32_t i_num_mb;
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	ps_ratectrl->i_vbv_occupancy = ps_ratectrl->i_vbv_size;
	ps_ratectrl->i64_vbv_occupancy_fractional = 0;

	ps_ratectrl->i_timescale = ps_y262->i_sequence_derived_timescale;
	ps_ratectrl->i_picture_duration = ps_y262->i_sequence_derived_picture_duration;
	ps_ratectrl->i_pulldown_timescale = ps_y262->i_sequence_derived_pulldown_timescale;
	ps_ratectrl->i_pulldown_picture_duration = ps_y262->i_sequence_derived_pulldown_picture_duration;

	ps_ratectrl->i64_output_ticks = 0;
	ps_ratectrl->i64_output_frames = 0;
	ps_ratectrl->i64_output_seconds = 0;
	ps_ratectrl->d_output_bits = 200.0;
	ps_ratectrl->d_qb_qplx = 12.0 * 200.0;
	ps_ratectrl->d_target_bits = 200.0;
	ps_ratectrl->rgd_satd_predictors[ 0 ] = 1.0;
	ps_ratectrl->rgd_satd_predictors_weight[ 0 ] = 1.0;
	ps_ratectrl->rgd_satd_predictors[ 1 ] = 1.0;
	ps_ratectrl->rgd_satd_predictors_weight[ 1 ] = 1.0;
	ps_ratectrl->rgd_satd_predictors[ 2 ] = 1.0;
	ps_ratectrl->rgd_satd_predictors_weight[ 2 ] = 1.0;
	ps_ratectrl->rgd_satd_predictors[ 3 ] = 1.0;
	ps_ratectrl->rgd_satd_predictors_weight[ 3 ] = 1.0;
	ps_ratectrl->d_confidence_predict_behind = 1.0;
	ps_ratectrl->d_confidence_predict_ahead = 1.0;
	ps_ratectrl->ps_samples = NULL;

	ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] = ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] = 0;
	ps_ratectrl->rgd_last_ref_quantizers[ 0 ] = ps_ratectrl->rgd_last_ref_quantizers[ 1 ] = 0.0;

	ps_ratectrl->i_i_picture_baseline_bits = 0;
	ps_ratectrl->i_min_satd_for_satd_prediction = ( ( ps_y262->i_sequence_width * ps_y262->i_sequence_height ) / 256 ) * 11;
	ps_ratectrl->i_min_bits_for_satd_prediction = ( ( ps_y262->i_sequence_width * ps_y262->i_sequence_height ) / 256 ) * 3;

	if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS2 )
	{
		int32_t i_don, i_num_samples, i_sample_size, i_iter;
		double d_target_bits_per_sample = ( ( ( double )ps_ratectrl->i_bitrate ) * ps_ratectrl->i_picture_duration ) / ps_ratectrl->i_timescale;
		double d_sequence_quantizer, d_sequence_cplx, d_estimated_bits, d_adjust;
		y262_ratectrl_sample_t s_sample;

		if( !ps_y262->s_funcs.pf_rcsample_callback )
		{
			return FALSE;
		}
		
		i_num_samples = 0;
		while( ( i_sample_size = ps_y262->s_funcs.pf_rcsample_callback( ps_y262->p_cb_handle, i_num_samples, ( uint8_t *)&s_sample, sizeof( s_sample ) ) ) )
		{
			if( i_sample_size != sizeof( s_sample ) )
			{
				return FALSE;
			}
			i_num_samples++;
		}

		i_don = 0;
		ps_ratectrl->d_qb_qplx_2p = 0;
		ps_ratectrl->d_target_bits_2p = 0;
		ps_ratectrl->d_estimated_bits = 0;
		d_sequence_cplx = 0;

		ps_ratectrl->ps_samples = ( y262_ratectrl_isample_t * ) y262_alloc( sizeof( y262_ratectrl_isample_t ) * i_num_samples );
		if( ps_ratectrl->ps_samples == NULL )
		{
			return FALSE;
		}

		while( i_don < i_num_samples )
		{
			i_sample_size = ps_y262->s_funcs.pf_rcsample_callback( ps_y262->p_cb_handle, i_don, ( uint8_t *)&s_sample, sizeof( s_sample ) );
			if( i_sample_size != sizeof( s_sample ) )
			{
				return FALSE;
			}
			ps_ratectrl->ps_samples[ i_don ].d_cplx = ( ( double )s_sample.i_cplx_f8 ) / 256.0;
			ps_ratectrl->ps_samples[ i_don ].d_quantizer = ( ( double )s_sample.i_quantizer_f8 ) / 256.0;
			ps_ratectrl->ps_samples[ i_don ].i_bits = s_sample.i_bits;
			ps_ratectrl->ps_samples[ i_don ].i_estimated_bits = 0;
			ps_ratectrl->ps_samples[ i_don ].ui8_frame_type = s_sample.ui8_frame_type;
			ps_ratectrl->ps_samples[ i_don ].i_satd_cost = s_sample.i_satd_cost;

			ps_ratectrl->d_target_bits_2p += d_target_bits_per_sample;
			ps_ratectrl->d_qb_qplx_2p += ( ps_ratectrl->ps_samples[ i_don ].d_quantizer * ps_ratectrl->ps_samples[ i_don ].i_bits ) / ps_ratectrl->ps_samples[ i_don ].d_cplx;
			d_sequence_cplx += ps_ratectrl->ps_samples[ i_don ].d_cplx;

			i_don++;
		}
		d_sequence_cplx /= ( double )i_num_samples;

		i_don = 0;
		i_iter = 0;
		d_adjust = 1.0;
		while( 1 )
		{
			d_sequence_quantizer = ( ps_ratectrl->d_qb_qplx_2p * d_sequence_cplx ) / ps_ratectrl->d_target_bits_2p;
			d_sequence_quantizer *= d_adjust;

			d_estimated_bits = 0;
			i_don = 0;
			while( i_don < i_num_samples )
			{
				double d_sample_quantizer = ( ( ps_ratectrl->ps_samples[ i_don ].d_cplx / ps_ratectrl->d_target_bits_2p ) * ps_ratectrl->d_qb_qplx_2p ) * d_adjust;
				ps_ratectrl->ps_samples[ i_don ].i_estimated_bits =  ( int32_t ) ( ( ps_ratectrl->ps_samples[ i_don ].i_bits * ps_ratectrl->ps_samples[ i_don ].d_quantizer ) / d_sample_quantizer );
				d_estimated_bits += ps_ratectrl->ps_samples[ i_don ].i_estimated_bits;
				i_don++;
			}

			if( fabs( d_estimated_bits - ps_ratectrl->d_target_bits_2p ) < ps_ratectrl->d_target_bits_2p / 200.0 || i_iter > 10 )
			{
				break;
			}
			i_iter++;
			d_adjust = d_estimated_bits / ps_ratectrl->d_target_bits_2p;
		}
		ps_ratectrl->d_qb_qplx_2p *= d_adjust;
		ps_ratectrl->i_current_sample = 0;
		ps_ratectrl->i_num_samples = i_num_samples;
	}

	ps_ratectrl->i_picture_scaled_satd = 0;

	i_num_mb = ( ps_y262->i_sequence_width >> 4 ) * ( ps_y262->i_sequence_height >> 4 );
	ps_ratectrl->ps_mb_samples = y262_alloc( sizeof( y262_ratectrl_mb_sample_t ) * i_num_mb );

	return TRUE;
}

void y262_ratectrl_deinit( y262_t *ps_y262 )
{
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	if( ps_ratectrl->ps_samples )
	{
		y262_dealloc( ps_ratectrl->ps_samples );
	}
	if( ps_ratectrl->ps_mb_samples )
	{
		y262_dealloc( ps_ratectrl->ps_mb_samples );
	}
}



double y262_ratectrl_get_cplx( y262_t *ps_y262, int32_t i_cost )
{
	if( 1 )
	{
		return 1.0;
	}
	else
	{
		return sqrt( ( float )i_cost );
	}
}


int32_t y262_ratectrl_predict_frame_size_baseline( y262_t *ps_y262, int32_t i_picture_type )
{
	int32_t i_bits;
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	if( i_picture_type == PICTURE_CODING_TYPE_I )
	{
		i_bits = ps_ratectrl->i_i_picture_baseline_bits;
		i_bits += ( ( ps_y262->i_sequence_width * ps_y262->i_sequence_height ) / 256 ) * 4 * 8;
	}
	else
	{
		i_bits = ( ( ( ps_y262->i_sequence_width * ps_y262->i_sequence_height ) / 256 ) * 5 ) / 2; 
	}

	return i_bits;
}




int32_t y262_ratectrl_predict_frame_size( y262_t *ps_y262, int32_t i_picture_type, int32_t i_picture_cost, double d_quantizer )
{
	int32_t i_bits;
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	i_bits = y262_ratectrl_predict_frame_size_baseline( ps_y262, i_picture_type );

	i_bits += ( int32_t ) ( ( ( ps_ratectrl->rgd_satd_predictors[ i_picture_type ] * i_picture_cost ) / d_quantizer ) / ps_ratectrl->rgd_satd_predictors_weight[ i_picture_type ] );

	return i_bits;
}


int32_t y262_ratectrl_predict_frame_size_2pass_i( y262_t *ps_y262, int32_t i_picture_type, int32_t i_picture_cost, double d_quantizer, int32_t i_don )
{
	int32_t i_idx, i_count, i_baseline, i_bits_ahead;
	y262_bitrate_control_t *ps_ratectrl;
	double d_satd_pred, d_satd_pred_weight, d_weight;

	ps_ratectrl = &ps_y262->s_ratectrl;

	i_baseline = y262_ratectrl_predict_frame_size_baseline( ps_y262, i_picture_type );

	if( i_picture_cost > ps_ratectrl->i_min_satd_for_satd_prediction )
	{
		d_satd_pred = d_satd_pred_weight = 0.0;
		d_weight = 1.0;
		i_count = 0;
		for( i_idx = i_don; i_idx < ps_ratectrl->i_num_samples; i_idx++ )
		{
			y262_ratectrl_isample_t *ps_sample = &ps_ratectrl->ps_samples[ i_idx ];
			if( ps_sample->ui8_frame_type == i_picture_type && ps_sample->i_satd_cost > ps_ratectrl->i_min_satd_for_satd_prediction )
			{
				d_satd_pred += ( ( MAX( ps_sample->i_bits - i_baseline, i_baseline * 0.05 ) * ps_sample->d_quantizer ) / ps_sample->i_satd_cost ) * d_weight;
				d_satd_pred_weight += d_weight;
				d_weight = d_weight * 0.5;
				i_count++;
				if( i_count >= 3 )
				{
					break;
				}
			}
		}
		if( i_count > 0 )
		{
			i_bits_ahead = i_baseline;
			i_bits_ahead += ( int32_t ) ( ( ( d_satd_pred * i_picture_cost ) / d_quantizer ) / d_satd_pred_weight );
			return i_bits_ahead;
		}
	}

	return ( int32_t ) ( i_baseline * 1.3 );
}


int32_t y262_ratectrl_predict_frame_size_2pass( y262_t *ps_y262, int32_t i_picture_type, int32_t i_picture_cost, double d_quantizer, int32_t i_don )
{
	int32_t i_bits_ahead, i_bits_behind;
	y262_bitrate_control_t *ps_ratectrl;
	double d_conf_sum, d_predicted;

	ps_ratectrl = &ps_y262->s_ratectrl;

	i_bits_behind = y262_ratectrl_predict_frame_size( ps_y262, i_picture_type, i_picture_cost, d_quantizer );
	i_bits_ahead = y262_ratectrl_predict_frame_size_2pass_i( ps_y262, i_picture_type, i_picture_cost, d_quantizer, i_don );

	d_conf_sum = ps_ratectrl->d_confidence_predict_behind + ps_ratectrl->d_confidence_predict_ahead;
	d_predicted = ( i_bits_behind * ps_ratectrl->d_confidence_predict_behind ) + ( i_bits_ahead * ps_ratectrl->d_confidence_predict_ahead );
	d_predicted = d_predicted / d_conf_sum;

	d_predicted = MIN( INT32_MAX, MAX( 1, d_predicted ) );

	return ( int32_t ) d_predicted;
}


int32_t y262_ratectrl_predict_first_frame_size( y262_t *ps_y262, int32_t i_picture_type, int32_t i_picture_cost, double d_quantizer, int32_t i_don, int32_t i_which )
{
	y262_bitrate_control_t *ps_ratectrl;
	y262_picture_t *ps_picture;
	int32_t i_estimated_bits;
	double d_intra_weight, d_intra_bits, d_type_bits;

	ps_ratectrl = &ps_y262->s_ratectrl;
	ps_picture = ps_y262->ps_input_picture;

	if( i_which == 1 || ps_ratectrl->i_mode != BITRATE_CONTROL_PASS2 )
	{
		d_intra_bits = y262_ratectrl_predict_frame_size( ps_y262, PICTURE_CODING_TYPE_I, ps_picture->i_frame_intra_cost, d_quantizer );
		d_type_bits = y262_ratectrl_predict_frame_size( ps_y262, ps_picture->i_frame_type, ps_picture->i_frame_cost, d_quantizer );
	}
	else if( i_which == 0 )
	{
		d_intra_bits = y262_ratectrl_predict_frame_size_2pass( ps_y262, PICTURE_CODING_TYPE_I, ps_picture->i_frame_intra_cost, d_quantizer, ps_picture->i_don );
		d_type_bits = y262_ratectrl_predict_frame_size_2pass( ps_y262, ps_picture->i_frame_type, ps_picture->i_frame_cost, d_quantizer, ps_picture->i_don );
	}
	else
	{
		if( i_which != 2 )
		{
			int32_t *pi_null = NULL;
			*pi_null = 0;
		}
		d_intra_bits = y262_ratectrl_predict_frame_size_2pass_i( ps_y262, PICTURE_CODING_TYPE_I, ps_picture->i_frame_intra_cost, d_quantizer, ps_picture->i_don );
		d_type_bits = y262_ratectrl_predict_frame_size_2pass_i( ps_y262, ps_picture->i_frame_type, ps_picture->i_frame_cost, d_quantizer, ps_picture->i_don );
	}
	if( ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] < ps_picture->i_pon )
	{
		d_intra_weight = MAX( 0.0, ( ps_ratectrl->rgd_last_ref_quantizers[ 1 ] - d_quantizer ) / ps_ratectrl->rgd_last_ref_quantizers[ 1 ] );
	}
	else
	{
		d_intra_weight = 0.0;
	}
	i_estimated_bits = ( int32_t ) ( ( d_intra_bits * d_intra_weight ) + ( d_type_bits * ( 1.0 - d_intra_weight ) ) );

	return i_estimated_bits;
}



double y262_ratectrl_get_picture_quantizer( y262_t *ps_y262, int32_t i_picture_type, double d_quantizer )
{
	return d_quantizer;
}



double y262_ratectrl_clamp_to_vbv( y262_t *ps_y262, double d_quantizer )
{
	int32_t i_idx, i_estimated_bits, i_iterations;
	int32_t i_estimated_vbv, i_start_estimated_vbv, i_lower_limit, i_upper_limit, i_total_vbv_gain, i_vbv_gain, i_vbv_pictures, i_first_keyframe, i_down_limit;
	bool_t b_up = FALSE, b_down = FALSE, b_cbr, b_force_down;
	y262_bitrate_control_t *ps_ratectrl;
	y262_picture_t *ps_picture;

	ps_ratectrl = &ps_y262->s_ratectrl;
	ps_picture = ps_y262->ps_input_picture;

	b_cbr = ps_ratectrl->i_bitrate == ps_ratectrl->i_vbvrate;

	d_quantizer = MIN( 31.0, MAX( 0.1, d_quantizer ) );

	if( ps_picture->i_pon == 66 )
	{
		ps_picture = ps_picture;
	}

	i_vbv_gain = ( ( ( int64_t ) ps_ratectrl->i_vbvrate ) * ps_ratectrl->i_picture_duration ) / ps_ratectrl->i_timescale;
	i_vbv_pictures = ( ps_ratectrl->i_vbv_size / i_vbv_gain ) + 1;

	i_iterations = 0;
	while( i_iterations++ < 1000 )
	{
		i_estimated_vbv = ps_ratectrl->i_vbv_occupancy;
		i_total_vbv_gain = 0;

		if( d_quantizer < 0.1 )
		{
			d_quantizer = 0.1;
			break;
		}
		if( d_quantizer > 31.0 )
		{
			d_quantizer = 31.0;
			break;
		}

		i_estimated_vbv -= i_vbv_gain; /* adjust */
		i_start_estimated_vbv = i_estimated_vbv;
		i_first_keyframe = -1;
		b_force_down = FALSE;

		for( i_idx = 0; i_idx < ps_picture->i_num_lookahead_pictures; i_idx++ )
		{
			double d_picture_quantizer;

			i_estimated_vbv += i_vbv_gain;
			i_estimated_vbv = MIN( i_estimated_vbv, ps_ratectrl->i_vbv_size );

			i_total_vbv_gain += i_vbv_gain;

			d_picture_quantizer = y262_ratectrl_get_picture_quantizer( ps_y262, ps_picture->rgi_lookahead_picture_types[ i_idx ], d_quantizer );
			if( i_idx == 0 )
			{
				i_estimated_bits = y262_ratectrl_predict_first_frame_size( ps_y262, ps_picture->rgi_lookahead_picture_types[ i_idx ], ps_picture->rgi_lookahead_picture_costs[ i_idx ], d_picture_quantizer, ps_picture->i_don, 0 );
			}
			else
			{
				if( i_first_keyframe == -1 && ps_picture->rgi_lookahead_picture_types[ i_idx ] == PICTURE_CODING_TYPE_I )
				{
					i_first_keyframe = i_idx;
				}
				if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS2 )
				{
					i_estimated_bits = y262_ratectrl_predict_frame_size_2pass( ps_y262, ps_picture->rgi_lookahead_picture_types[ i_idx ], ps_picture->rgi_lookahead_picture_costs[ i_idx ], d_picture_quantizer, ps_picture->i_don + i_idx );
				}
				else
				{
					i_estimated_bits = y262_ratectrl_predict_frame_size( ps_y262, ps_picture->rgi_lookahead_picture_types[ i_idx ], ps_picture->rgi_lookahead_picture_costs[ i_idx ], d_picture_quantizer );
				}
			}
			
			i_estimated_bits = ( int32_t ) ( i_estimated_bits * 1.05 );
			i_estimated_vbv -= i_estimated_bits;
			if( i_estimated_vbv < ps_ratectrl->i_vbv_size / 4 )
			{
				b_force_down = TRUE;
				break;
			}
			if( ( i_estimated_bits * 1.4 ) > ps_ratectrl->i_vbv_size )
			{
				b_force_down = TRUE;
				break;
			}
		}

		if( i_first_keyframe == -1 )
		{
			i_first_keyframe = ps_picture->i_num_lookahead_pictures;
		}
		i_down_limit = MAX( i_first_keyframe, ( ( i_vbv_pictures * 3 ) / 2 ) );

		i_lower_limit = ps_ratectrl->i_vbv_size / 2;
		i_lower_limit = MIN( i_lower_limit, i_start_estimated_vbv + ( i_total_vbv_gain / 2 ) );

		i_upper_limit = ( ps_ratectrl->i_vbv_size * 9 ) / 10;
		i_upper_limit = MAX( i_upper_limit, i_start_estimated_vbv - ( i_total_vbv_gain / 2 ) );

		/*if( ps_y262->ps_input_picture->i_pon == 2652 )
			fprintf( stderr, "iter: %d ( %d %d )\n", i_estimated_vbv, i_lower_limit, i_upper_limit );*/

		if( i_estimated_vbv < i_lower_limit && !b_up && ( i_idx <= i_down_limit || b_force_down ) )
		{
			d_quantizer *= 1.1;
			b_down = TRUE;
			continue;
		}
		if( i_estimated_vbv > i_upper_limit && !b_down && b_cbr )
		{
			d_quantizer *= 0.9;
			b_up = TRUE;
			continue;
		}
		break;
	}

	d_quantizer = MIN( 31.0, MAX( 0.1, d_quantizer ) );
	/*if( ps_y262->ps_input_picture->i_pon == 2652 )
		fprintf( stderr, "end: %f ( %d )\n", d_quantizer, i_estimated_vbv ); */
	return d_quantizer;
}


void y262_ratectrl_start_picture( y262_t *ps_y262, int32_t i_header_bits )
{
	int32_t i_picture_cost, i_picture_type, i_predicted_frame_size, i_predicted_frame_size_behind, i_predicted_frame_size_ahead;
	double d_picture_cplx, d_quantizer;
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	i_picture_cost = ps_y262->ps_input_picture->i_frame_cost;
	i_picture_type = ps_y262->ps_input_picture->i_frame_type;

	d_picture_cplx = y262_ratectrl_get_cplx( ps_y262, i_picture_cost );
	
	if( i_picture_type == PICTURE_CODING_TYPE_I )
	{
		ps_ratectrl->i_i_picture_baseline_bits = i_header_bits;
	}

	if( ps_ratectrl->i_mode == BITRATE_CONTROL_CQ )
	{
		d_quantizer = ps_y262->i_quantizer;
	}
	else if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS1 )
	{
		double d_adjust;

		d_quantizer = ( d_picture_cplx / ps_ratectrl->d_target_bits ) * ps_ratectrl->d_qb_qplx;

		/* short term adjust */
		d_adjust = 1.0 + ( ( ps_ratectrl->d_output_bits - ps_ratectrl->d_target_bits ) / ps_ratectrl->i_vbv_size );
		d_adjust = MIN( 2.5, MAX( 0.5, d_adjust ) );
		d_quantizer = d_quantizer * d_adjust;
	}
	else if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS2 )
	{
		double d_adjust;

		d_quantizer = ( d_picture_cplx / ps_ratectrl->d_target_bits_2p ) * ps_ratectrl->d_qb_qplx_2p;

		/* long term adjust */
		d_adjust = 1.0 + ( ( ps_ratectrl->d_output_bits - ps_ratectrl->d_estimated_bits ) / ( ps_ratectrl->d_target_bits_2p / 20.0 ) );
		d_adjust = MIN( 2.5, MAX( 0.5, d_adjust ) );
		d_quantizer = d_quantizer * d_adjust;
	}
	else
	{
		assert( FALSE );
	}
	/*fprintf( stderr, "pre: %3.2f -", d_quantizer );*/
	d_quantizer = y262_ratectrl_get_picture_quantizer( ps_y262, ps_y262->ps_input_picture->i_frame_type, d_quantizer );

	d_quantizer = y262_ratectrl_clamp_to_vbv( ps_y262, d_quantizer );

	/*fprintf( stderr, "post: %3.2f -", d_quantizer );*/
	d_quantizer = MIN( 31.0, MAX( 0.1, d_quantizer ) );

	if( ps_y262->ps_input_picture->i_frame_type == PICTURE_CODING_TYPE_B )
	{
		/*fprintf( stderr, " rqp %d %d %d\n", ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] , ps_y262->ps_input_picture->i_pon , ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] );*/
		if( ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] < ps_y262->ps_input_picture->i_pon &&
			ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] > ps_y262->ps_input_picture->i_pon )
		{
			double d_mid_quantizer, d_delta, d_w1, d_w2;
			d_delta = ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] - ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ];
			d_w1 = d_delta - ( ps_y262->ps_input_picture->i_pon - ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] );
			d_w2 = d_delta - ( ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] - ps_y262->ps_input_picture->i_pon );
			d_mid_quantizer = ( ps_ratectrl->rgd_last_ref_quantizers[ 0 ] * d_w1 ) + ( ps_ratectrl->rgd_last_ref_quantizers[ 1 ] * d_w2 );
			d_mid_quantizer = d_mid_quantizer / d_delta;
			d_quantizer = MAX( d_mid_quantizer, d_quantizer );
		}
		else if( ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] > ps_y262->ps_input_picture->i_pon )
		{
			d_quantizer = MAX( d_quantizer, ps_ratectrl->rgd_last_ref_quantizers[ 0 ] );
		}
		else if( ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] < ps_y262->ps_input_picture->i_pon )
		{
			d_quantizer = MAX( d_quantizer, ps_ratectrl->rgd_last_ref_quantizers[ 1 ] );
		}
		else
		{
			d_quantizer = MAX( d_quantizer, MAX( ps_ratectrl->rgd_last_ref_quantizers[ 0 ], ps_ratectrl->rgd_last_ref_quantizers[ 1 ] ) );
		}
	}

	d_quantizer = MIN( 31.0, MAX( 0.1, d_quantizer ) );
	/*fprintf( stderr, "post2: %3.2f -\n", d_quantizer );*/

	i_predicted_frame_size = y262_ratectrl_predict_first_frame_size( ps_y262, i_picture_type, i_picture_cost, d_quantizer, ps_y262->ps_input_picture->i_don, 0 );
	i_predicted_frame_size_behind = y262_ratectrl_predict_first_frame_size( ps_y262, i_picture_type, i_picture_cost, d_quantizer, ps_y262->ps_input_picture->i_don, 1 );
	if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS2 )
	{
		i_predicted_frame_size_ahead = y262_ratectrl_predict_first_frame_size( ps_y262, i_picture_type, i_picture_cost, d_quantizer, ps_y262->ps_input_picture->i_don, 2 );
	}
	else
	{
		i_predicted_frame_size_ahead = i_predicted_frame_size_behind;
	}


	ps_ratectrl->i_quantizer = ( int32_t ) ( d_quantizer * 256.0 );

	ps_ratectrl->i_picture_accumulated_quantizer = 0;
	ps_ratectrl->i_picture_num_accumulated_quantizer = 0;
	ps_ratectrl->d_picture_accumulated_quantizer_bits = 0.0;
	ps_ratectrl->d_picture_accumulated_bits_quantizer_over_satd = 0.0;
	ps_ratectrl->i_num_picture_accumulated_bits_quantizer_over_satd = 0;
	ps_ratectrl->i_picture_bit_budget = ( int32_t ) ( ( i_predicted_frame_size * 1.3 ) * ( 1.0 + ( 0.2 * ps_y262->i_num_slice_encoders ) ) );
	ps_ratectrl->i_picture_bit_budget += MAX( 0, ( ps_ratectrl->i_vbv_occupancy - ps_ratectrl->i_picture_bit_budget ) / 4 );
	ps_ratectrl->i_picture_bit_budget = MIN( ps_ratectrl->i_picture_bit_budget, ps_ratectrl->i_vbv_occupancy - ( ps_ratectrl->i_vbv_occupancy / 10 ) );
	ps_ratectrl->i_picture_coded_scaled_satd = 0;
	ps_ratectrl->i_picture_coded_size = 0;
	ps_ratectrl->i_picture_adjusted_bit_budget = ps_ratectrl->i_picture_bit_budget;
	ps_ratectrl->i_predicted_frame_size = i_predicted_frame_size;
	ps_ratectrl->i_predicted_frame_size_behind = i_predicted_frame_size_behind;
	ps_ratectrl->i_predicted_frame_size_ahead = i_predicted_frame_size_ahead;
	ps_ratectrl->b_picture_bad_encountered = FALSE;
	ps_ratectrl->i_picture_uncoded_size = 0;
	ps_ratectrl->b_picture_reencode_pass = FALSE;


	/*fprintf( stderr, "pred %d budget %d\n", i_predicted_frame_size, ps_ratectrl->i_picture_bit_budget );*/

	/*fprintf( stderr, "PRED: %d %f : %d\n", ps_y262->ps_input_picture->i_frame_cost, d_quantizer,
		y262_ratectrl_predict_frame_size( ps_y262, ps_y262->ps_input_picture->i_frame_type, ps_y262->ps_input_picture->i_frame_cost, d_quantizer ) );*/

	{
		int32_t i_mb_idx, i_num_mb;
		ps_ratectrl->i_picture_scaled_satd = 0;

		i_num_mb = ( ps_y262->i_sequence_width >> 4 ) * ( ps_y262->i_sequence_height >> 4 );
		
		for( i_mb_idx = 0; i_mb_idx < i_num_mb; i_mb_idx++ )
		{
			int32_t i_satd, i_scale, i_scaled_satd;
			if( ps_y262->ps_input_picture->i_frame_type == PICTURE_CODING_TYPE_I )
			{
				i_satd = ps_y262->ps_input_picture->ps_lookahead[ i_mb_idx ].i_intra_cost;
			}
			else
			{
				i_satd = ps_y262->ps_input_picture->ps_lookahead[ i_mb_idx ].i_best_cost;
			}
			i_scale = ( ps_y262->ps_input_picture->ps_lookahead[ i_mb_idx ].i_quantizer_scale * ps_y262->ps_input_picture->ps_lookahead[ i_mb_idx ].i_quantizer_aq_scale ) >> 12;
			i_scaled_satd = ( i_satd << 12 ) / i_scale;

			ps_ratectrl->ps_mb_samples[ i_mb_idx ].i_satd = i_satd;
			ps_ratectrl->ps_mb_samples[ i_mb_idx ].i_scaled_satd = i_scaled_satd;

			ps_ratectrl->i_picture_scaled_satd += i_scaled_satd;
		}
	}

	//fprintf( stderr, "predict: %d, budget: %d\n", y262_ratectrl_predict_frame_size( ps_y262, ps_y262->ps_input_picture->i_frame_type, ps_y262->ps_input_picture->i_frame_cost, d_quantizer ), ps_ratectrl->i_picture_bit_budget );
	/*fprintf( stderr, "%d: quant: %f\n", ps_y262->ps_input_picture->i_pon, d_quantizer );*/

	if( !ps_y262->b_sequence_mpeg1 )
	{
		int32_t i_vbv_delay, i_occupancy_bits;
		int64_t i64_occupancy_ticks;

		i_occupancy_bits = ps_ratectrl->i_vbv_occupancy - i_header_bits;

		i64_occupancy_ticks = ( ( ( ( int64_t ) 90000 ) * ps_ratectrl->i_vbv_size ) + ( ps_ratectrl->i_vbvrate - 1 ) ) / ps_ratectrl->i_vbvrate;
		if( i64_occupancy_ticks >= 0xffff )
		{
			/* buffer too large, signal "*shrug*" */
			i_vbv_delay = 0xffff;
		}
		else
		{
			i64_occupancy_ticks = ( ( ( ( int64_t ) 90000 ) * ( i_occupancy_bits + !!ps_ratectrl->i64_vbv_occupancy_fractional ) ) + ( ps_ratectrl->i_vbvrate - 1 ) ) / ps_ratectrl->i_vbvrate;
		}
		ps_y262->ps_input_picture->i_vbv_delay = ( int32_t ) i64_occupancy_ticks;
	}
	else
	{
		int32_t i_vbv_delay, i_occupancy_bits;
		int64_t i64_occupancy_ticks;

		if( !ps_y262->b_sequence_cbr )
		{
			i_vbv_delay = 0xffff; /* signal vbr */
		}
		else
		{
			i_occupancy_bits = ps_ratectrl->i_vbv_occupancy - i_header_bits;

			i64_occupancy_ticks = ( ( ( ( int64_t ) 90000 ) * ps_ratectrl->i_vbv_size ) + ( ps_ratectrl->i_vbvrate - 1 ) ) / ps_ratectrl->i_vbvrate;
			if( i64_occupancy_ticks >= 0xffff )
			{
				/* buffer too large, signal vbr as best effort */
				i_vbv_delay = 0xffff;
			}
			else
			{
				i64_occupancy_ticks = ( ( ( ( int64_t ) 90000 ) * ( i_occupancy_bits + !!ps_ratectrl->i64_vbv_occupancy_fractional ) ) + ( ps_ratectrl->i_vbvrate - 1 ) ) / ps_ratectrl->i_vbvrate;
			}
		}
		ps_y262->ps_input_picture->i_vbv_delay = ( int32_t ) i64_occupancy_ticks;
	}
}


bool_t y262_ratectrl_commit_bits( y262_t *ps_y262, int32_t i_bits )
{
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	ps_ratectrl->i_vbv_occupancy -= i_bits;
	if( ps_ratectrl->i_vbv_occupancy < 0 )
	{
		return FALSE;
	}
	return TRUE;
}


int32_t y262_ratectrl_stuffing_bits_needed( y262_t *ps_y262 )
{
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	if( ps_ratectrl->i_vbv_occupancy_overflow >= ps_ratectrl->i_vbv_size )
	{
		return ps_ratectrl->i_vbv_occupancy_overflow + 2 - ps_ratectrl->i_vbv_size; /* +1 because of fractional, +1 because reasons */
	}
	else
	{
		return 0;
	}
}


void y262_ratectrl_commit_stuffing_bits( y262_t *ps_y262, int32_t i_bits )
{
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	ps_ratectrl->i_vbv_occupancy = ps_ratectrl->i_vbv_occupancy_overflow - i_bits;
	ps_ratectrl->i64_vbv_occupancy_fractional = ps_ratectrl->i64_vbv_occupancy_overflow_fractional;

	if( ps_ratectrl->i_vbv_occupancy >= ps_ratectrl->i_vbv_size )
	{
		if( ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_BUFFER, "Not enough stuffing bits commited, internal encoder error." );
		}
		ps_ratectrl->i_vbv_occupancy = ps_ratectrl->i_vbv_size;
		ps_ratectrl->i64_vbv_occupancy_fractional = 0;
	}
}


void y262_ratectrl_commit_ticks( y262_t *ps_y262, int32_t i_ticks )
{
	int64_t i64_vbv_gain_fractional;
	int32_t i_vbv_gain;
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	ps_ratectrl->i64_output_ticks += i_ticks;
	ps_ratectrl->i64_output_frames++;
	if( ps_ratectrl->i64_output_ticks >= ps_ratectrl->i_pulldown_timescale )
	{
		ps_ratectrl->i64_output_ticks -= ps_ratectrl->i_pulldown_timescale;
		ps_ratectrl->i64_output_frames = 0;
		ps_ratectrl->i64_output_seconds++;
	}

	i_vbv_gain = ( int32_t)( ( ( ( int64_t )ps_ratectrl->i_vbvrate ) * i_ticks ) / ps_ratectrl->i_pulldown_timescale );
	i64_vbv_gain_fractional = ( ps_ratectrl->i_vbvrate * i_ticks ) % ps_ratectrl->i_pulldown_timescale;

	ps_ratectrl->i_vbv_occupancy += i_vbv_gain;
	ps_ratectrl->i64_vbv_occupancy_fractional += i64_vbv_gain_fractional;
	while( ps_ratectrl->i64_vbv_occupancy_fractional >= ps_ratectrl->i_pulldown_timescale )
	{
		ps_ratectrl->i_vbv_occupancy++;
		ps_ratectrl->i64_vbv_occupancy_fractional -= ps_ratectrl->i_pulldown_timescale;
	}

	ps_ratectrl->i_vbv_occupancy_overflow = ps_ratectrl->i_vbv_occupancy;
	ps_ratectrl->i64_vbv_occupancy_overflow_fractional = ps_ratectrl->i64_vbv_occupancy_fractional;
	if( ps_ratectrl->i_vbv_occupancy >= ps_ratectrl->i_vbv_size )
	{
		ps_ratectrl->i_vbv_occupancy = ps_ratectrl->i_vbv_size;
		ps_ratectrl->i64_vbv_occupancy_fractional = 0;
	}
}


void y262_ratectrl_end_picture( y262_t *ps_y262, int32_t i_bits )
{
	int32_t i_picture_cost, i_picture_type, i_picture_ticks;
	double d_quantizer, d_cplx, d_bits_quantizer_over_satd, d_prev_set_quantizer;
	y262_bitrate_control_t *ps_ratectrl;
	y262_picture_t *ps_picture;

	ps_ratectrl = &ps_y262->s_ratectrl;
	ps_picture = ps_y262->ps_input_picture;

	d_prev_set_quantizer = ps_ratectrl->i_quantizer / 256.0;
	d_quantizer = ps_ratectrl->d_picture_accumulated_quantizer_bits / ps_ratectrl->i_picture_coded_size;
	/*fprintf( stderr, " set quant: %f, actual quant: %f\n", ( ( double ) ps_ratectrl->i_quantizer ) / 256.0, d_quantizer );*/
	i_picture_cost = ps_y262->ps_input_picture->i_frame_cost;
	i_picture_type = ps_y262->ps_input_picture->i_frame_type;

	d_bits_quantizer_over_satd = ps_ratectrl->d_picture_accumulated_bits_quantizer_over_satd / ps_ratectrl->i_num_picture_accumulated_bits_quantizer_over_satd;

	/* single pass */
	ps_ratectrl->d_target_bits += ( ( ( double )ps_ratectrl->i_bitrate ) * ps_ratectrl->i_picture_duration ) / ps_ratectrl->i_timescale;
	ps_ratectrl->d_output_bits += i_bits;
	d_cplx = y262_ratectrl_get_cplx( ps_y262, i_picture_cost );
	ps_ratectrl->d_qb_qplx += ( d_quantizer * i_bits ) / d_cplx;

	/* second pass */
	if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS2 )
	{
		if( ps_ratectrl->i_current_sample < ps_ratectrl->i_num_samples )
		{
			ps_ratectrl->d_estimated_bits += ps_ratectrl->ps_samples[ ps_ratectrl->i_current_sample ].i_estimated_bits;
			ps_ratectrl->i_current_sample++;
		}
		else
		{
			/* error */
		}
	}

	/*fprintf( stderr, "est: %d, actual %d, %f ( %f )\n", ps_ratectrl->i_predicted_frame_size, i_bits, ( double )ps_ratectrl->i_predicted_frame_size / i_bits, d_prev_set_quantizer );*/

	/* buffer model */
	/*fprintf( stderr, "actual: %d ( satd: %d )\n", i_bits, i_picture_cost );*/
	if( !y262_ratectrl_commit_bits( ps_y262, i_bits ) )
	{
		y262_error( ps_y262, Y262_ERROR_BUFFER, ( int8_t * )"buffer underrun by %d bits", ps_ratectrl->i_vbv_occupancy );
	}
	i_picture_ticks = ps_ratectrl->i_pulldown_picture_duration;
	i_picture_ticks += ps_y262->ps_input_picture->b_repeat_first_field ? ( ps_ratectrl->i_pulldown_picture_duration / 2 ) : 0;

	y262_ratectrl_commit_ticks( ps_y262, i_picture_ticks );

	/* update satd predictor */
	if( i_picture_cost > ps_ratectrl->i_min_satd_for_satd_prediction )
	{
		int32_t i_baseline_bits = y262_ratectrl_predict_frame_size_baseline( ps_y262, i_picture_type );
		int32_t i_satd_pred_bits = i_bits - i_baseline_bits;

		if( i_satd_pred_bits > ps_ratectrl->i_min_bits_for_satd_prediction )
		{
			ps_ratectrl->rgd_satd_predictors[ i_picture_type ] *= 0.5;
			ps_ratectrl->rgd_satd_predictors_weight[ i_picture_type ] *= 0.5;
			ps_ratectrl->rgd_satd_predictors[ i_picture_type ] += ( ( ( double ) i_satd_pred_bits ) / i_picture_cost ) * d_quantizer;
			ps_ratectrl->rgd_satd_predictors_weight[ i_picture_type ] += 1.0;
		}
		/*
		if( i_picture_type != PICTURE_CODING_TYPE_I )
		{
		if( ps_picture->i_frame_intra_cost < ( ps_picture->i_frame_cost * 1.2 ) )
			{
				ps_ratectrl->rgd_satd_predictors[ PICTURE_CODING_TYPE_I ] *= 0.5;
				ps_ratectrl->rgd_satd_predictors_weight[ PICTURE_CODING_TYPE_I ] *= 0.5;
				ps_ratectrl->rgd_satd_predictors[ PICTURE_CODING_TYPE_I ] += ( ( ( double ) i_bits ) / i_picture_cost ) * d_prev_set_quantizer;
				ps_ratectrl->rgd_satd_predictors_weight[ PICTURE_CODING_TYPE_I ] += 1.0;
			}
		}
		*/
	}
	else
	{
		i_picture_cost = i_picture_cost;
	}

	/* update confidence */
	if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS2 )
	{
		double d_delta_behind, d_conf_behind;
		double d_delta_ahead, d_conf_ahead;

		d_delta_behind = ps_ratectrl->i_predicted_frame_size_behind - i_bits;
		d_delta_ahead = ps_ratectrl->i_predicted_frame_size_ahead - i_bits;

		d_delta_behind *= d_delta_behind;
		d_delta_ahead *= d_delta_ahead;

		d_conf_behind = 1.0 / MAX( d_delta_behind, 1.0 );
		d_conf_ahead = 1.0 / MAX( d_delta_ahead, 1.0 );

		/*fprintf( stderr, "pred actual %d, merge: %d, behind: %d ( %f ), ahead: %d ( %f )\n", i_bits, ps_ratectrl->i_predicted_frame_size, ps_ratectrl->i_predicted_frame_size_behind, d_conf_behind, ps_ratectrl->i_predicted_frame_size_ahead, d_conf_ahead );*/

		ps_ratectrl->d_confidence_predict_behind = ( ps_ratectrl->d_confidence_predict_behind * 0.5 ) + d_conf_behind;
		ps_ratectrl->d_confidence_predict_ahead = ( ps_ratectrl->d_confidence_predict_ahead * 0.5 ) + d_conf_ahead;
	}

	if( i_picture_type != PICTURE_CODING_TYPE_B )
	{
		ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] = ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ];
		ps_ratectrl->rgd_last_ref_quantizers[ 0 ] = ps_ratectrl->rgd_last_ref_quantizers[ 1 ];
		ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] = ps_y262->ps_input_picture->i_pon;
		ps_ratectrl->rgd_last_ref_quantizers[ 1 ] = d_quantizer;
	}


	if( ps_y262->s_funcs.pf_result_callback )
	{
		y262_ratectrl_sample_t s_sample;
		y262_result_t s_result;

		s_sample.ui8_frame_type = i_picture_type;
		s_sample.i_bits = i_bits;
		s_sample.i_quantizer_f8 = ( int32_t )( d_quantizer * 256.0 );
		s_sample.i_cplx_f8 = ( int32_t )( d_cplx * 256 );
		s_sample.i_estimated_bits = 0;
		s_sample.i_satd_cost = ps_picture->i_frame_cost;

		s_result.rc_sample.i_data_length = sizeof( s_sample );
		s_result.rc_sample.pui8_data = ( uint8_t *)( &s_sample );
		s_result.rc_sample.i_don = ps_y262->ps_input_picture->i_don;

		ps_y262->s_funcs.pf_result_callback( ps_y262->p_cb_handle, Y262_RESULT_RC_SAMPLE, &s_result );
	}
}


bool_t y262_ratectrl_check_for_reencode( y262_t *ps_y262, int32_t i_bits )
{
	int32_t i_picture_cost, i_picture_type, i_predicted_frame_size, i_predicted_frame_size_ahead, i_predicted_frame_size_behind;
	double d_quantizer, d_prev_quantizer, d_prev_set_quantizer;
	y262_bitrate_control_t *ps_ratectrl;
	y262_picture_t *ps_picture;
	bool_t b_this_shall_pass = FALSE;

	ps_ratectrl = &ps_y262->s_ratectrl;

	ps_picture = ps_y262->ps_input_picture;

	if( ps_y262->ps_input_picture->i_pon == 48 )
	{
		ps_y262 = ps_y262;
	}

	if( i_bits < ( ps_ratectrl->i_min_bits_for_satd_prediction * 5 ) / 2 )
	{
		b_this_shall_pass = TRUE;
	}

	if( !b_this_shall_pass &&
		ps_ratectrl->b_picture_bad_encountered )
	{
		d_prev_set_quantizer = ps_ratectrl->i_quantizer / 256.0;
		d_prev_quantizer = ps_ratectrl->d_picture_accumulated_quantizer_bits / ps_ratectrl->i_picture_coded_size;

		i_picture_cost = ps_y262->ps_input_picture->i_frame_cost;
		i_picture_type = ps_y262->ps_input_picture->i_frame_type;
		
		/*if( ( i_picture_cost / ( ( ps_y262->i_sequence_width * ps_y262->i_sequence_height ) / 256 ) ) > 11 )
		{
			if( i_bits >( int32_t )( ps_ratectrl->i_predicted_frame_size * 2 ) )
			{
				ps_ratectrl->rgd_satd_predictors[ i_picture_type ] = ( ( ( double ) i_bits ) / i_picture_cost ) * d_prev_quantizer;
				ps_ratectrl->rgd_satd_predictors_weight[ i_picture_type ] = 1.0;
				if( i_picture_type != PICTURE_CODING_TYPE_I )
				{
					if( ps_y262->ps_input_picture->i_frame_intra_cost < ( ps_picture->i_frame_cost * 1.2 ) )
					{
						ps_ratectrl->rgd_satd_predictors[ PICTURE_CODING_TYPE_I ] = ( ( ( double ) i_bits * 1.2 ) / i_picture_cost ) * d_prev_quantizer;
						ps_ratectrl->rgd_satd_predictors_weight[ PICTURE_CODING_TYPE_I ] = 1.0;
					}
				}
			}
			d_quantizer = d_prev_quantizer;
		}
		else*/
		{
			d_quantizer = d_prev_quantizer;
			ps_ratectrl->i_predicted_frame_size = ps_ratectrl->i_picture_coded_size;
		}

		d_quantizer = y262_ratectrl_get_picture_quantizer( ps_y262, ps_y262->ps_input_picture->i_frame_type, d_quantizer );

		/*fprintf( stderr, "1: pre: %3.2f, enc: %3.2f, post: %3.2f - ( %3.2f ), %d %d\n", d_prev_set_quantizer, d_prev_quantizer, d_quantizer, d_quantizer - d_prev_set_quantizer, i_bits, ps_ratectrl->i_min_bits_for_satd_prediction );*/

		d_quantizer = y262_ratectrl_clamp_to_vbv( ps_y262, d_quantizer );

		/*fprintf( stderr, "2: pre: %3.2f, post: %3.2f - ( %3.2f ), %d %d\n", d_prev_set_quantizer, d_quantizer, d_quantizer - d_prev_set_quantizer, i_bits, ps_ratectrl->i_min_bits_for_satd_prediction );*/
		d_quantizer = MIN( 31.0, MAX( 0.1, d_quantizer ) );

		if( ps_y262->ps_input_picture->i_frame_type == PICTURE_CODING_TYPE_B )
		{
			if( ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] < ps_y262->ps_input_picture->i_pon &&
				ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] > ps_y262->ps_input_picture->i_pon )
			{
				double d_mid_quantizer, d_delta, d_w1, d_w2;
				d_delta = ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] - ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ];
				d_w1 = d_delta - ( ps_y262->ps_input_picture->i_pon - ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] );
				d_w2 = d_delta - ( ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] - ps_y262->ps_input_picture->i_pon );
				d_mid_quantizer = ( ps_ratectrl->rgd_last_ref_quantizers[ 0 ] * d_w1 ) + ( ps_ratectrl->rgd_last_ref_quantizers[ 0 ] * d_w2 );
				d_mid_quantizer = d_mid_quantizer / d_delta;
				d_quantizer = MAX( d_mid_quantizer, d_quantizer );
			}
			else if( ps_ratectrl->rgi_last_ref_quantizers_pons[ 0 ] > ps_y262->ps_input_picture->i_pon )
			{
				d_quantizer = MAX( d_quantizer, ps_ratectrl->rgd_last_ref_quantizers[ 0 ] );
			}
			else if( ps_ratectrl->rgi_last_ref_quantizers_pons[ 1 ] < ps_y262->ps_input_picture->i_pon )
			{
				d_quantizer = MAX( d_quantizer, ps_ratectrl->rgd_last_ref_quantizers[ 1 ] );
			}
			else
			{
				d_quantizer = MAX( d_quantizer, MAX( ps_ratectrl->rgd_last_ref_quantizers[ 0 ], ps_ratectrl->rgd_last_ref_quantizers[ 1 ] ) );
			}
		}
		d_quantizer = MIN( 31.0, MAX( 0.1, d_quantizer ) );

		i_predicted_frame_size = y262_ratectrl_predict_first_frame_size( ps_y262, i_picture_type, i_picture_cost, d_quantizer, ps_y262->ps_input_picture->i_don, 0 );
		i_predicted_frame_size_behind = y262_ratectrl_predict_first_frame_size( ps_y262, i_picture_type, i_picture_cost, d_quantizer, ps_y262->ps_input_picture->i_don, 1 );
		if( ps_ratectrl->i_mode == BITRATE_CONTROL_PASS2 )
		{
			i_predicted_frame_size_ahead = y262_ratectrl_predict_first_frame_size( ps_y262, i_picture_type, i_picture_cost, d_quantizer, ps_y262->ps_input_picture->i_don, 2 );
		}
		else
		{
			i_predicted_frame_size_ahead = i_predicted_frame_size_behind;
		}


		if( fabs( d_prev_set_quantizer - d_quantizer ) > ( d_quantizer * 0.1 ) ||
			ps_ratectrl->b_picture_bad_encountered )
		{
			int32_t i_mb_idx, i_num_mb;

			ps_ratectrl->i_quantizer = ( int32_t ) ( d_quantizer * 256.0 );

			ps_ratectrl->i_picture_accumulated_quantizer = 0;
			ps_ratectrl->i_picture_num_accumulated_quantizer = 0;
			ps_ratectrl->d_picture_accumulated_quantizer_bits = 0.0;
			ps_ratectrl->d_picture_accumulated_bits_quantizer_over_satd = 0.0;
			ps_ratectrl->i_num_picture_accumulated_bits_quantizer_over_satd = 0;
			ps_ratectrl->i_picture_coded_scaled_satd = 0;
			ps_ratectrl->i_picture_coded_size = 0;
			ps_ratectrl->b_picture_bad_encountered = FALSE;
			ps_ratectrl->i_picture_uncoded_size = 0;
			ps_ratectrl->b_picture_reencode_pass = TRUE;
/*			ps_ratectrl->i_predicted_frame_size = i_predicted_frame_size;
			ps_ratectrl->i_predicted_frame_size_behind = i_predicted_frame_size_behind;
			ps_ratectrl->i_predicted_frame_size_ahead = i_predicted_frame_size_ahead;*/

			i_num_mb = ( ps_y262->i_sequence_width >> 4 ) * ( ps_y262->i_sequence_height >> 4 );

			for( i_mb_idx = 0; i_mb_idx < i_num_mb; i_mb_idx++ )
			{
				int32_t i_coded_bits;
				double d_mb_quantizer, d_mb_size_scale;

				d_mb_quantizer = ps_ratectrl->ps_mb_samples[ i_mb_idx ].i_quantizer / 256.0;
				d_mb_size_scale = d_quantizer / d_mb_quantizer;

				i_coded_bits = ps_ratectrl->ps_mb_samples[ i_mb_idx ].i_coded_bits;
				i_coded_bits = MAX( 0, i_coded_bits - 5 );
				ps_ratectrl->ps_mb_samples[ i_mb_idx ].i_predicted_bits = ( ( int32_t )( i_coded_bits * d_mb_size_scale ) ) + 5;
			}

			return TRUE;
		}
	}
	return FALSE;
}


void y262_ratectrl_start_slice_encoder( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc, int32_t i_start_mb_addr, int32_t i_end_mb_addr )
{
	int32_t i_idx;
	double d_slice_in_picture, d_slice_base, d_slice_dynamic;
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	ps_slice_rc->i_slice_scaled_satd = 0;
	for( i_idx = i_start_mb_addr; i_idx <= i_end_mb_addr; i_idx++ )
	{
		ps_slice_rc->i_slice_scaled_satd += ps_ratectrl->ps_mb_samples[ i_idx ].i_scaled_satd;
	}

	if( ps_y262->ps_input_picture->i_pon == 2652 )
	{
		ps_y262 = ps_y262;
	}

	d_slice_in_picture = ( ( double )ps_slice_rc->i_slice_scaled_satd ) / ps_ratectrl->i_picture_scaled_satd;
	d_slice_base = ( ( double )ps_ratectrl->i_picture_bit_budget ) * 0.2 * ps_y262->i_num_slice_encoders;
	d_slice_dynamic = ( ( double )ps_ratectrl->i_picture_bit_budget ) - d_slice_base;
	d_slice_base /= ps_y262->i_num_slice_encoders;
	d_slice_dynamic *= d_slice_in_picture;

	ps_slice_rc->i_slice_bit_budget = ( int32_t ) ( ( d_slice_base + d_slice_dynamic ) * 0.8 );
	ps_slice_rc->i_slice_bit_budget_extra = ( int32_t )( ( d_slice_base + d_slice_dynamic ) * 0.2 );

	ps_slice_rc->i_slice_accumulated_quantizer = 0;
	ps_slice_rc->i_slice_num_accumulated_quantizer = 0;
	ps_slice_rc->i_slice_coded_scaled_satd = 0;
	ps_slice_rc->i_slice_coded_size = 0;
	ps_slice_rc->d_slice_accumulated_quantizer_bits = 0.0;
	ps_slice_rc->d_slice_accumulated_bits_quantizer_over_satd = 0.0;
	ps_slice_rc->i_num_slice_accumulated_bits_quantizer_over_satd = 0;
	ps_slice_rc->b_slice_bad_encountered = FALSE;

	ps_slice_rc->b_reencode_pass = ps_ratectrl->b_picture_reencode_pass;
	ps_slice_rc->i_slice_accumulated_predicted_size = 0;
}

void y262_ratectrl_end_slice_encoder( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc )
{
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	ps_ratectrl->i_picture_accumulated_quantizer += ps_slice_rc->i_slice_accumulated_quantizer;
	ps_ratectrl->i_picture_num_accumulated_quantizer += ps_slice_rc->i_slice_num_accumulated_quantizer;
	ps_ratectrl->d_picture_accumulated_quantizer_bits += ps_slice_rc->d_slice_accumulated_quantizer_bits;
	ps_ratectrl->i_picture_coded_scaled_satd += ps_slice_rc->i_slice_coded_scaled_satd;
	ps_ratectrl->i_picture_coded_size += ps_slice_rc->i_slice_coded_size;
	ps_ratectrl->d_picture_accumulated_bits_quantizer_over_satd += ps_slice_rc->d_slice_accumulated_bits_quantizer_over_satd;
	ps_ratectrl->i_num_picture_accumulated_bits_quantizer_over_satd += ps_slice_rc->i_num_slice_accumulated_bits_quantizer_over_satd;
	ps_ratectrl->b_picture_bad_encountered = ps_ratectrl->b_picture_bad_encountered || ps_slice_rc->b_slice_bad_encountered;
	ps_ratectrl->i_picture_uncoded_size += ( ps_slice_rc->i_slice_bit_budget + ps_slice_rc->i_slice_bit_budget_extra ) - ps_slice_rc->i_slice_coded_size;
}


int32_t y262_ratectrl_get_slice_mb_quantizer( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc, int32_t i_mb_addr )
{
	int32_t i_quantizer, i_extra_bits, i_baseline, i_adjusted_slice_coded_size;
	double d_quantizer, d_adjusted_quantizer, d_adjustment_weight;
	y262_bitrate_control_t *ps_ratectrl;

	ps_ratectrl = &ps_y262->s_ratectrl;

	d_quantizer = ps_ratectrl->i_quantizer / 256.0;

	if( ps_slice_rc->i_slice_coded_scaled_satd > ( ps_slice_rc->i_slice_scaled_satd / 20 ) ||
		ps_slice_rc->i_slice_coded_size > ( ps_slice_rc->i_slice_bit_budget / 20 ) )
	{
		i_baseline = ( ps_slice_rc->i_slice_num_accumulated_quantizer * 5 );
		i_adjusted_slice_coded_size = ps_slice_rc->i_slice_coded_size - i_baseline;
		i_adjusted_slice_coded_size = MAX( 0, i_adjusted_slice_coded_size );

		i_extra_bits = ( int32_t )( ( ps_slice_rc->i_slice_bit_budget_extra * 0.8 ) * ( ( ( double )ps_slice_rc->i_slice_coded_scaled_satd ) / ps_slice_rc->i_slice_scaled_satd ) );
		i_extra_bits += ( int32_t )( ( ps_slice_rc->i_slice_bit_budget_extra * 0.2 ) * MAX( 0.0,
			( ( ( double )ps_slice_rc->i_slice_coded_scaled_satd - ( ( ps_slice_rc->i_slice_scaled_satd * 4 ) / 5 ) ) / MAX( 1, ( ps_slice_rc->i_slice_scaled_satd - ( ( ps_slice_rc->i_slice_scaled_satd * 4 ) / 5 ) ) ) ) ) );
		i_extra_bits -= i_baseline;

		if( !ps_slice_rc->b_reencode_pass )
		{ 
			if( ( ps_slice_rc->i_slice_bit_budget + i_extra_bits - i_adjusted_slice_coded_size ) > ( 512 + 9 ) )
			{
				d_adjusted_quantizer = ( ( ( ( ps_slice_rc->i_slice_scaled_satd - ps_slice_rc->i_slice_coded_scaled_satd ) * 11 ) / 10 ) * ( ps_slice_rc->d_slice_accumulated_quantizer_bits / MAX( 1, ps_slice_rc->i_slice_coded_scaled_satd ) ) ) /
					( ( ( ( ps_slice_rc->i_slice_bit_budget + i_extra_bits - i_adjusted_slice_coded_size ) - 512 ) * 9 ) / 10 );
				d_adjusted_quantizer += ( d_adjusted_quantizer - d_quantizer ) * 0.1;
			}
			else
			{
				d_adjusted_quantizer = 9000.0;
			}
		}
		else
		{
			int32_t i_adjusted_slice_predicted_size;
			i_adjusted_slice_predicted_size = ps_slice_rc->i_slice_accumulated_predicted_size;
			i_adjusted_slice_predicted_size -= i_baseline;
			i_adjusted_slice_predicted_size = MAX( 0, i_adjusted_slice_predicted_size );

			if( ( ps_slice_rc->i_slice_bit_budget + i_extra_bits - i_adjusted_slice_coded_size ) > ( 512 + 9 ) )
			{
				double d_scale;
				if( i_adjusted_slice_coded_size > 0 && i_adjusted_slice_predicted_size > 0 )
				{
					d_scale = i_adjusted_slice_coded_size / ( double ) i_adjusted_slice_predicted_size;
				}

				d_adjusted_quantizer = d_quantizer * d_scale;
				d_adjusted_quantizer += ( d_adjusted_quantizer - d_quantizer ) * 0.1;
			}
			else
			{
				d_adjusted_quantizer = 9000.0;
			}
		}
		d_adjustment_weight = MIN( 1.0, ( ( double )i_adjusted_slice_coded_size ) / MAX( 1, ( ( ps_slice_rc->i_slice_bit_budget + i_extra_bits ) / 4 ) ) );

		d_adjusted_quantizer = ( d_quantizer * ( 1.0 - d_adjustment_weight ) ) + ( d_adjusted_quantizer * d_adjustment_weight );

		if( d_adjusted_quantizer > ( d_quantizer * 1.5 ) || ( ( d_adjusted_quantizer - d_quantizer ) > 2.0 ) )
		{
			/*fprintf( stderr, "BAD: %d %d ( %d ): %f ( %f )\n", ps_y262->ps_input_picture->i_pon, i_mb_addr, ps_slice_rc->i_slice_coded_scaled_satd, d_adjusted_quantizer - d_quantizer, d_adjusted_quantizer );*/
			ps_slice_rc->b_slice_bad_encountered = TRUE;

		}
		d_quantizer = MAX( d_quantizer, d_adjusted_quantizer );
	}

	d_quantizer = MIN( 31.0, MAX( 0.5, d_quantizer ) ); /* 0.5 because it might be increased by aq */
	i_quantizer = ( int32_t ) ( d_quantizer * 256.0 );

	ps_slice_rc->i_mb_queued_quantizer_f8 = i_quantizer; /* 128-* */
	return i_quantizer;
}

void y262_ratectrl_update_slice_mb( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc, int32_t i_mb_addr, int32_t i_mb_bits )
{
	y262_bitrate_control_t *ps_ratectrl;
	int32_t i_scaled_satd_cost;
	int32_t i_satd, i_quantizer;

	ps_ratectrl = &ps_y262->s_ratectrl;

	i_quantizer = MAX( 256, ps_slice_rc->i_mb_queued_quantizer_f8 ); /* clamp to valid range to not anger 2pass mechanics */
	i_scaled_satd_cost = ps_ratectrl->ps_mb_samples[ i_mb_addr ].i_scaled_satd;
	i_satd = ps_ratectrl->ps_mb_samples[ i_mb_addr ].i_satd;

	ps_slice_rc->i_slice_coded_scaled_satd += i_scaled_satd_cost;
	ps_slice_rc->i_slice_coded_size += i_mb_bits;
	ps_slice_rc->i_slice_accumulated_quantizer += i_quantizer;
	ps_slice_rc->i_slice_num_accumulated_quantizer++;
	ps_slice_rc->d_slice_accumulated_quantizer_bits += ( i_quantizer * MAX( 1, i_mb_bits ) ) / 256.0;
	if( i_satd > 10 )
	{
		ps_slice_rc->d_slice_accumulated_bits_quantizer_over_satd = ( ( double ) ( i_mb_bits * ps_slice_rc->i_mb_queued_quantizer_f8 ) ) / i_satd;
		ps_slice_rc->i_num_slice_accumulated_bits_quantizer_over_satd++;
	}

	ps_ratectrl->ps_mb_samples[ i_mb_addr ].i_quantizer = ps_slice_rc->i_mb_queued_quantizer_f8;
	ps_ratectrl->ps_mb_samples[ i_mb_addr ].i_coded_bits = i_mb_bits;
	ps_slice_rc->i_slice_accumulated_predicted_size += ps_ratectrl->ps_mb_samples[ i_mb_addr ].i_predicted_bits;

}





