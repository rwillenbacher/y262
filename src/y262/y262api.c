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

void *y262_alloc( size_t s_size )
{
	size_t s_offs;
	void *p_allocated, **p_ptr;

	p_allocated = malloc( s_size + 16 + sizeof( void * ) );

	if( p_allocated == NULL )
	{
		return NULL;
	}

	s_offs = ( 16 - ( ( ( ( size_t ) p_allocated ) + sizeof( void * ) ) & 0xf ) );
	if( s_offs < sizeof( void * ) )
	{
		s_offs += 16;
	}
	p_ptr = ( void ** )( ( unsigned char * )p_allocated + s_offs );
	*p_ptr = p_allocated;

	return p_ptr + 1;
}

void y262_dealloc( void *p_ptr )
{
	if( p_ptr )
	{
		void *p_memory;

		p_memory = *( ( ( void ** )( p_ptr ) ) - 1 );
		free( p_memory );
	}
}


void *y262_create( y262_configuration_t *ps_config )
{
	y262_t *ps_y262;

	if( !ps_config )
	{
		return NULL;
	}


	memset( ps_config, 0, sizeof( y262_configuration_t ) );
	ps_config->i_multithreading = 0;
	ps_config->i_num_threads = 1;
	ps_config->b_interlaced = FALSE;
	ps_config->b_top_field_first = 1;
	ps_config->i_profile = Y262_PROFILE_DERIVE;
	ps_config->i_level = Y262_LEVEL_DERIVE;
	ps_config->i_coded_chroma_format = Y262_CHROMA_FORMAT_420;
	ps_config->rgi_fcode[ 0 ] = 5;
	ps_config->rgi_fcode[ 1 ] = 5;
	ps_config->i_coded_width = 352;
	ps_config->i_coded_height = 288;
	ps_config->i_display_width = 352;
	ps_config->i_display_height = 288;
	ps_config->i_vbv_size = 400;
	ps_config->i_vbv_rate = 800;
	ps_config->i_bitrate = 400;
	ps_config->i_frame_rate_code = 1;
	ps_config->i_aspect_ratio_information = 1;
	ps_config->i_quantizer = 2;
	ps_config->i_rcmode = 0;
	ps_config->i_bframes = 2;
	ps_config->i_keyframe_ref_distance = 4;
	ps_config->b_closed_gop = FALSE;
	ps_config->i_lookahead_pictures = 40;
	ps_config->b_variance_aq = TRUE;
	ps_config->i_psyrd_strength = 0;
	ps_config->b_qscale_type = 1;
	ps_config->b_mpeg1 = FALSE;
	ps_config->b_cbr_padding = FALSE;

	ps_y262 = ( y262_t * )y262_alloc( sizeof( y262_t ) );

	memset( ps_y262, 0, sizeof( y262_t ) );

	return ( void *)ps_y262;
}


bool_t y262_validate_level( y262_t *ps_y262, int32_t i_level, bool_t b_forderive )
{
	int32_t i_idx, i_luma_sample_rate;
	int32_t rgi_max_hfcode[ 4 ] = { 7, 8, 9, 9 };
	int32_t rgi_max_vfcode[ 4 ] = { 4, 5, 5, 5 };
	int32_t rgi_max_frcode[ 4 ] = { 5, 5, 8, 8 };
	int32_t rgi_max_pic_width[ 4 ] = { 352, 720, 1440, 1920 };
	int32_t rgi_max_pic_height[ 4 ] = { 288, 576, 1152, 1152 };
	int32_t rgi_fr[ 16 ] = { 100, 24, 24, 25, 30, 30, 50, 60, 60, 100, 100, 100, 100, 100, 100, 100 };
	int32_t rgi_max_fr[ 4 ] = { 30, 30, 60, 60 };
	int32_t rgi_max_luma_sample_rate[ 4 ] = { 3041280, 10368000, 47001600, 62668800 };
	int32_t rgi_max_bitrate[ 4 ] = { 4000000, 15000000, 60000000, 80000000 };
	int32_t rgi_max_buffer_size[ 4 ] = { 475136, 1835008, 7340032, 9781248 };

	i_idx = -1;
	switch( i_level )
	{
	case Y262_LEVEL_LOW:
		i_idx = 0;
		break;
	case Y262_LEVEL_MAIN:
		i_idx = 1;
		break;
	case Y262_LEVEL_HIGH1440:
		i_idx = 2;
		break;
	case Y262_LEVEL_HIGH:
		i_idx = 3;
		break;
	}

	if( i_idx == -1 )
	{
		return FALSE;
	}
	if( ps_y262->rgi_fcode[ 0 ][ 0 ] < 1 || ps_y262->rgi_fcode[ 0 ][ 0 ] > rgi_max_hfcode[ i_idx ] ||
		ps_y262->rgi_fcode[ 1 ][ 0 ] < 1 || ps_y262->rgi_fcode[ 1 ][ 0 ] > rgi_max_hfcode[ i_idx ] )
	{
		if( !b_forderive && ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_PROFILELEVEL, ( int8_t *)"horizontal fcode exceeds level limit" );
		}
		return FALSE;
	}
	if( ps_y262->rgi_fcode[ 0 ][ 1 ] < 1 || ps_y262->rgi_fcode[ 0 ][ 1 ] > rgi_max_vfcode[ i_idx ] ||
		ps_y262->rgi_fcode[ 1 ][ 1 ] < 1 || ps_y262->rgi_fcode[ 1 ][ 1 ] > rgi_max_vfcode[ i_idx ] )
	{
		if( !b_forderive && ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_PROFILELEVEL, ( int8_t *)"vertical fcode exceeds level limit" );
		}
		return FALSE;
	}
	if( ps_y262->i_sequence_frame_rate_code < 1 || ps_y262->i_sequence_frame_rate_code > rgi_max_frcode[ i_idx ]  )
	{
		if( !b_forderive && ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_PROFILELEVEL, ( int8_t *)"frame rate exceeds level limit" );
		}
		return FALSE;
	}
	if( ps_y262->i_sequence_width > rgi_max_pic_width[ i_idx ] || 
		ps_y262->i_sequence_height > rgi_max_pic_height[ i_idx ] )
	{
		if( !b_forderive && ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_PROFILELEVEL, ( int8_t *)"picture size exceeds level limit" );
		}
		return FALSE;
	}
	i_luma_sample_rate = ( int32_t )( ( ( ( int64_t )( ps_y262->i_sequence_width * ps_y262->i_sequence_height ) ) * ps_y262->i_sequence_derived_timescale ) / ps_y262->i_sequence_derived_picture_duration );
	if( i_luma_sample_rate > rgi_max_luma_sample_rate[ i_idx ] )
	{
		if( !b_forderive && ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_PROFILELEVEL, ( int8_t *)"luma sample rate exceeds level limit" );
		}
		return FALSE;
	}
	if( ps_y262->s_ratectrl.i_vbvrate > rgi_max_bitrate[ i_idx ] )
	{
		if( !b_forderive && ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_PROFILELEVEL, ( int8_t *)"maximum bitrate exceeds level limit" );
		}
		return FALSE;
	}
	if( ps_y262->s_ratectrl.i_vbv_size > rgi_max_buffer_size[ i_idx ] )
	{
		if( !b_forderive && ps_y262->s_funcs.pf_error_callback )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_PROFILELEVEL, ( int8_t *)"video buffer size exceeds level limit" );
		}
		return FALSE;
	}
	return TRUE;
}

