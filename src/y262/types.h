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

typedef uint32_t bool_t;

#define TRUE  1
#define FALSE 0

/* main@high max vbv size is 9781248, +3 byte next startcode  */
#define MAX_ELEMENTARY_STREAM_BUFFER  ( ( 9781248 / 8 ) + 3 )

/* 1920 / 16 */
#define MAX_MACROBLOCKS_PER_ROW     120
/* 1152 / 16 */
#define MAX_MACROBLOCK_COLUMN_HEIGHT 72

#define MAX_MACROBLOCKS_PER_PICTURE ( MAX_MACROBLOCKS_PER_ROW * MAX_MACROBLOCK_COLUMN_HEIGHT )
#define MAX_SLICES_PER_PICTURE      ( MAX_MACROBLOCKS_PER_PICTURE )

#define MACROBLOCK_SIZE			     16

/* startcodes */
#define STARTCODE_PICTURE          0x00
#define STARTCODE_SLICE_START      0x01
#define STARTCODE_SLICE_END        0xAF
#define STARTCODE_USER_DATA        0xB2
#define STARTCODE_SEQUENCE_HEADER  0xB3
#define STARTCODE_SEQUENCE_ERROR   0xB4
#define STARTCODE_EXTENSION        0xB5
#define STARTCODE_SEQUENCE_END     0xB7
#define STARTCODE_GROUP            0xB8
#define STARTCODE_STUFFING         0x100 /* not a valid start code */

/* y262 bitstream layer types */

#define H262_STARTCODE_PREFIX           1
#define H262_STARTCODE_PREFIX_LENGTH   24
#define H262_STARTCODE_COMPLETE_BYTE_LENGTH 4

#define H262_CHROMA_FORMAT_420	1
#define H262_CHROMA_FORMAT_422	2
#define H262_CHROMA_FORMAT_444	3

#define MC_BLOCK_16x16 0
#define MC_BLOCK_16x8  1
#define MC_BLOCK_8x16  2
#define MC_BLOCK_8x8   3
#define MC_BLOCK_8x4   4

#define MC_BLOCK_00    0
#define MC_BLOCK_01    1
#define MC_BLOCK_10    2
#define MC_BLOCK_11    3

#define MAX_COST ( 0x7fffffff )

typedef struct {
	uint8_t *pui8_bitstream;
	int32_t i_length;
	
	int32_t i_byte_count;
	int32_t i_next_bit;

	uint8_t *pui8_codeword_ptr;
	uint32_t ui_codeword;
	int32_t  i_codeword_fill;

} y262_bitstream_t;

/* y262 video sequence layer types */

typedef struct {
	int32_t i_horizontal_size; /* 12 bit, in pixels */
	int32_t i_vertical_size;   /* 12 bit, in pixels */

	int32_t i_aspect_ratio_information; /* 4 bit */
	int32_t i_frame_rate_code;          /* 4 bit */
	int32_t i_bit_rate_value;           /* 18 bit */
	bool_t  b_marker_bit;               /* 1 bit */
	int32_t i_vbv_buffer_size_value;    /* 10 bit */
	bool_t  b_constrained_parameters_flag; /* 1 bit */

	bool_t  b_load_intra_quantiser_matrix; /* 1 bit */
	uint8_t rgui8_intra_quantiser_matrix[64];  /* only present if load_intra_quantiser_matrix is 1 */

	bool_t  b_load_non_intra_quantiser_matrix; /* 1 bit */
	uint8_t rgui8_non_intra_quantiser_matrix[64];  /* only present if load_non_intra_quantiser_matrix 1 */

} y262_sequence_header_t;


#define H262_SEQUENCE_EXTENSION_ID				1
typedef struct {
	int32_t i_profile_and_level_indication;	/* 8 bit */
	bool_t  b_progressive_sequence;			/* 1 bit */

	int32_t i_chroma_format;					/* 2 bit */
	int32_t i_horizontal_size_extension;		/* 2 bit */
	int32_t i_vertical_size_extension;		    /* 2 bit */
	int32_t i_bit_rate_extension;				/* 12 bit */
	bool_t  b_marker_bit;						/* 1 bit */
	int32_t i_vbv_buffer_size_extension;		/* 8 bit */
	bool_t  b_low_delay;						/* 1 bit */
	int32_t i_frame_rate_extension_n;			/* 2 bit */
	int32_t i_frame_rate_extension_d;			/* 5 bit */
} y262_sequence_extension_t;


