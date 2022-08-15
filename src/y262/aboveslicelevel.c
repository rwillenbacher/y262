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


void y262_write_sequence_header( y262_t *ps_context )
{
	int32_t i_idx;
	y262_bitstream_t *ps_bitstream;
	y262_sequence_header_t s_sequence_header;

	ps_bitstream = &ps_context->s_bitstream;

	if( ( ps_context->i_sequence_width >> 4 ) == ( ( ps_context->i_sequence_display_width + 15 ) >> 4 ) )
	{
		s_sequence_header.i_horizontal_size = ps_context->i_sequence_display_width;
	}
	else
	{
		s_sequence_header.i_horizontal_size = ps_context->i_sequence_width;
	}
	if( ps_context->b_progressive_sequence )
	{
		if( ( ps_context->i_sequence_height >> 4 ) == ( ( ps_context->i_sequence_display_height + 15 ) >> 4 ) )
		{
			s_sequence_header.i_vertical_size = ps_context->i_sequence_display_height;
		}
		else
		{
			s_sequence_header.i_vertical_size = ps_context->i_sequence_height;
		}
	}
	else
	{
		if( ( ps_context->i_sequence_height >> 5 ) == ( ( ps_context->i_sequence_display_height + 31 ) >> 5 ) )
		{
			s_sequence_header.i_vertical_size = ps_context->i_sequence_display_height;
		}
		else
		{
			s_sequence_header.i_vertical_size = ps_context->i_sequence_height;
		}
	}
	s_sequence_header.i_aspect_ratio_information = ps_context->i_sequence_aspect_ratio_information;
	s_sequence_header.i_frame_rate_code = ps_context->i_sequence_pulldown_frame_rate_code;
	s_sequence_header.b_marker_bit = 1;
	if( !ps_context->b_sequence_mpeg1 )
	{
		s_sequence_header.i_bit_rate_value = ( ( ps_context->s_ratectrl.i_vbvrate + 399 ) / 400 ) & 0x3ffff;
		s_sequence_header.i_vbv_buffer_size_value = ( ( ps_context->s_ratectrl.i_vbv_size + ( ( 16 * 1024 ) - 1 ) ) / ( 16 * 1024 ) ) & 0x3ff;
		s_sequence_header.b_constrained_parameters_flag = FALSE;
	}
	else
	{
		/* MPEG1__ FIXME: bitrate and constrained parameters flag */

		if( ps_context->b_sequence_cbr )
		{
			s_sequence_header.i_bit_rate_value = ( ( ps_context->s_ratectrl.i_vbvrate + 399 ) / 400 ) & 0x3ffff;
		}
		else
		{
			s_sequence_header.i_bit_rate_value = 0x3ffff;
		}
		s_sequence_header.i_vbv_buffer_size_value = ( ( ps_context->s_ratectrl.i_vbv_size + ( ( 16 * 1024 ) - 1 ) ) / ( 16 * 1024 ) ) & 0x3ff;
		s_sequence_header.b_constrained_parameters_flag = FALSE;
	}

	if( memcmp( ps_context->rgui8_intra_quantiser_matrix, rgui8_y262_default_intra_matrix, 64 * sizeof( uint8_t ) ) != 0 )
	{
		s_sequence_header.b_load_intra_quantiser_matrix = TRUE;
		memcpy( s_sequence_header.rgui8_intra_quantiser_matrix, ps_context->rgui8_intra_quantiser_matrix, 64 * sizeof( uint8_t ) );
	}
	else
	{
		s_sequence_header.b_load_intra_quantiser_matrix = FALSE;
	}

	if( memcmp( ps_context->rgui8_non_intra_quantiser_matrix, rgui8_y262_default_non_intra_matrix, 64 * sizeof( uint8_t ) ) != 0 )
	{
		s_sequence_header.b_load_non_intra_quantiser_matrix = TRUE;
		memcpy( s_sequence_header.rgui8_non_intra_quantiser_matrix, ps_context->rgui8_non_intra_quantiser_matrix, 64 * sizeof( uint8_t ) );
	}
	else
	{
		s_sequence_header.b_load_non_intra_quantiser_matrix = FALSE;
	}

	y262_bitstream_write( ps_bitstream, 1, 24 );
	y262_bitstream_write( ps_bitstream, STARTCODE_SEQUENCE_HEADER, 8 );

	y262_bitstream_write( ps_bitstream, s_sequence_header.i_horizontal_size, 12 );
	y262_bitstream_write( ps_bitstream, s_sequence_header.i_vertical_size, 12 );
	y262_bitstream_write( ps_bitstream, s_sequence_header.i_aspect_ratio_information, 4 );
	y262_bitstream_write( ps_bitstream, s_sequence_header.i_frame_rate_code, 4 );
	y262_bitstream_write( ps_bitstream, s_sequence_header.i_bit_rate_value, 18 );
	y262_bitstream_write( ps_bitstream, s_sequence_header.b_marker_bit, 1 );
	y262_bitstream_write( ps_bitstream, s_sequence_header.i_vbv_buffer_size_value, 10 );
	y262_bitstream_write( ps_bitstream, s_sequence_header.b_constrained_parameters_flag, 1 );

	y262_bitstream_write( ps_bitstream, s_sequence_header.b_load_intra_quantiser_matrix, 1 );
	if( s_sequence_header.b_load_intra_quantiser_matrix )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			y262_bitstream_write( ps_bitstream, s_sequence_header.rgui8_intra_quantiser_matrix[ i_idx ], 8 );
		}
	}

	y262_bitstream_write( ps_bitstream, s_sequence_header.b_load_non_intra_quantiser_matrix, 1 );
	if( s_sequence_header.b_load_non_intra_quantiser_matrix )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			y262_bitstream_write( ps_bitstream, s_sequence_header.rgui8_non_intra_quantiser_matrix[ i_idx ], 8 );
		}
	}

}