int32_t y262_initialize( void *p_y262, y262_configuration_t *ps_config )
{
	int32_t i_idx, i_mv_range, i_mvs, i_fcode_m1, i_wrap;
	y262_t *ps_y262;

	if( p_y262 == NULL )
	{
		return Y262_INIT_ERROR_CONTEXT;
	}

	ps_y262 = ( y262_t * )p_y262;

	ps_y262->p_cb_handle = ps_config->p_cb_handle;
	ps_y262->s_funcs.pf_error_callback = ps_config->pf_error_callback;
	ps_y262->s_funcs.pf_result_callback = ps_config->pf_result_callback;
	ps_y262->s_funcs.pf_rcsample_callback = ps_config->pf_rcsample_callback;

	ps_y262->b_multithreading = !!ps_config->i_multithreading;
	ps_y262->i_num_slice_encoders = MIN( MAX_NUM_SLICE_ENCODERS, MAX( 1, ps_config->i_num_threads ) );
	ps_y262->i_num_lookahead_encoders = MIN( MAX_NUM_SLICE_ENCODERS, MAX( 1, ps_config->i_num_threads ) );

	if( ps_config->i_coded_width < 16 || ps_config->i_coded_width > 4096 ||
		ps_config->i_coded_height < 16 || ps_config->i_coded_height > 4096 )
	{
		return Y262_INIT_ERROR_CODED_SIZE;
	}

	if( ( ps_config->i_coded_width & 0xf ) != 0 )
	{
		return Y262_INIT_ERROR_CODED_SIZE;
	}
	if( ( ps_config->i_coded_height & 0xf ) != 0 )
	{
		return Y262_INIT_ERROR_CODED_SIZE;
	}
	if( ps_config->i_coded_chroma_format < Y262_CHROMA_FORMAT_420 ||
		ps_config->i_coded_chroma_format > Y262_CHROMA_FORMAT_444 )
	{
		return Y262_INIT_ERROR_CHROMA_FORMAT;
	}
	if( ps_config->i_display_width > ps_config->i_coded_width )
	{
		return Y262_INIT_ERROR_DISPLAY_SIZE;
	}
	if( ps_config->i_display_height > ps_config->i_coded_height )
	{
		return Y262_INIT_ERROR_DISPLAY_SIZE;
	}
	if( ps_config->i_frame_rate_code < 1 || ps_config->i_frame_rate_code > 8 )
	{
		return Y262_INIT_ERROR_FRAMERATE;
	}
	if( ps_config->i_pulldown_frame_rate_code != 0 )
	{
		if( ps_config->i_pulldown_frame_rate_code < 1 || ps_config->i_pulldown_frame_rate_code > 8 )
		{
			return Y262_INIT_ERROR_PFRAMERATE;
		}
	}
	if( ps_config->i_aspect_ratio_information < 1 || ps_config->i_aspect_ratio_information > 4 && !ps_config->b_mpeg1 )
	{
		return Y262_INIT_ERROR_ASPECT;
	}
	else if( ps_config->i_aspect_ratio_information < 1 || ps_config->i_aspect_ratio_information > 14 && ps_config->b_mpeg1 )
	{
		return Y262_INIT_ERROR_ASPECT;
	}
	if( ps_config->i_videoformat < 0 || ps_config->i_videoformat > 4 )
	{
		return Y262_INIT_ERROR_VIDEO_FORMAT;
	}
	if( ps_config->i_profile != Y262_PROFILE_DERIVE &&
		ps_config->i_profile != Y262_PROFILE_SIMPLE && 
		ps_config->i_profile != Y262_PROFILE_MAIN )
	{
		return Y262_INIT_ERROR_PROFILE;
	}
	if( ps_config->i_level != Y262_LEVEL_DERIVE &&
		ps_config->i_level != Y262_LEVEL_LOW &&
		ps_config->i_level != Y262_LEVEL_MAIN &&
		ps_config->i_level != Y262_LEVEL_HIGH1440 &&
		ps_config->i_level != Y262_LEVEL_HIGH )
	{
		return Y262_INIT_ERROR_LEVEL;
	}

	if( ps_config->i_lookahead_pictures < 10 || ps_config->i_lookahead_pictures > 50 )
	{
		return Y262_INIT_ERROR_LOOKAHEADPICS;
	}
	if( ps_config->i_keyframe_ref_distance < 0 )
	{
		return Y262_INIT_ERROR_KEYFRAME_DIST;
	}
	if( ps_config->i_bframes < 0 || ps_config->i_bframes > 4 )
	{
		return Y262_INIT_ERROR_BFRAMES_COUNT;
	}
	if( ps_config->i_quality_for_speed < -100 || ps_config->i_quality_for_speed > 100 )
	{
		return Y262_INIT_ERROR_QUALITY_SPEED;
	}
	if( ps_config->i_psyrd_strength > 512 )
	{
		return Y262_INIT_ERROR_PSYRD_STR;
	}
	if( ps_config->i_num_threads < 1 || ps_config->i_num_threads > 8 )
	{
		return Y262_INIT_ERROR_THREADS;
	}

	if( ps_config->i_rcmode < BITRATE_CONTROL_CQ || ps_config->i_rcmode > BITRATE_CONTROL_PASS2 )
	{
		return Y262_INIT_ERROR_RCMODE;
	}
	if( ps_config->i_bitrate < 20 && ps_config->i_rcmode != BITRATE_CONTROL_CQ )
	{
		return Y262_INIT_ERROR_BITRATE;
	}
	if( ps_config->i_vbv_rate < 20 )
	{
		return Y262_INIT_ERROR_VBVRATE;
	}
	if( ps_config->i_vbv_size < 20 )
	{
		return Y262_INIT_ERROR_VBVSIZE;
	}
	if( ps_config->i_quantizer < 1 || ps_config->i_quantizer > 31 )
	{
		return Y262_INIT_ERROR_QUANTIZER;
	}
	ps_y262->s_ratectrl.i_mode = ps_config->i_rcmode;
	ps_y262->s_ratectrl.i_bitrate = ps_config->i_bitrate * 1000;
	ps_y262->s_ratectrl.i_vbvrate = ps_config->i_vbv_rate * 1000;
	ps_y262->s_ratectrl.i_vbv_size = ps_config->i_vbv_size * 1000;
	ps_y262->b_sequence_cbr = ps_config->b_cbr_padding;
	ps_y262->i_quantizer = ps_config->i_quantizer;


	ps_y262->b_sequence_mpeg1 = ps_config->b_mpeg1;
	if( ps_y262->b_sequence_mpeg1 )
	{
		ps_y262->b_progressive_sequence = TRUE;
		ps_y262->i_intra_dc_precision = 0;
	}
	else
	{
		ps_y262->b_progressive_sequence = FALSE;
		ps_y262->i_intra_dc_precision = 1;
	}
	ps_y262->b_frame_pred_frame_dct = !ps_config->b_interlaced; /* interlaced */
	ps_y262->b_qscale_type = !!ps_config->b_qscale_type;
	ps_y262->b_intra_vlc_format = 0;
	ps_y262->i_sequence_width = ps_config->i_coded_width;
	ps_y262->i_sequence_height = ps_config->i_coded_height;
	switch( ps_config->i_coded_chroma_format )
	{
		case Y262_CHROMA_FORMAT_420:
			ps_y262->i_sequence_chroma_width = ps_y262->i_sequence_width >> 1;
			ps_y262->i_sequence_chroma_height = ps_y262->i_sequence_height >> 1;
			break;
		case Y262_CHROMA_FORMAT_422:
			ps_y262->i_sequence_chroma_width = ps_y262->i_sequence_width >> 1;
			ps_y262->i_sequence_chroma_height = ps_y262->i_sequence_height;
			break;
		case Y262_CHROMA_FORMAT_444:
			ps_y262->i_sequence_chroma_width = ps_y262->i_sequence_width;
			ps_y262->i_sequence_chroma_height = ps_y262->i_sequence_height;
			break;
	}
	ps_y262->i_sequence_chroma_format = ps_config->i_coded_chroma_format;
	ps_y262->i_sequence_display_width = ps_config->i_display_width;
	ps_y262->i_sequence_display_height = ps_config->i_display_height;
	ps_y262->i_sequence_video_format = ps_config->i_videoformat;
	ps_y262->i_sequence_frame_rate_code = ps_config->i_frame_rate_code;
	ps_y262->i_sequence_pulldown_frame_rate_code = ps_config->i_pulldown_frame_rate_code ? ps_config->i_pulldown_frame_rate_code : ps_config->i_frame_rate_code;
	ps_y262->i_sequence_frame_rate_extension_n = 0;
	ps_y262->i_sequence_frame_rate_extension_d = 0;
	ps_y262->i_sequence_aspect_ratio_information = ps_config->i_aspect_ratio_information;

	if( ps_y262->b_sequence_mpeg1 )
	{
		if( !ps_y262->b_progressive_sequence )
		{
			return Y262_INIT_ERROR_MPEG1_CONSTRAINT;
		}
		if( ps_y262->i_intra_dc_precision != 0 )
		{
			return Y262_INIT_ERROR_MPEG1_CONSTRAINT;
		}
		if( ps_y262->i_sequence_chroma_format != Y262_CHROMA_FORMAT_420 )
		{
			return Y262_INIT_ERROR_MPEG1_CHROMA_FORMAT;
		}
		if( ps_y262->i_sequence_frame_rate_extension_n != 0 ||
			ps_y262->i_sequence_frame_rate_extension_d != 0 )
		{
			return Y262_INIT_ERROR_MPEG1_CONSTRAINT;
		}
		if( !ps_y262->b_frame_pred_frame_dct )
		{
			return Y262_INIT_ERROR_MPEG1_INTERLACED;
		}
		if( ps_y262->b_qscale_type )
		{
			return Y262_INIT_ERROR_MPEG1_QSCALE;
		}
		if( ps_y262->b_intra_vlc_format )
		{
			return Y262_INIT_ERROR_MPEG1_CONSTRAINT;
		}
		if( ps_config->rgi_fcode[ 0 ] < 1 || ps_config->rgi_fcode[ 0 ] > 7 )
		{
			return Y262_INIT_ERROR_MPEG1_FCODE;
		}
	}

	ps_y262->i_sequence_derived_picture_duration = rgi_y262_framerate_code_duration[ ps_y262->i_sequence_frame_rate_code ];
	ps_y262->i_sequence_derived_picture_duration *= ( 1 + ps_y262->i_sequence_frame_rate_extension_d );
	ps_y262->i_sequence_derived_picture_duration *= 2; /* for repeat first field */
	ps_y262->i_sequence_derived_timescale = rgi_y262_framerate_code_timescale[ ps_y262->i_sequence_frame_rate_code ];
	ps_y262->i_sequence_derived_timescale *= ( 1 + ps_y262->i_sequence_frame_rate_extension_n );
	ps_y262->i_sequence_derived_timescale *= 2;

	ps_y262->i_sequence_derived_pulldown_picture_duration = rgi_y262_framerate_code_duration[ ps_y262->i_sequence_pulldown_frame_rate_code ];
	ps_y262->i_sequence_derived_pulldown_picture_duration *= ( 1 + ps_y262->i_sequence_frame_rate_extension_d );
	ps_y262->i_sequence_derived_pulldown_picture_duration *= 2;
	ps_y262->i_sequence_derived_pulldown_timescale = rgi_y262_framerate_code_timescale[ ps_y262->i_sequence_pulldown_frame_rate_code ];
	ps_y262->i_sequence_derived_pulldown_timescale *= ( 1 + ps_y262->i_sequence_frame_rate_extension_n );
	ps_y262->i_sequence_derived_pulldown_timescale *= 2;

	ps_y262->i_num_lookahead_pictures = MAX( 1, ps_config->i_lookahead_pictures );	/* lookahead */
	ps_y262->i_current_input_pon = 0;
	ps_y262->i_current_input_field = ps_config->b_top_field_first ? 0 : 1;
	ps_y262->i_leading_lookahead_don = 0;
	ps_y262->i_current_lookahead_don = 0;
	ps_y262->i_current_encoder_don = 0;
	ps_y262->i_current_eof_pon = -1;
	ps_y262->i_current_eof_don = -1;
	ps_y262->i_sequence_num_bframes = ps_config->i_bframes;	/* bframes */
	ps_y262->i_sequence_keyframe_distance = ps_config->i_keyframe_ref_distance; /* 0 = I B B I, 1 = I B B P B B I ... */
	ps_y262->b_closed_gop = ps_config->b_closed_gop;
	ps_y262->i_keyframe_countdown = 0;
	ps_y262->i_last_keyframe_temporal_reference = 0;
	if( ps_y262->b_sequence_mpeg1 )
	{
		ps_y262->rgi_fcode[ 0 ][ 0 ] = ps_config->rgi_fcode[ 0 ];
		ps_y262->rgi_fcode[ 0 ][ 1 ] = ps_config->rgi_fcode[ 1 ];
		ps_y262->rgi_fcode[ 1 ][ 0 ] = ps_y262->rgi_fcode[ 0 ][ 0 ];
		ps_y262->rgi_fcode[ 1 ][ 1 ] = ps_y262->rgi_fcode[ 0 ][ 1 ];
	}
	else
	{
		ps_y262->rgi_fcode[ 0 ][ 0 ] = ps_config->rgi_fcode[ 0 ];
		ps_y262->rgi_fcode[ 0 ][ 1 ] = ps_config->rgi_fcode[ 0 ];
		ps_y262->rgi_fcode[ 1 ][ 0 ] = ps_config->rgi_fcode[ 0 ];
		ps_y262->rgi_fcode[ 1 ][ 1 ] = ps_config->rgi_fcode[ 0 ];

	}
	ps_y262->i_max_buffered_input_pictures = ps_y262->i_num_lookahead_pictures + ( ps_y262->i_sequence_num_bframes * 2 ) + 2; /* not + 1 ? */

	ps_y262->b_variance_aq = ps_config->b_variance_aq;
	ps_y262->i_psyrd_strength = ps_config->i_psyrd_strength;
	ps_y262->i_quality_for_speed = ps_config->i_quality_for_speed;


	memcpy( ps_y262->rgui8_intra_quantiser_matrix, rgui8_y262_default_intra_matrix, sizeof( rgui8_y262_default_intra_matrix ) );
	memcpy( ps_y262->rgui8_non_intra_quantiser_matrix, rgui8_y262_default_non_intra_matrix, sizeof( rgui8_y262_default_non_intra_matrix ) );

	if( ps_config->b_non_default_intra_matrix )
	{
		memcpy( ps_y262->rgui8_intra_quantiser_matrix, ps_config->rgui8_non_default_intra_matrix, sizeof( ps_y262->rgui8_intra_quantiser_matrix ) );
	}
	if( ps_config->b_non_default_inter_matrix )
	{
		memcpy( ps_y262->rgui8_non_intra_quantiser_matrix, ps_config->rgui8_non_default_inter_matrix, sizeof( ps_y262->rgui8_non_intra_quantiser_matrix ) );
	}

	memset( ps_y262->rgui16_intra_quantizer_matrices, 0, sizeof( ps_y262->rgui16_intra_quantizer_matrices ) );
	for( i_idx = 1; i_idx < 122; i_idx++ )
	{
		int32_t i_qmat_idx;
		for( i_qmat_idx = 0; i_qmat_idx < 64; i_qmat_idx++ )
		{
			int32_t i_qm, i_qt, i_scale;
			i_qm = ps_y262->rgui8_intra_quantiser_matrix[ i_qmat_idx ];
			i_qt = i_qm * i_idx;
			i_scale = MIN( ( 1 << 16 ) - 1, ( ( 1 << 20 ) + ( i_qt / 2 ) )  / i_qt );
			ps_y262->rgui16_intra_quantizer_matrices[ i_idx ][ i_qmat_idx ] = i_scale;
			if( i_scale > 0 )
			{
				ps_y262->rgui16_intra_quantizer_matrices_bias[ i_idx ][ i_qmat_idx ] = 0x6000 / i_scale;
				ps_y262->rgui16_intra_quantizer_matrices_trellis_bias[ i_idx ][ i_qmat_idx ] = 0x8000 / i_scale;
			}
			else
			{
				ps_y262->rgui16_intra_quantizer_matrices_bias[ i_idx ][ i_qmat_idx ] = 0;
				ps_y262->rgui16_intra_quantizer_matrices_trellis_bias[ i_idx ][ i_qmat_idx ] = 0;
			}
		}
	}

	memset( ps_y262->rgui16_non_intra_quantizer_matrices, 0, sizeof( ps_y262->rgui16_non_intra_quantizer_matrices ) );
	for( i_idx = 1; i_idx < 122; i_idx++ )
	{
		int32_t i_qmat_idx;
		for( i_qmat_idx = 0; i_qmat_idx < 64; i_qmat_idx++ )
		{
			int32_t i_qm, i_qt, i_scale;
			i_qm = ps_y262->rgui8_non_intra_quantiser_matrix[ i_qmat_idx ];
			i_qt = i_qm * i_idx;
			i_scale = MIN( ( 1 << 16 ) - 1, ( ( 1 << 20 ) + ( i_qt / 2 ) )  / i_qt );
			ps_y262->rgui16_non_intra_quantizer_matrices[ i_idx ][ i_qmat_idx ] = i_scale;
		}
	}


	ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x16 ] = y262_sad_16x16;
	ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x8 ] = y262_sad_16x8;
	ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x16 ] = y262_satd_16x16;
	ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x8 ] = y262_satd_16x8;

	ps_y262->s_funcs.f_ssd_16x16 = y262_ssd_16x16;
	ps_y262->s_funcs.f_ssd_8x8 = y262_ssd_8x8;

	ps_y262->s_funcs.f_add_8x8 = y262_add_8x8;
	ps_y262->s_funcs.f_sub_8x8 = y262_sub_8x8;

	ps_y262->s_funcs.f_variance_16x16 = y262_variance_16x16;
	ps_y262->s_funcs.f_variance_8x8 = y262_variance_8x8;

	ps_y262->s_funcs.f_quant8x8_intra_fw = y262_quant8x8_intra_fw_mpeg2;
	ps_y262->s_funcs.f_quant8x8_inter_fw = y262_quant8x8_inter_fw_mpeg2;

	ps_y262->s_funcs.f_fdct_8x8 = y262_fdct_c;
	ps_y262->s_funcs.f_idct_8x8 = y262_idct_c;