#define H262_SEQUENCE_DISPLAY_EXTENSION_ID		2
typedef struct {
	int32_t i_video_format;					/* 3 bit */
	bool_t  b_colour_description;			/* 1 bit */

	struct {
		int32_t i_colour_primaries; /* 8 bit */
		int32_t i_transfer_characteristics; /* 8 bit */
		int32_t i_matrix_coefficients; /* 8 bit */
	} s_colour_description; /* only present when colour_description is 1 */

	int32_t i_display_horizontal_size;	/* 14 bit */
	bool_t  b_marker_bit;				/* 1 bit */
	int32_t i_display_vertical_size;	/* 14 bit */

} y262_sequence_display_extension_t;


typedef struct
{
	int32_t i_time_code;			/* 25 bit */
	bool_t  b_closed_gop;			/* 1 bit */
	bool_t  b_broken_link;		/* 1 bit */
} y262_group_of_pictures_header_t;


#define PICTURE_CODING_TYPE_I 1
#define PICTURE_CODING_TYPE_P 2
#define PICTURE_CODING_TYPE_B 3

typedef struct
{
	int32_t	i_temporal_reference;	/* 10 bit */
	int32_t	i_picture_coding_type;	/* 3 bit */
	int32_t	i_vbv_delay;			/* 16 bit */
	bool_t	b_full_pel_forward_vector;	/* 1 bit, picture coding type == 2 || 3 */
	int32_t	i_forward_f_code;			/* 3 bit, picture coding type == 2 || 3 */
	bool_t	b_full_pel_backward_vector;	/* 1 bit, picture coding type == 3 */
	int32_t	i_backward_f_code;			/* 3 bit, picture coding type == 3 */
	bool_t	b_extra_bit_picture;		/* 1 bit */
#define MAX_EXTRA_INFORMATION_PICTURE 0x20
	uint8_t	rgui8_extra_information_picture[ MAX_EXTRA_INFORMATION_PICTURE ];

} y262_picture_header_t;

#define H262_PICTURE_CODING_EXTENSION_ID		8

#define PICTURE_CODING_FORWARD    0
#define PICTURE_CODING_BACKWARD   1
#define PICTURE_CODING_HORIZONTAL 0
#define PICTURE_CODING_VERTICAL   1

#define PICTURE_CODING_STRUCTURE_TOP    1
#define PICTURE_CODING_STRUCTURE_BOTTOM 2
#define PICTURE_CODING_STRUCTURE_FRAME  3


typedef struct {
	int32_t rgi_f_code[2][2];				/* 4 bits each */
	int32_t i_intra_dc_precision;			/* 2 bit */
	int32_t i_picture_structure;			/* 2 bit */
	bool_t  b_top_field_first;			/* 1 bit */
	bool_t  b_frame_pred_frame_dct;		/* 1 bit */
	bool_t  b_concealment_motion_vectors;	/* 1 bit */
	bool_t  b_q_scale_type;				/* 1 bit */
	bool_t  b_intra_vlc_format;			/* 1 bit */
	bool_t  b_alternate_scan;				/* 1 bit */
	bool_t  b_repeat_first_field;			/* 1 bit */
	bool_t  b_chroma_420_type;			/* 1 bit */
	bool_t  b_progressive_frame;			/* 1 bit */

	bool_t  b_composite_display_flag;		/* 1 bit */

	struct {
		bool_t  b_v_axis;				/* 1 bit */
		int32_t i_field_sequence;		/* 3 bit */
		bool_t  b_sub_carrier;		/* 1 bit */
		int32_t i_burst_amplitude;	/* 7 bit */
		int32_t i_sub_carrier_phase;	/* 8 bit */
	} s_composite_display;			/* only present when composite_display_flag is 1 */

} y262_picture_coding_extension_t;


#define H262_QUANT_MATRIX_EXTENSION_ID			3
#if 0
typedef struct {
	bool_t  b_load_intra_quantiser_matrix;	/* 1 bit */
	uint8_t rgui8_intra_quantiser_matrix[64];	/* 64 x 8 bit */

	bool_t  b_load_non_intra_quantiser_matrix;	/* 1 bit */
	uint8_t rgui8_non_intra_quantiser_matrix[64];	/* 64 x 8 bit */

	bool_t  b_load_chroma_intra_quantiser_matrix;	/* 1 bit */
	uint8_t rgui8_chroma_intra_quantiser_matrix[64];	/* 64 x 8 bit */

	bool_t  b_load_chroma_non_intra_quantiser_matrix;	/* 1 bit */
	uint8_t rgui8_chroma_non_intra_quantiser_matrix[64];	/* 64 x 8 bit */

} y262_quant_matrix_extension_t;
#endif