void y262_write_sequence_extension( y262_t *ps_context )
{
	y262_bitstream_t *ps_bitstream;
	y262_sequence_extension_t s_sequence_extension;

	ps_bitstream = &ps_context->s_bitstream;

	s_sequence_extension.i_profile_and_level_indication = ( ps_context->i_derived_profile << 4 ) | ps_context->i_derived_level;
	s_sequence_extension.b_progressive_sequence = ps_context->b_progressive_sequence;
	switch( ps_context->i_sequence_chroma_format )
	{
		default:
		case Y262_CHROMA_FORMAT_420:
			s_sequence_extension.i_chroma_format = H262_CHROMA_FORMAT_420;
			break;
		case Y262_CHROMA_FORMAT_422:
			s_sequence_extension.i_chroma_format = H262_CHROMA_FORMAT_422;
			break;
		case Y262_CHROMA_FORMAT_444:
			s_sequence_extension.i_chroma_format = H262_CHROMA_FORMAT_444;
			break;

	}
	s_sequence_extension.i_horizontal_size_extension = 0;
	s_sequence_extension.i_vertical_size_extension = 0;
	s_sequence_extension.i_bit_rate_extension = ( ( ps_context->s_ratectrl.i_vbvrate + 399 ) / 400 ) >> 18;
	s_sequence_extension.b_marker_bit = TRUE;
	s_sequence_extension.i_vbv_buffer_size_extension = ( ( ps_context->s_ratectrl.i_vbv_size + ( ( 16 * 1024 ) - 1 ) ) / ( 16 * 1024 ) ) >> 10;
	s_sequence_extension.b_low_delay = 0;
	s_sequence_extension.i_frame_rate_extension_n = ps_context->i_sequence_frame_rate_extension_n;
	s_sequence_extension.i_frame_rate_extension_d = ps_context->i_sequence_frame_rate_extension_d;

	y262_bitstream_write( ps_bitstream, 1, 24 );
	y262_bitstream_write( ps_bitstream, STARTCODE_EXTENSION, 8 );
	y262_bitstream_write( ps_bitstream, H262_SEQUENCE_EXTENSION_ID, 4 );

	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_profile_and_level_indication, 8 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.b_progressive_sequence, 1 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_chroma_format, 2 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_horizontal_size_extension, 2 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_vertical_size_extension, 2 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_bit_rate_extension, 12 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.b_marker_bit, 1 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_vbv_buffer_size_extension, 8 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.b_low_delay, 1 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_frame_rate_extension_n, 2 );
	y262_bitstream_write( ps_bitstream, s_sequence_extension.i_frame_rate_extension_d, 5 );

}


#define Y262_VIDEOFORMAT_PAL     0
#define Y262_VIDEOFORMAT_NTSC    1
#define Y262_VIDEOFORMAT_SECAM   2
#define Y262_VIDEOFORMAT_709     3
#define Y262_VIDEOFORMAT_UNKNOWN 4