#ifdef ASSEMBLY_X86
	if( 1 )
	{	
		ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x16 ] = y262_sad_16x16_sse2;
		ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x8 ] = y262_sad_16x8_sse2;
		ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x16 ] = y262_satd_16x16_sse2;
		ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x8 ] = y262_satd_16x8_sse2;
		
		ps_y262->s_funcs.f_ssd_16x16 = y262_ssd_16x16_sse2;
		ps_y262->s_funcs.f_ssd_8x8 = y262_ssd_8x8_sse2;
		ps_y262->s_funcs.f_add_8x8 = y262_add_8x8_sse2;
		ps_y262->s_funcs.f_sub_8x8 = y262_sub_8x8_sse2;
		ps_y262->s_funcs.f_quant8x8_intra_fw = y262_quant8x8_intra_fw_sse2;
		ps_y262->s_funcs.f_quant8x8_inter_fw = y262_quant8x8_inter_fw_sse2;

		ps_y262->s_funcs.f_fdct_8x8 = y262_fdct_sse2;
		ps_y262->s_funcs.f_idct_8x8 = y262_idct_sse2;
	}
#endif

#ifdef ASSEMBLY_ARM64
	if( 1 )
	{
		ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x16 ] = y262_sad_16x16_neon;
		ps_y262->s_funcs.rgf_sad[ BLOCK_TYPE_16x8 ] = y262_sad_16x8_neon;
		ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x16 ] = y262_satd_16x16_neon;
		ps_y262->s_funcs.rgf_satd[ BLOCK_TYPE_16x8 ] = y262_satd_16x8_neon;

		ps_y262->s_funcs.f_ssd_16x16 = y262_ssd_16x16_neon;
		ps_y262->s_funcs.f_ssd_8x8 = y262_ssd_8x8_neon;
		ps_y262->s_funcs.f_add_8x8 = y262_add_8x8_neon;
		ps_y262->s_funcs.f_sub_8x8 = y262_sub_8x8_neon;
		ps_y262->s_funcs.f_quant8x8_intra_fw = y262_quant8x8_intra_fw_mpeg2_neon;
		ps_y262->s_funcs.f_quant8x8_inter_fw = y262_quant8x8_inter_fw_mpeg2_neon;

		ps_y262->s_funcs.f_fdct_8x8 = y262_fdct_neon;
		ps_y262->s_funcs.f_idct_8x8 = y262_idct_neon;


	}