#define H262_PICTURE_DISPLAY_EXTENSION_ID		7
#define PICTURE_DISPLAY_EXTENSION_MAX_FRAME_CENTRE_OFFSETS 3

typedef struct {
	int32_t i_num_frame_centre_offsets;
	struct {
		int32_t i_frame_centre_horizontal_offset;	/* 16 bit */
		bool_t  b_marker_bit_1;						/* 1 bit */
		int32_t i_frame_centre_vertical_offset;		/* 16 bit */
		bool_t  b_marker_bit_2;						/* 1 bit */
	} rgs_frame_centre_offsets[3];
} y262_picture_display_extension_t;


#define H262_COPYRIGHT_EXTENSION_ID		4
typedef struct {
	bool_t  b_copyright_flag;		/* 1 bit */
	int32_t i_copyright_identifier; /* 8 bit */
	bool_t  b_original_or_copy;	/* 1 bit */
	int32_t i_reserved;			/* 7 bit */
	bool_t  b_marker_bit_1;		/* 1 bit start code emulation prevention */
	int32_t i_copyright_number_1;	/* 20 bit */
	bool_t  b_marker_bit_2;		/* 1 bit start code emulation prevention */
	int32_t i_copyright_number_2;	/* 22 bit */
	bool_t  b_marker_bit_3;		/* 1 bit */
	int32_t i_copyright_number_3;	/* 22 bit */
} y262_copyright_extension_t;


typedef struct {
	int32_t rgi_mvs[ 2 ][ 2 ];
	int32_t rgi_mv_costs[ 2 ];
	int32_t i_intra_cost;
	int32_t i_best_cost;
	int32_t i_quantizer_scale;
	int32_t i_quantizer_aq_scale;
#define LOOKAHEAD_MODE_INTRA 0
#define LOOKAHEAD_MODE_INTER_FW 1
#define LOOKAHEAD_MODE_INTER_BW 2
	int32_t i_best_mode;
} y262_lookahead_mb_t;


typedef struct
{
	int32_t i_where;
	int32_t i_len;
	uint8_t rgui8_user_data[ Y262_MAX_USER_DATA_SIZE ];
} y262_user_data_t;

typedef struct {
	bool_t b_used;
	uint8_t *pui8_luma;
	uint8_t *pui8_cb;
	uint8_t *pui8_cr;
	int32_t i_pon;
	int32_t i_temporal_reference;
	int32_t i_vbv_delay;
	int32_t i_don;
	int32_t i_frame_type;
	int32_t b_force_new_gop;
	int32_t b_progressive_frame;
	int32_t b_top_field_first;
	int32_t b_repeat_first_field;
	int32_t b_backward_pred_only;

	y262_lookahead_mb_t *ps_lookahead;
	int32_t i_forward_pon;
	int32_t i_backward_pon;
	int32_t i_frame_cost;
	int32_t i_frame_intra_cost;
#define MAX_BITRATE_CONTROL_LOOKAHEAD_PICTURES 40
	int32_t i_num_lookahead_pictures;
	int32_t rgi_lookahead_picture_types[ MAX_BITRATE_CONTROL_LOOKAHEAD_PICTURES ];
	int32_t rgi_lookahead_picture_costs[ MAX_BITRATE_CONTROL_LOOKAHEAD_PICTURES ];

	int32_t i_num_user_data;
#define MAX_NUM_USER_DATA 4
	y262_user_data_t *rgps_user_data[ MAX_NUM_USER_DATA ];
} y262_picture_t;

typedef struct {
	uint8_t *pui8_luma;
	int32_t i_stride_luma;
	uint8_t *pui8_cb;
	uint8_t *pui8_cr;
	int32_t i_stride_chroma;
} y262_reference_picture_t;