void y262_write_sequence_display_extension( y262_t *ps_context )
{
	int32_t i_video_format;
	int32_t rgi_video_format[ 5 ] = { 1, 2, 3, 5, 5 };
	int32_t rgb_colour_description_present[ 5 ] = { 1, 1, 1, 1, 0 };
	int32_t rgi_colour_primaries[ 5 ] = { 5, 4, 5, 1, 0 };
	int32_t rgi_transfer_characteristics[ 5 ] = { 4, 6, 4, 1, 0 };
	int32_t rgi_matrix_coefficients[ 5 ] = { 5, 6, 5, 1, 0 };
	y262_bitstream_t *ps_bitstream;
	y262_sequence_display_extension_t s_sequence_display_extension;

	ps_bitstream = &ps_context->s_bitstream;
	
	i_video_format = ps_context->i_sequence_video_format;
	if( i_video_format < Y262_VIDEOFORMAT_PAL || i_video_format > Y262_VIDEOFORMAT_UNKNOWN )
	{
		i_video_format = Y262_VIDEOFORMAT_UNKNOWN;
	}

	s_sequence_display_extension.i_video_format = rgi_video_format[ i_video_format ];
	s_sequence_display_extension.b_colour_description = rgb_colour_description_present[ i_video_format ];
	s_sequence_display_extension.s_colour_description.i_colour_primaries = rgi_colour_primaries[ i_video_format ];
	s_sequence_display_extension.s_colour_description.i_transfer_characteristics = rgi_transfer_characteristics[ i_video_format ];
	s_sequence_display_extension.s_colour_description.i_matrix_coefficients = rgi_matrix_coefficients[ i_video_format ];
	s_sequence_display_extension.i_display_horizontal_size = ps_context->i_sequence_display_width;
	s_sequence_display_extension.b_marker_bit = TRUE;
	s_sequence_display_extension.i_display_vertical_size = ps_context->i_sequence_display_height;

	y262_bitstream_write( ps_bitstream, 1, 24 );
	y262_bitstream_write( ps_bitstream, STARTCODE_EXTENSION, 8 );
	y262_bitstream_write( ps_bitstream, H262_SEQUENCE_DISPLAY_EXTENSION_ID, 4 );

	y262_bitstream_write( ps_bitstream, s_sequence_display_extension.i_video_format, 3 );
	y262_bitstream_write( ps_bitstream, s_sequence_display_extension.b_colour_description, 1 );
	if( s_sequence_display_extension.b_colour_description )
	{
		y262_bitstream_write( ps_bitstream, s_sequence_display_extension.s_colour_description.i_colour_primaries, 8 );
		y262_bitstream_write( ps_bitstream, s_sequence_display_extension.s_colour_description.i_transfer_characteristics, 8 );
		y262_bitstream_write( ps_bitstream, s_sequence_display_extension.s_colour_description.i_matrix_coefficients, 8 );
	}
	y262_bitstream_write( ps_bitstream, s_sequence_display_extension.i_display_horizontal_size, 14 );
	y262_bitstream_write( ps_bitstream, s_sequence_display_extension.b_marker_bit, 1 );
	y262_bitstream_write( ps_bitstream, s_sequence_display_extension.i_display_vertical_size, 14 );

}

void y262_write_group_of_pictures_header( y262_t *ps_context )
{
	y262_bitstream_t *ps_bitstream;
	y262_group_of_pictures_header_t s_gop;
	bool_t b_drop_frame_flag;
	int64_t i64_ticks_per_unit, i64_ticks;
	int32_t i_hours, i_minutes, i_seconds, i_frames;

	ps_bitstream = &ps_context->s_bitstream;

	s_gop.i_time_code = 0;
	s_gop.b_closed_gop = !!ps_context->b_closed_gop;
	s_gop.b_broken_link = FALSE;

	y262_bitstream_write( ps_bitstream, 1, 24 );
	y262_bitstream_write( ps_bitstream, STARTCODE_GROUP, 8 );

	/* fixme: need next picture with temporal reference 0, is this here correct ? */
	i64_ticks = ps_context->s_ratectrl.i64_output_seconds;

	i64_ticks_per_unit = 3600LL;
	i_hours = ( int32_t )( i64_ticks / i64_ticks_per_unit );
	i64_ticks = i64_ticks % i64_ticks_per_unit;
	i_hours = i_hours % 24;

	i64_ticks_per_unit = 60LL;
	i_minutes = ( int32_t )( i64_ticks / i64_ticks_per_unit );
	i64_ticks = i64_ticks % i64_ticks_per_unit;

	i64_ticks_per_unit = 1;
	i_seconds = ( int32_t ) (i64_ticks / i64_ticks_per_unit );
	i64_ticks = i64_ticks % i64_ticks_per_unit;

	i_frames = ( int32_t ) ps_context->s_ratectrl.i64_output_frames;
	b_drop_frame_flag = FALSE; /* FIXME */

	y262_bitstream_write( ps_bitstream, b_drop_frame_flag, 1 );
	y262_bitstream_write( ps_bitstream, i_hours, 5 );
	y262_bitstream_write( ps_bitstream, i_minutes, 6 );
	y262_bitstream_write( ps_bitstream, 1, 1 ); /* marker */
	y262_bitstream_write( ps_bitstream, i_seconds, 6 );
	y262_bitstream_write( ps_bitstream, i_frames, 6 );

	y262_bitstream_write( ps_bitstream, s_gop.b_closed_gop, 1 );
	y262_bitstream_write( ps_bitstream, s_gop.b_broken_link, 1 );

}