#endif

	memset( ps_y262->rgi_y262_motion_bits_x, 0, sizeof( ps_y262->rgi_y262_motion_bits_x ) );
	memset( ps_y262->rgi_y262_motion_bits_y, 1, sizeof( ps_y262->rgi_y262_motion_bits_y ) );

	i_fcode_m1 = ps_y262->rgi_fcode[ 0 ][ 0 ] - 1;
	ps_y262->rgi_y262_motion_bits_x[ 2048 ] = 1;
	i_wrap = ( 16 << ( i_fcode_m1 + 1 ) ) + 1;
	for( i_idx = 0; i_idx < 16; i_idx++ )
	{
		i_mvs = rgi_y262_motion_bits[ i_idx + 1 ] + i_fcode_m1;
		for( i_mv_range = ( i_idx << i_fcode_m1 ) + 1; i_mv_range <= ( ( i_idx + 1 ) << i_fcode_m1 ); i_mv_range++ )
		{
			ps_y262->rgi_y262_motion_bits_x[ 2048 + i_mv_range ] = i_mvs;
			ps_y262->rgi_y262_motion_bits_x[ 2048 - i_mv_range ] = i_mvs;
			ps_y262->rgi_y262_motion_bits_x[ 2048 + i_wrap - i_mv_range ] = i_mvs;
			ps_y262->rgi_y262_motion_bits_x[ 2048 - i_wrap + i_mv_range ] = i_mvs;
		}
	}

	i_fcode_m1 = ps_y262->rgi_fcode[ 0 ][ 1 ] - 1;
	ps_y262->rgi_y262_motion_bits_y[ 2048 ] = 1;
	i_wrap = ( 16 << ( i_fcode_m1 + 1 ) ) + 1;
	for( i_idx = 0; i_idx < 16; i_idx++ )
	{
		i_mvs = rgi_y262_motion_bits[ i_idx + 1 ] + i_fcode_m1;
		for( i_mv_range = ( i_idx << i_fcode_m1 ) + 1; i_mv_range <= ( ( i_idx + 1 ) << i_fcode_m1 ); i_mv_range++ )
		{
			ps_y262->rgi_y262_motion_bits_y[ 2048 + i_mv_range ] = i_mvs;
			ps_y262->rgi_y262_motion_bits_y[ 2048 - i_mv_range ] = i_mvs;
			ps_y262->rgi_y262_motion_bits_y[ 2048 + i_wrap - i_mv_range ] = i_mvs;
			ps_y262->rgi_y262_motion_bits_y[ 2048 - i_wrap + i_mv_range ] = i_mvs;
		}
	}
	

	ps_y262->b_next_reference_picture_toggle = FALSE;
	for( i_idx = 0; i_idx < 3; i_idx++ )
	{
		ps_y262->rgs_frame_buffer[ i_idx ].pui8_luma = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * ps_y262->i_sequence_width * ps_y262->i_sequence_height );
		ps_y262->rgs_frame_buffer[ i_idx ].i_stride_luma = ps_y262->i_sequence_width;
		ps_y262->rgs_frame_buffer[ i_idx ].pui8_cb = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * ps_y262->i_sequence_chroma_width * ps_y262->i_sequence_chroma_height );
		ps_y262->rgs_frame_buffer[ i_idx ].pui8_cr = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * ps_y262->i_sequence_chroma_width * ps_y262->i_sequence_chroma_height );
		ps_y262->rgs_frame_buffer[ i_idx ].i_stride_chroma = ps_y262->i_sequence_chroma_width;
	}

	for( i_idx = 0; i_idx < ps_y262->i_max_buffered_input_pictures; i_idx++ )
	{
		int32_t i_lookahead_size;
		int32_t i_user_idx;
		ps_y262->rgs_buffered_input_pictures[ i_idx ].b_used = FALSE;
		ps_y262->rgs_buffered_input_pictures[ i_idx ].pui8_luma = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * ps_y262->i_sequence_width * ps_y262->i_sequence_height );
		ps_y262->rgs_buffered_input_pictures[ i_idx ].pui8_cb = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * ps_y262->i_sequence_chroma_width * ps_y262->i_sequence_chroma_height );
		ps_y262->rgs_buffered_input_pictures[ i_idx ].pui8_cr = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * ps_y262->i_sequence_chroma_width * ps_y262->i_sequence_chroma_height );
		i_lookahead_size = ( ps_y262->i_sequence_width * ps_y262->i_sequence_height ) >> 8;
		ps_y262->rgs_buffered_input_pictures[ i_idx ].ps_lookahead = ( y262_lookahead_mb_t * ) y262_alloc( sizeof( y262_lookahead_mb_t ) * i_lookahead_size );
		for( i_user_idx = 0; i_user_idx < Y262_MAX_NUM_USER_DATA; i_user_idx++ )
		{
			ps_y262->rgs_buffered_input_pictures[ i_idx ].rgps_user_data[ i_user_idx ] = ( y262_user_data_t * ) y262_alloc( sizeof( y262_user_data_t ) );
		}
	}

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		int32_t i_lookahead_size_x = ps_y262->i_sequence_width >> 4;
		int32_t i_lookahead_size_y = ps_y262->i_sequence_height >> 4;
		ps_y262->rgpi_mbtree_references[ i_idx ] = ( int32_t *) y262_alloc( i_lookahead_size_x * i_lookahead_size_y * sizeof( int32_t ) );
	}

	y262_bitstream_init( &ps_y262->s_bitstream, 10000000 );

	y262_init_motion_compensation( ps_y262 );
	y262_ratectrl_init( ps_y262 );

	if( ps_config->i_profile == Y262_PROFILE_DERIVE )
	{
		ps_y262->i_derived_profile = ps_y262->i_sequence_num_bframes > 0 ? Y262_PROFILE_MAIN : Y262_PROFILE_SIMPLE;
	}
	else
	{
		if( ps_config->i_profile == Y262_PROFILE_SIMPLE )
		{
			if( ps_y262->i_sequence_num_bframes > 0 )
			{
				return Y262_INIT_ERROR_PROFILE_BFRAMES_COUNT;
			}
		}
		else if( ps_config->i_profile == Y262_PROFILE_MAIN )
		{
		}
		else
		{
			return Y262_INIT_ERROR_PROFILE;
		}
		ps_y262->i_derived_profile = ps_config->i_profile;
	}
	if( ps_config->i_level == Y262_LEVEL_DERIVE )
	{
		int32_t i_check;
		int32_t rgi_levels[ 4 ] = { Y262_LEVEL_LOW, Y262_LEVEL_MAIN, Y262_LEVEL_HIGH1440, Y262_LEVEL_HIGH };

		for( i_check = 0; i_check < 4; i_check++ )
		{
			if( y262_validate_level( ps_y262, rgi_levels[ i_check ], i_check < 3 ? TRUE : FALSE ) )
			{
				ps_y262->i_derived_level = rgi_levels[ i_check ];
				break;
			}
		}
		if( i_check == 4 )
		{
			return Y262_INIT_ERROR_LEVEL_LIMITS;
		}
	}
	else
	{
		if( !y262_validate_level( ps_y262, ps_config->i_level, FALSE ) )
		{
			return Y262_INIT_ERROR_LEVEL_LIMITS;
		}
		ps_y262->i_derived_level = ps_config->i_level;
	}

	if( ps_y262->b_multithreading )
	{
		ps_y262->b_multithreading = y262_can_do_threads();
	}

	if( ps_y262->b_multithreading )
	{
		ps_y262->p_resource_mutex = y262_create_mutex( ps_y262 );
		if( !ps_y262->p_resource_mutex )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_THREADING, ( int8_t * )"failure to create mutex for multithreading" );
			return Y262_INIT_ERROR_RESOURCE_INTERNAL;
		}
	}

	for( i_idx = 0; i_idx < ps_y262->i_num_slice_encoders; i_idx++ )
	{
		ps_y262->rgps_slice_encoders[ i_idx ] = ( y262_t * ) y262_alloc( sizeof( y262_t ) );
		y262_bitstream_init( &ps_y262->rgps_slice_encoders[ i_idx ]->s_bitstream, 10000000 );
		if( ps_y262->b_multithreading )
		{
			ps_y262->rgs_slice_threads[ i_idx ].i_slice_encoder_idx = i_idx;
			ps_y262->rgs_slice_threads[ i_idx ].ps_y262 = ps_y262;
			ps_y262->rgs_slice_threads[ i_idx ].p_start_event = y262_create_event( ps_y262 );
			ps_y262->rgs_slice_threads[ i_idx ].p_finished_event = y262_create_event( ps_y262 );
			if( ps_y262->rgs_slice_threads[ i_idx ].p_start_event == NULL || ps_y262->rgs_slice_threads[ i_idx ].p_finished_event == NULL )
			{
				ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_THREADING, ( int8_t * )"failure to create events for multithreading" );
				return Y262_INIT_ERROR_RESOURCE_INTERNAL;
			}
			ps_y262->rgs_slice_threads[ i_idx ].p_thread = y262_create_thread( ps_y262, y262_slice_thread, &ps_y262->rgs_slice_threads[ i_idx ] );
			if( ps_y262->rgs_slice_threads[ i_idx ].p_thread == NULL )
			{
				ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_THREADING, ( int8_t * )"failure to spawn slice thread for multithreading" );
				return Y262_INIT_ERROR_RESOURCE_INTERNAL;
			}
		}
	}

	ps_y262->b_lookahead_running = FALSE;
	if( ps_y262->b_multithreading )
	{
		ps_y262->s_lookahead_thread.p_start_event = y262_create_event( ps_y262 );
		ps_y262->s_lookahead_thread.p_finished_event = y262_create_event( ps_y262 );
		if( ps_y262->s_lookahead_thread.p_start_event == NULL || ps_y262->s_lookahead_thread.p_finished_event == NULL )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_THREADING, ( int8_t * )"failure to create events for multithreading" );
			return Y262_INIT_ERROR_RESOURCE_INTERNAL;
		}
		ps_y262->s_lookahead_thread.p_thread = y262_create_thread( ps_y262, y262_main_lookahead_thread, ps_y262 );
		if( ps_y262->s_lookahead_thread.p_thread == NULL )
		{
			ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_THREADING, ( int8_t * )"failure to spawn slice thread for multithreading" );
			return Y262_INIT_ERROR_RESOURCE_INTERNAL;
		}

		for( i_idx = 0; i_idx < ps_y262->i_num_lookahead_encoders; i_idx++ )
		{
			ps_y262->rgs_lookahead_threads[ i_idx ].i_slice_encoder_idx = i_idx;
			ps_y262->rgs_lookahead_threads[ i_idx ].ps_y262 = ps_y262;
			ps_y262->rgs_lookahead_threads[ i_idx ].p_start_event = y262_create_event( ps_y262 );
			ps_y262->rgs_lookahead_threads[ i_idx ].p_finished_event = y262_create_event( ps_y262 );
			if( ps_y262->rgs_lookahead_threads[ i_idx ].p_start_event == NULL || ps_y262->rgs_lookahead_threads[ i_idx ].p_finished_event == NULL )
			{
				ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_THREADING, ( int8_t * )"failure to create events for multithreading" );
				return Y262_INIT_ERROR_RESOURCE_INTERNAL;
			}
			ps_y262->rgs_lookahead_threads[ i_idx ].p_thread = y262_create_thread( ps_y262, y262_lookahead_thread, &ps_y262->rgs_lookahead_threads[ i_idx ] );
			if( ps_y262->rgs_lookahead_threads[ i_idx ].p_thread == NULL )
			{
				ps_y262->s_funcs.pf_error_callback( ps_y262->p_cb_handle, Y262_ERROR_THREADING, ( int8_t * )"failure to spawn slice thread for multithreading" );
				return Y262_INIT_ERROR_RESOURCE_INTERNAL;
			}
		}
	}

	return Y262_INIT_SUCCESS;
}