typedef struct {
#define MECALL_LOOKAHEAD 0
#define MECALL_MAIN      1
	int32_t i_me_call;
	int32_t i_num_candidates_fp;
	int32_t rgi_candidates_fp[ 10 ][ 2 ];
	int32_t i_lambda;
	int32_t i_pred_mv_x;
	int32_t i_pred_mv_y;

	uint8_t *pui8_blk;
	int32_t i_blk_stride;
#define BLOCK_TYPE_16x16  0
#define BLOCK_TYPE_16x8   1
	int32_t i_blk_type;

	int32_t i_x_offset;
	int32_t i_y_offset;
	int32_t i_min_mv_x, i_min_mv_y;
	int32_t i_max_mv_x, i_max_mv_y;

	int32_t i_ref_width;
	int32_t i_ref_height;
	uint8_t *pui8_ref;
	int32_t i_ref_stride;

	int32_t i_best_mv_x, i_best_mv_y;
	int32_t i_best_mv_sad;
	int32_t i_best_mv_cost;
} y262_me_context_t;

#define VLC_SENTINEL -1

typedef struct {
	int32_t i_code;
	int32_t i_length;
} y262_vlc_t;

#define RUN_LEVEL_END_OF_BLOCK	-1
#define RUN_LEVEL_ESCAPE		-2
#define RUN_LEVEL_INVALID		-3

typedef struct {
	int32_t i_run;
	int32_t i_level;
} y262_run_level_t;

#define Y262_MBMODE_SKIP     0
#define Y262_MBMODE_FW       1
#define Y262_MBMODE_BW       2
#define Y262_MBMODE_BI       3
#define Y262_MBMODE_INTRA    4
#define Y262_MBMODE_FW_IL    5
#define Y262_MBMODE_BW_IL    6
#define Y262_MBMODE_BI_IL    7
#define Y262_MBMODE_INTRA_IL 8

typedef struct {
	int32_t i_x;
	int32_t i_y;
#define Y262_MV_FRAME_FIELD  0
#define Y262_MV_TOP_FIELD    0
#define Y262_MV_BOTTOM_FIELD 1
	int32_t i_field;
	int32_t i_cost;
} y262_mv_t;

typedef struct {
	int32_t i_skip_cost;

	int32_t i_fw_cost;
	y262_mv_t s_fw_mv;

	int32_t i_bw_cost;
	y262_mv_t s_bw_mv;

	int32_t i_bi_cost;
	y262_mv_t s_bi_mv[ 2 ];

	int32_t i_fw_il_cost;
	y262_mv_t s_fw_il_mv[ 2 ];

	int32_t i_bw_il_cost;
	y262_mv_t s_bw_il_mv[ 2 ];

	int32_t i_bi_il_cost;
	y262_mv_t s_bi_il_mv[ 2 ][ 2 ];

	int32_t i_intra_cost;
	int32_t i_intra_il_cost;
} y262_mode_decision_t;


typedef struct {
	int32_t i_mb_x, i_mb_y;
	uint8_t *pui8_src_luma;
	int32_t i_src_luma_stride;
	uint8_t *pui8_src_cb;
	uint8_t *pui8_src_cr;
	int32_t i_src_chroma_stride;

	uint8_t *pui8_dst_luma;
	int32_t i_dst_luma_stride;
	uint8_t *pui8_dst_cb;
	uint8_t *pui8_dst_cr;
	int32_t i_dst_chroma_stride;

	int32_t i_mb_addr;
	int32_t i_quantizer;
	int32_t i_scaled_quantizer;

#define MACROBLOCK_QUANT			1
#define MACROBLOCK_MOTION_FORWARD	2
#define MACROBLOCK_MOTION_BACKWARD	4
#define MACROBLOCK_PATTERN			8
#define MACROBLOCK_INTRA			16
#define MACROBLOCK_INTERLACED		32
#define MACROBLOCK_MOTION_TYPE		64
#define FRAME_MOTION_TYPE_FIELD			1
#define FRAME_MOTION_TYPE_FRAME			2
#define FRAME_MOTION_TYPE_DUAL_PRIME	3
#define FIELD_MOTION_TYPE_FIELD			1
#define FIELD_MOTION_TYPE_16x8			2
#define FIELD_MOTION_TYPE_DUAL_PRIME	3
	int32_t i_macroblock_type;

	ALIGNED( 16 ) uint8_t rgui8_prediction[ 3 ][ 4 ][ 8 * 8 ];
	ALIGNED( 16 ) int16_t rgi16_residual[ 3 ][ 4 ][ 8 * 8 ];

	bool_t rgb_cbp[ 3 ][ 4 ];
	int32_t i_cbp;
	ALIGNED( 16 ) int16_t rgi16_coeffs[ 3 ][ 4 ][ 8 * 8 ];
	y262_mv_t rgs_motion[ 2 ][ 2 ];

#define Y262_LAMBDA_BITS 6
	int32_t i_lambda;
	int32_t i_lambda_sqr;
} y262_macroblock_t;