void y262_write_picture_header( y262_t *ps_context, int32_t i_picture_coding_type )
{
	y262_bitstream_t *ps_bitstream;
	y262_picture_header_t s_picture_header;

	ps_bitstream = &ps_context->s_bitstream;

	s_picture_header.i_temporal_reference = ps_context->ps_input_picture->i_temporal_reference;
	s_picture_header.i_picture_coding_type = i_picture_coding_type;
	s_picture_header.i_vbv_delay = ps_context->ps_input_picture->i_vbv_delay & 0xffff;
	if( s_picture_header.i_picture_coding_type == PICTURE_CODING_TYPE_P ||
		s_picture_header.i_picture_coding_type == PICTURE_CODING_TYPE_B )
	{
		s_picture_header.b_full_pel_forward_vector = FALSE;
		if( ps_context->b_sequence_mpeg1 )
		{
			s_picture_header.i_forward_f_code = ps_context->rgi_fcode[ PICTURE_CODING_FORWARD ][ PICTURE_CODING_HORIZONTAL ];
		}
		else
		{
			s_picture_header.i_forward_f_code = 0x7;
		}
	}	
	if( s_picture_header.i_picture_coding_type == PICTURE_CODING_TYPE_B )
	{
		s_picture_header.b_full_pel_backward_vector = FALSE;
		if( ps_context->b_sequence_mpeg1 )
		{
			s_picture_header.i_backward_f_code = ps_context->rgi_fcode[ PICTURE_CODING_BACKWARD ][ PICTURE_CODING_HORIZONTAL ];
		}
		else
		{
			s_picture_header.i_backward_f_code = 0x7;
		}
	}

	s_picture_header.b_extra_bit_picture = FALSE;
	assert( s_picture_header.b_extra_bit_picture == FALSE );

	y262_bitstream_write( ps_bitstream, 1, 24 );
	y262_bitstream_write( ps_bitstream, STARTCODE_PICTURE, 8 );

	y262_bitstream_write( ps_bitstream, s_picture_header.i_temporal_reference, 10 );
	y262_bitstream_write( ps_bitstream, s_picture_header.i_picture_coding_type, 3 );
	y262_bitstream_write( ps_bitstream, s_picture_header.i_vbv_delay, 16 );
	if( s_picture_header.i_picture_coding_type == PICTURE_CODING_TYPE_P ||
		s_picture_header.i_picture_coding_type == PICTURE_CODING_TYPE_B )
	{
		y262_bitstream_write( ps_bitstream, s_picture_header.b_full_pel_forward_vector, 1 );
		y262_bitstream_write( ps_bitstream, s_picture_header.i_forward_f_code, 3 );
	}
	if( s_picture_header.i_picture_coding_type == PICTURE_CODING_TYPE_B )
	{
		y262_bitstream_write( ps_bitstream, s_picture_header.b_full_pel_backward_vector, 1 );
		y262_bitstream_write( ps_bitstream, s_picture_header.i_backward_f_code, 3 );
	}

	y262_bitstream_write( ps_bitstream, s_picture_header.b_extra_bit_picture, 1 );
	assert( s_picture_header.b_extra_bit_picture == FALSE );
}