void y262_deinitialize( void *p_y262 )
{
	int32_t i_idx;
	y262_t *ps_y262;

	if( p_y262 == NULL )
	{
		return;
	}

	ps_y262 = ( y262_t * )p_y262;

	y262_bitstream_deinit( &ps_y262->s_bitstream );

	for( i_idx = 0; i_idx < 3; i_idx++ )
	{
		y262_dealloc( ps_y262->rgs_frame_buffer[ i_idx ].pui8_luma );
		y262_dealloc( ps_y262->rgs_frame_buffer[ i_idx ].pui8_cb );
		y262_dealloc( ps_y262->rgs_frame_buffer[ i_idx ].pui8_cr );
	}

	for( i_idx = 0; i_idx < ps_y262->i_max_buffered_input_pictures; i_idx++ )
	{
		int32_t i_user_idx;
		y262_dealloc( ps_y262->rgs_buffered_input_pictures[ i_idx ].pui8_luma );
		y262_dealloc( ps_y262->rgs_buffered_input_pictures[ i_idx ].pui8_cb );
		y262_dealloc( ps_y262->rgs_buffered_input_pictures[ i_idx ].pui8_cr );
		y262_dealloc( ps_y262->rgs_buffered_input_pictures[ i_idx ].ps_lookahead );
		for( i_user_idx = 0; i_user_idx < Y262_MAX_NUM_USER_DATA; i_user_idx++ )
		{
			y262_dealloc( ps_y262->rgs_buffered_input_pictures[ i_idx ].rgps_user_data[ i_user_idx ] );
		}
	}

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		y262_dealloc( ps_y262->rgpi_mbtree_references[ i_idx ] );
	}

	y262_ratectrl_deinit( ps_y262 );

	if( ps_y262->b_multithreading )
	{
		for( i_idx = 0; i_idx < ps_y262->i_num_slice_encoders; i_idx++ )
		{
			y262_slice_thread_t *ps_slice_thread;
			ps_slice_thread = &ps_y262->rgs_slice_threads[ i_idx ];
			ps_slice_thread->i_command = Y262_SLICE_THREAD_CMD_EXIT;
			y262_event_set_g( ps_y262, ps_slice_thread->p_start_event );
			y262_event_wait_g( ps_y262, ps_slice_thread->p_finished_event );

			y262_join_thread( ps_y262, ps_slice_thread->p_thread );
			y262_destroy_event( ps_y262, ps_slice_thread->p_start_event );
			y262_destroy_event( ps_y262, ps_slice_thread->p_finished_event );
		}
		for( i_idx = 0; i_idx < ps_y262->i_num_lookahead_encoders; i_idx++ )
		{
			y262_slice_thread_t *ps_slice_thread;
			ps_slice_thread = &ps_y262->rgs_lookahead_threads[ i_idx ];
			ps_slice_thread->i_command = Y262_SLICE_THREAD_CMD_EXIT;
			y262_event_set_g( ps_y262, ps_slice_thread->p_start_event );
			y262_event_wait_g( ps_y262, ps_slice_thread->p_finished_event );

			y262_join_thread( ps_y262, ps_slice_thread->p_thread );
			y262_destroy_event( ps_y262, ps_slice_thread->p_start_event );
			y262_destroy_event( ps_y262, ps_slice_thread->p_finished_event );
		}
		ps_y262->s_lookahead_thread.i_command = Y262_SLICE_THREAD_CMD_EXIT;
		y262_event_set_g( ps_y262, ps_y262->s_lookahead_thread.p_start_event );
		y262_event_wait_g( ps_y262, ps_y262->s_lookahead_thread.p_finished_event );

		y262_join_thread( ps_y262, ps_y262->s_lookahead_thread.p_thread );
		y262_destroy_event( ps_y262, ps_y262->s_lookahead_thread.p_start_event );
		y262_destroy_event( ps_y262, ps_y262->s_lookahead_thread.p_finished_event );

		y262_destroy_mutex( ps_y262, ps_y262->p_resource_mutex );
	}

	for( i_idx = 0; i_idx < ps_y262->i_num_slice_encoders; i_idx++ )
	{
		y262_bitstream_deinit( &ps_y262->rgps_slice_encoders[ i_idx ]->s_bitstream );
		y262_dealloc( ps_y262->rgps_slice_encoders[ i_idx ] );
	}

	y262_dealloc( ps_y262 );
}