typedef struct {

	struct {
		int32_t i_slice_vertical_position;
		int32_t i_mb_row;
		int32_t i_quantizer_scale_code;
		bool_t  b_intra_slice_flag;
		bool_t  b_intra_slice;
	} s_slice_header;

	int32_t i_quantizer_f8;
	/* coding */
	y262_macroblock_t s_macroblock;
	y262_mode_decision_t s_mode_decision;
	int32_t i_quantizer;
	int32_t i_picture_type;
	int32_t i_start_mb_addr;
	int32_t i_end_mb_addr;
	int32_t i_skip_run;
	int32_t i_mb_addr;
	int32_t rgi_dc_dct_pred[ 3 ];
	int32_t rgi_pmv[ 2 ][ 2 ][ 2 ];
	bool_t b_allow_skip;
	int32_t i_last_mb_motion_flags;

} y262_slice_t;


typedef int32_t ( *y262_costfunction_f ) ( uint8_t *pui8_blk1, int32_t i_stride1, uint8_t *pui8_blk2, int32_t i_stride2 );
typedef void ( *y262_motcomp_f ) ( uint8_t *pui8_src, int32_t i_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
typedef void ( y262_thread_f ) ( void *p_arg );


typedef struct {
	y262_costfunction_f rgf_sad[ 2 ];
	y262_costfunction_f rgf_satd[ 2 ];
	y262_costfunction_f f_ssd_16x16;
	y262_costfunction_f f_ssd_8x8;

	int32_t ( *f_variance_16x16 ) ( uint8_t *pui8_blk, int32_t i_blk_stride );
	int32_t ( *f_variance_8x8 ) ( uint8_t *pui8_blk, int32_t i_blk_stride );

	void ( *f_sub_8x8 ) ( int16_t *pi16_diff, uint8_t *pui8_src1, int32_t i_stride_src1, uint8_t *pui8_src2, int32_t i_stride_src2 );
	void ( *f_add_8x8 ) ( uint8_t *pui8_destination, int32_t i_destination_stride, uint8_t *pui8_base, int32_t i_base_stride, int16_t *pi_difference );

	int32_t ( *f_quant8x8_inter_fw ) ( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat );
	int32_t ( *f_quant8x8_intra_fw ) ( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias );

	void ( *f_fdct_8x8 ) ( int16_t *pi16_source, int16_t *pi16_destination );
	void ( *f_idct_8x8 ) ( int16_t *pi16_source, int16_t *pi16_destination );


	pf_error_callback_t pf_error_callback;
	pf_result_callback_t pf_result_callback;
	pf_rcsample_callback_t pf_rcsample_callback;

	y262_motcomp_f rgf_motcomp_copy[ 5 ][ 4 ];
	y262_motcomp_f rgf_motcomp_avg[ 5 ][ 4 ];

} y262_function_toolbox_t;

typedef struct {
	int32_t i_bits;
	double d_quantizer;
	double d_cplx;
	int32_t i_estimated_bits;
	uint8_t ui8_frame_type;
	int32_t i_satd_cost;
} y262_ratectrl_isample_t;

typedef struct {
	int32_t i_quantizer_f8;
	int32_t i_bits;
	int32_t i_cplx_f8;
	int32_t i_estimated_bits;
	uint8_t ui8_frame_type;
	int32_t i_satd_cost;
} y262_ratectrl_sample_t;

typedef struct
{
	int32_t i_satd;
	int32_t i_scaled_satd;
	int32_t i_quantizer;
	int32_t i_coded_bits;
	int32_t i_predicted_bits;
} y262_ratectrl_mb_sample_t;

typedef struct {
	int32_t i_mode;
	int32_t i_bitrate;
	int32_t i_vbvrate;
	int32_t i_vbv_size;
	int32_t i_vbv_occupancy;
	int64_t i64_vbv_occupancy_fractional;
	int32_t i_vbv_incoming;
	int32_t i_vbv_outgoing;
	int32_t i_vbv_occupancy_overflow;
	int64_t i64_vbv_occupancy_overflow_fractional;
	int32_t i_timescale;
	int32_t i_picture_duration;
	int32_t i_pulldown_timescale;
	int32_t i_pulldown_picture_duration;
	int32_t i_quantizer;

#define MAX_LOOKAHEAD_SAMPLES 50
	int32_t i_num_lookahead_samples;
	struct {
		int32_t i_frame_type;
		int32_t i_frame_cost;
	} rgs_lookahead_samples[ MAX_LOOKAHEAD_SAMPLES ];

	int64_t i64_output_ticks;
	int64_t i64_output_frames;
	int64_t i64_output_seconds;

	double d_target_bits;
	double d_output_bits;
	double d_qb_qplx;

	double d_target_bits_2p;
	double d_estimated_bits;
	double d_qb_qplx_2p;

	double rgd_satd_predictors[ 4 ];
	double rgd_satd_predictors_weight[ 4 ];

	double d_confidence_predict_behind;
	double d_confidence_predict_ahead;

	int32_t i_i_picture_baseline_bits;
	int32_t i_min_satd_for_satd_prediction;
	int32_t i_min_bits_for_satd_prediction;

	/* picture coding state */
	int32_t i_picture_bit_budget;
	int32_t i_predicted_frame_size;
	int32_t i_picture_adjusted_bit_budget;
	int32_t i_picture_coded_scaled_satd;
	int32_t i_picture_coded_size;
	int32_t i_picture_scaled_satd;
	int32_t i_picture_accumulated_quantizer;
	int32_t i_picture_num_accumulated_quantizer;
	double d_picture_accumulated_quantizer_bits;
	double d_picture_accumulated_bits_quantizer_over_satd;
	int32_t i_num_picture_accumulated_bits_quantizer_over_satd;
	bool_t b_picture_bad_encountered;
	int32_t i_picture_uncoded_size;
	int32_t i_predicted_frame_size_behind;
	int32_t i_predicted_frame_size_ahead;


	int32_t i_num_samples;
	int32_t i_current_sample;
	y262_ratectrl_isample_t *ps_samples;

	double rgd_last_ref_quantizers[ 2 ];
	int rgi_last_ref_quantizers_pons[ 2 ];

	bool_t b_picture_reencode_pass;
	y262_ratectrl_mb_sample_t *ps_mb_samples;
} y262_bitrate_control_t;

typedef struct {
	int32_t i_slice_bit_budget;
	int32_t i_slice_bit_budget_extra;
	int32_t i_slice_coded_scaled_satd;
	int32_t i_slice_coded_size;
	int32_t i_slice_scaled_satd;
	int32_t i_slice_accumulated_quantizer;
	int32_t i_slice_num_accumulated_quantizer;
	double d_slice_accumulated_quantizer_bits;
	int32_t i_mb_queued_quantizer_f8;
	double d_slice_accumulated_bits_quantizer_over_satd;
	int32_t i_num_slice_accumulated_bits_quantizer_over_satd;
	bool_t b_slice_bad_encountered;
	bool_t b_reencode_pass;
	int32_t i_slice_accumulated_predicted_size;
} y262_slice_encoder_bitrate_control_t;

typedef struct {
	int32_t i_slice_encoder_idx;
#define Y262_SLICE_THREAD_CMD_LOOKAHEAD 0
#define Y262_SLICE_THREAD_CMD_ENCODE    1
#define Y262_SLICE_THREAD_CMD_EXIT      2
	int32_t i_command;
	void *p_thread;
	void *p_start_event;
	void *p_finished_event;
	struct y262_s *ps_y262;
	int32_t i_picture_type;
	int32_t i_first_slice_row;
	int32_t i_last_slice_row;
	/* lookahead */
	y262_picture_t *ps_pic;
	y262_picture_t *ps_fw_ref;
	y262_picture_t *ps_bw_ref;
} y262_slice_thread_t;


typedef struct
{
#define Y262_LOOKAHEAD_THREAD_CMD_LOOKAHEAD 0
#define Y262_LOOKAHEAD_THREAD_CMD_EXIT      1
	int32_t i_command;
	void *p_thread;
	void *p_start_event;
	void *p_finished_event;
} y262_lookahead_thread_t;


typedef struct y262_s {
	void *p_cb_handle;
	y262_function_toolbox_t s_funcs;
	y262_bitstream_t s_bitstream;
	y262_bitrate_control_t s_ratectrl;
	y262_slice_encoder_bitrate_control_t s_slice_encoder_ratectrl;
	y262_picture_t *ps_input_picture;

	bool_t b_multithreading;
	void *p_resource_mutex;
#define MAX_NUM_SLICE_ENCODERS 8
	int32_t i_num_slice_encoders;
	struct y262_s *rgps_slice_encoders[ MAX_NUM_SLICE_ENCODERS ];
	y262_slice_thread_t rgs_slice_threads[ MAX_NUM_SLICE_ENCODERS ];

	bool_t b_lookahead_running;
	int32_t i_num_lookahead_encoders;
	y262_lookahead_thread_t s_lookahead_thread;
	y262_slice_thread_t rgs_lookahead_threads[ MAX_NUM_SLICE_ENCODERS ];

#define MAX_BUFFERED_INPUT_PICTURES 128
	int32_t i_num_lookahead_pictures;
	int32_t i_max_buffered_input_pictures;
	int32_t i_current_input_pon;
	int32_t i_current_input_field;
	int32_t i_lookahead_next_ref;
	int32_t i_lookahead_next_pon;
	int32_t i_leading_lookahead_don;
	int32_t i_current_lookahead_don;
	int32_t i_current_encoder_don;
	int32_t i_keyframe_countdown;
	int32_t i_last_keyframe_temporal_reference;
	int32_t i_current_eof_pon;
	int32_t i_current_eof_don;
	y262_picture_t rgs_buffered_input_pictures[ MAX_BUFFERED_INPUT_PICTURES ];
	bool_t b_flushing;

	bool_t b_next_reference_picture_toggle;
	y262_reference_picture_t rgs_frame_buffer[ 3 ];

	int32_t *rgpi_mbtree_references[ 4 ];

	y262_reference_picture_t *ps_refpic_forward;
	y262_reference_picture_t *ps_refpic_backward;
	y262_reference_picture_t *ps_refpic_dst;

	bool_t b_sequence_mpeg1;
	int32_t i_sequence_display_width;
	int32_t i_sequence_display_height;
	int32_t i_sequence_width;
	int32_t i_sequence_height;
	int32_t i_sequence_chroma_width;
	int32_t i_sequence_chroma_height;
	int32_t i_sequence_chroma_format;
	int32_t i_sequence_video_format;
	int32_t i_sequence_frame_rate_code;
	int32_t i_sequence_pulldown_frame_rate_code;
	int32_t i_sequence_frame_rate_extension_n;
	int32_t i_sequence_frame_rate_extension_d;
	int32_t i_sequence_aspect_ratio_information;
	int32_t i_sequence_derived_picture_duration;
	int32_t i_sequence_derived_timescale;
	int32_t i_sequence_derived_pulldown_picture_duration;
	int32_t i_sequence_derived_pulldown_timescale;
	int32_t i_sequence_num_bframes;
	int32_t i_sequence_keyframe_distance;

	int32_t i_derived_profile;
	int32_t i_derived_level;

	int32_t rgi_fcode[ 2 ][ 2 ];
	int32_t i_intra_dc_precision;
	bool_t b_progressive_sequence;
	bool_t b_frame_pred_frame_dct;
	bool_t b_qscale_type;
	bool_t b_intra_vlc_format;
	bool_t b_closed_gop;
	bool_t b_sequence_cbr;
	int32_t rgi_y262_motion_bits_x[ 2048 + 1 + 2048 ];
	int32_t rgi_y262_motion_bits_y[ 2048 + 1 + 2048 ];

	uint8_t rgui8_intra_quantiser_matrix[64];
	uint8_t rgui8_non_intra_quantiser_matrix[64];
	ALIGNED( 16 ) uint16_t rgui16_intra_quantizer_matrices[ 122 ][ 64 ];
	ALIGNED( 16 ) uint16_t rgui16_intra_quantizer_matrices_bias[ 122 ][ 64 ];
	ALIGNED( 16 ) uint16_t rgui16_intra_quantizer_matrices_trellis_bias[ 122 ][ 64 ];
	ALIGNED( 16 ) uint16_t rgui16_non_intra_quantizer_matrices[ 122 ][ 64 ];

	int32_t i_quantizer;

	bool_t b_variance_aq;
	int32_t i_psyrd_strength;
	int32_t i_quality_for_speed;

} y262_t;