void y262_write_picture_coding_extension( y262_t *ps_context )
{
	y262_bitstream_t *ps_bitstream;
	y262_picture_coding_extension_t s_picture_coding_extension;

	ps_bitstream = &ps_context->s_bitstream;

	memset( &s_picture_coding_extension, 0, sizeof( s_picture_coding_extension ) );

	s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_FORWARD ][ PICTURE_CODING_HORIZONTAL ] = ps_context->rgi_fcode[ PICTURE_CODING_FORWARD ][ PICTURE_CODING_HORIZONTAL ];
	s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_FORWARD ][ PICTURE_CODING_VERTICAL ]    = ps_context->rgi_fcode[ PICTURE_CODING_FORWARD ][ PICTURE_CODING_VERTICAL ];
	s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_BACKWARD ][ PICTURE_CODING_HORIZONTAL ] = ps_context->rgi_fcode[ PICTURE_CODING_BACKWARD ][ PICTURE_CODING_HORIZONTAL ];
	s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_BACKWARD ][ PICTURE_CODING_VERTICAL ]   = ps_context->rgi_fcode[ PICTURE_CODING_BACKWARD ][ PICTURE_CODING_VERTICAL ];
	s_picture_coding_extension.i_intra_dc_precision = ps_context->i_intra_dc_precision;
	s_picture_coding_extension.i_picture_structure = PICTURE_CODING_STRUCTURE_FRAME;
	s_picture_coding_extension.b_top_field_first = ps_context->ps_input_picture->b_top_field_first;
	s_picture_coding_extension.b_frame_pred_frame_dct = ps_context->b_frame_pred_frame_dct;
	s_picture_coding_extension.b_concealment_motion_vectors = FALSE;
	s_picture_coding_extension.b_q_scale_type = ps_context->b_qscale_type;
	s_picture_coding_extension.b_intra_vlc_format = ps_context->b_intra_vlc_format;
	s_picture_coding_extension.b_alternate_scan = 0;
	s_picture_coding_extension.b_repeat_first_field = ps_context->ps_input_picture->b_repeat_first_field;
	if( ps_context->i_sequence_chroma_format == Y262_CHROMA_FORMAT_420 )
	{
		s_picture_coding_extension.b_chroma_420_type = ps_context->ps_input_picture->b_progressive_frame;
	}
	else
	{
		s_picture_coding_extension.b_chroma_420_type = FALSE;
	}
	s_picture_coding_extension.b_progressive_frame = ps_context->ps_input_picture->b_progressive_frame;
	s_picture_coding_extension.b_composite_display_flag = FALSE;

	assert( s_picture_coding_extension.b_composite_display_flag == FALSE );

	y262_bitstream_write( ps_bitstream, 1, 24 );
	y262_bitstream_write( ps_bitstream, STARTCODE_EXTENSION, 8 );
	y262_bitstream_write( ps_bitstream, H262_PICTURE_CODING_EXTENSION_ID, 4 );

	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_FORWARD ][ PICTURE_CODING_HORIZONTAL ], 4 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_FORWARD ][ PICTURE_CODING_VERTICAL ], 4 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_BACKWARD ][ PICTURE_CODING_HORIZONTAL ], 4 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.rgi_f_code[ PICTURE_CODING_BACKWARD ][ PICTURE_CODING_VERTICAL ], 4 );

	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.i_intra_dc_precision, 2 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.i_picture_structure, 2 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_top_field_first, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_frame_pred_frame_dct, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_concealment_motion_vectors, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_q_scale_type, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_intra_vlc_format, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_alternate_scan, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_repeat_first_field, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_chroma_420_type, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_progressive_frame, 1 );
	y262_bitstream_write( ps_bitstream, s_picture_coding_extension.b_composite_display_flag, 1 );

	if( s_picture_coding_extension.b_composite_display_flag )
	{
		y262_bitstream_write( ps_bitstream, s_picture_coding_extension.s_composite_display.b_v_axis, 1 );
		y262_bitstream_write( ps_bitstream, s_picture_coding_extension.s_composite_display.i_field_sequence, 3 );
		y262_bitstream_write( ps_bitstream, s_picture_coding_extension.s_composite_display.b_sub_carrier, 1 );
		y262_bitstream_write( ps_bitstream, s_picture_coding_extension.s_composite_display.i_burst_amplitude, 7 );
		y262_bitstream_write( ps_bitstream, s_picture_coding_extension.s_composite_display.i_sub_carrier_phase, 8 );
	}

}

void y262_write_user_data( y262_t *ps_context, int32_t i_which )
{
	y262_bitstream_t *ps_bitstream;
	y262_user_data_t *ps_ud = ps_context->ps_input_picture->rgps_user_data[ i_which ];
	int32_t i_idx;

	ps_bitstream = &ps_context->s_bitstream;

	y262_bitstream_write( ps_bitstream, 1, 24 );
	y262_bitstream_write( ps_bitstream, STARTCODE_USER_DATA, 8 );

	for( i_idx = 0; i_idx < ps_ud->i_len; i_idx++ )
	{
		y262_bitstream_write( ps_bitstream, ps_ud->rgui8_user_data[ i_idx ], 8 );
	}
}


void y262_write_zero_stuffing( y262_t *ps_context, int32_t i_num_bytes )
{
	y262_bitstream_t *ps_bitstream;
	int32_t i_idx;

	ps_bitstream = &ps_context->s_bitstream;

	for( i_idx = 0; i_idx < i_num_bytes; i_idx++ )
	{
		y262_bitstream_write( ps_bitstream, 0, 8 );
	}
}