int32_t y262_push_input_picture( void *p_y262, y262_input_picture_t *ps_picture, int32_t i_pon )
{
	int32_t i_buf_idx, i_idx;
	y262_t *ps_y262;

	if( p_y262 == NULL )
	{
		return Y262_PUSH_INPUT_ERR_ARG;
	}

	ps_y262 = ( y262_t *) p_y262;
	if( ps_y262->i_current_eof_pon >= 0 && ps_picture != NULL )
	{
		return Y262_PUSH_INPUT_ERR_STATE;
	}

	if( ps_picture )
	{
		i_buf_idx = y262_get_free_input_frame_idx( ps_y262 );

		if( ps_y262->b_multithreading )
		{
			y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex );
		}

		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].b_used = TRUE;
		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].i_don = -1;
		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].i_pon = ps_y262->i_current_input_pon++;
		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].i_frame_type = -1;
		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].b_progressive_frame = ps_picture->i_frame_structure != Y262_INPUT_PICTURE_FRAME_INTERLACED;
		if( ps_picture->i_frame_structure != Y262_INPUT_PICTURE_FRAME_INTERLACED )
		{
			ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].b_repeat_first_field = ps_picture->i_frame_structure == Y262_INPUT_PICTURE_FRAME_PROGRESSIVE_REPEAT;
		}
		else
		{
			ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].b_repeat_first_field = FALSE;
		}
		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].b_top_field_first = ( ps_y262->i_current_input_field & 1 ) ? FALSE : TRUE;
		if( ps_picture->i_frame_structure == Y262_INPUT_PICTURE_FRAME_PROGRESSIVE_REPEAT )
		{
			ps_y262->i_current_input_field += 3;
		}
		else
		{
			ps_y262->i_current_input_field += 2;
		}

		if( ps_y262->b_multithreading )
		{
			y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
		}


 		memcpy( ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].pui8_luma, ps_picture->pui8_luma, sizeof( uint8_t ) * ps_y262->i_sequence_width * ps_y262->i_sequence_height );
		memcpy( ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].pui8_cb, ps_picture->pui8_cb, sizeof( uint8_t ) * ps_y262->i_sequence_chroma_width * ps_y262->i_sequence_chroma_height );
		memcpy( ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].pui8_cr, ps_picture->pui8_cr, sizeof( uint8_t ) * ps_y262->i_sequence_chroma_width * ps_y262->i_sequence_chroma_height );

		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].b_force_new_gop = ps_picture->b_start_new_gop;

		ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].i_num_user_data = 0;
		for( i_idx = 0; i_idx < ps_picture->i_num_user_data; i_idx++ )
		{
			int32_t i_len = ps_picture->rgi_user_data_len[ i_idx ];
			memcpy( &ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].rgps_user_data[ i_idx ]->rgui8_user_data[ 0 ], ps_picture->rgpui8_user_data[ i_idx ], i_len * sizeof( uint8_t ) );
			ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].rgps_user_data[ i_idx ]->i_len = i_len;
			ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].rgps_user_data[ i_idx ]->i_where = ps_picture->rgi_user_data_where[ i_idx ];
			ps_y262->rgs_buffered_input_pictures[ i_buf_idx ].i_num_user_data++;
		}
	}
	else
	{
		ps_y262->i_current_eof_pon = ps_y262->i_current_input_pon;
	}

	if( ps_y262->i_current_input_pon > 290 )
	{
		ps_y262->i_current_lookahead_don = ps_y262->i_current_lookahead_don;
	}

	if( ps_y262->b_lookahead_running && !ps_y262->b_flushing )
	{
		if( ( ps_y262->i_current_input_pon > ps_y262->i_current_lookahead_don + ps_y262->i_sequence_num_bframes ) || 
			( ps_y262->i_current_eof_pon >= 0 && ps_y262->i_current_eof_pon <= ps_y262->i_current_lookahead_don + ps_y262->i_sequence_num_bframes ) )
		{
			y262_finish_lookahead( ps_y262 );
			ps_y262->b_lookahead_running = FALSE;
			assert( ps_y262->i_current_lookahead_don == ps_y262->i_leading_lookahead_don );
		}
	}

	if( !ps_y262->b_flushing && (
		( ps_y262->i_current_input_pon == 1 ) ||
		( ps_y262->i_current_input_pon > ps_y262->i_current_lookahead_don + ps_y262->i_sequence_num_bframes ) ||
		( ( ps_y262->i_current_eof_pon >= 0 && ps_y262->i_current_eof_pon <= ps_y262->i_current_lookahead_don + ps_y262->i_sequence_num_bframes &&
		ps_y262->i_leading_lookahead_don < ps_y262->i_current_input_pon ) ) ) )
	{
		if( !ps_y262->b_lookahead_running )
		{
			y262_setup_lookahead_next_and_start_lookahead( ps_y262 );

			ps_y262->b_lookahead_running = TRUE;
			ps_y262->i_current_lookahead_don += ps_y262->i_lookahead_next_ref - ps_y262->i_lookahead_next_pon + 1;
		}
		else
		{
			assert( FALSE );
		}
	}

	if( ps_y262->i_current_eof_pon == ps_y262->i_current_lookahead_don )
	{
		if( ps_y262->b_lookahead_running )
		{
			y262_finish_lookahead( ps_y262 );
		}
		ps_y262->b_lookahead_running = FALSE;
		ps_y262->i_current_eof_don = ps_y262->i_current_lookahead_don;
		ps_y262->b_flushing = TRUE;
	}

	if( ( ps_y262->i_current_lookahead_don - ps_y262->i_current_encoder_don > ps_y262->i_num_lookahead_pictures + ( 1 + ps_y262->i_sequence_num_bframes ) ) )
	{
		y262_picture_t *ps_pic;

		ps_pic = &ps_y262->rgs_buffered_input_pictures[ y262_get_input_frame_don( ps_y262, ps_y262->i_current_encoder_don++ ) ];
		y262_encode_picture( ps_y262, ps_pic, ps_pic->i_frame_type, ps_pic->i_pon );
		if( ps_y262->b_multithreading )
		{
			y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex );
		}
		ps_pic->b_used = FALSE;
		if( ps_y262->b_multithreading )
		{
			y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
		}
	}
	else if( ps_y262->b_flushing )
	{
		if( ps_y262->i_current_encoder_don < ps_y262->i_current_lookahead_don )
		{
			y262_picture_t *ps_pic;

			ps_pic = &ps_y262->rgs_buffered_input_pictures[ y262_get_input_frame_don( ps_y262, ps_y262->i_current_encoder_don++ ) ];
			y262_encode_picture( ps_y262, ps_pic, ps_pic->i_frame_type, ps_pic->i_pon );

			if( ps_y262->b_multithreading )
			{
				y262_mutex_lock( ps_y262, ps_y262->p_resource_mutex );
			}
			ps_pic->b_used = FALSE;
			if( ps_y262->b_multithreading )
			{
				y262_mutex_unlock( ps_y262, ps_y262->p_resource_mutex );
			}
		}
		if(	ps_y262->i_current_encoder_don < ps_y262->i_current_lookahead_don )
		{
			return Y262_PUSH_INPUT_CONTINUE;
		}
		else
		{
			return Y262_PUSH_INPUT_FLUSHED;
		}
	}
	else
	{
		int32_t i;
		i = 0;
	}

	return Y262_PUSH_INPUT_CONTINUE;
}

