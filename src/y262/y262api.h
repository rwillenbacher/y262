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

/* unable to allocate memory */
#define Y262_ERROR_MEMORY         1
/* vbv buffer failure */
#define Y262_ERROR_BUFFER         2
/* profile/level limit error */
#define Y262_ERROR_PROFILELEVEL   3
/* thread resource error */
#define Y262_ERROR_THREADING      4
/*
once the encoder is ( partly ) initialized it will report errors through an error callback function
*/
typedef void ( *pf_error_callback_t ) ( void *p_handle, int32_t i_error_code, int8_t *pi8_error_text );


/* structure for result callback function: */
typedef union
{
	struct
	{
		int32_t i_type; /* Y262_RESULT_BITSTREAM */
		int32_t i_don; /* decode order number */
		int32_t i_pon; /* picture ( display ) order number */
		int32_t i_unit_type; /* startcode of bitstream unit */
		uint8_t *pui8_unit; /* pointer to bitstream data */
		int32_t i_unit_length; /* bitstream data length */
	} bitstream_unit;
	struct
	{
		int32_t i_type; /* Y262_RESULT_RECON */
		int32_t i_don; /* decode order number */
		int32_t i_pon; /* picture order number */
		int32_t i_frame_type;
		double f64_psnr[ 3 ]; /* psnr of the 3 planes to the src */
		uint8_t *pui8_luma; /* ptr to recon luma plane, i_coded_width * i_coded_height size */
		uint8_t *pui8_cb; /* ptr to recon cr plane, ( i_coded_width / 2 ) * ( i_coded_height / 2 ) size  */
		uint8_t *pui8_cr; /* ptr to recon cb plane, ( i_coded_width / 2 ) * ( i_coded_height / 2 ) size  */
	} recon;
	struct
	{
		int32_t i_don; /* decode order number */
		uint8_t *pui8_data; /* rc sample data */
		int32_t i_data_length; /* rc sample data length */
	} rc_sample;
} y262_result_t;

#define Y262_RESULT_BITSTREAM  1
#define Y262_RESULT_RECON      2
#define Y262_RESULT_RC_SAMPLE  3
/*
when the encoder has an encoding result ( bitstream unit, recon picture or bitrate control sample for multipass )
it will output the result through a callback function. i_result_type one of the Y262_RESULT_* defines above
*/
typedef void ( *pf_result_callback_t ) ( void *p_handle, int32_t i_result_type, y262_result_t *ps_result );


/*
if the rate control mode is set to 2nd pass the encoder will read bitrate control samples from a previous pass
through a callback function. the callback should return the bitrate control sample from the previous pass
with the same decode order number ( i_don ) by copying it to pui8_rcv_data. i_data_length is the expected size of
the rc sample data and the size of the pui8_rvc_data array.
the callback should return the size of the rc sample ( equal to i_data_length ) if such an rc sample is available.
if the previous pass did not put out an rc sample with the requested decode order number, the sequence was shorter,
the callback function should return 0, signalling end of sequence.

the behaviour of the encoder is undefined if more frames are pushed into the encoder for encoding than rc samples were
supplied through the callback function.
*/
typedef int32_t ( *pf_rcsample_callback_t ) ( void *p_handle, int32_t i_don, uint8_t *pui8_rcv_data, int32_t i_data_length );

typedef struct {
	int32_t i_multithreading; /* != 0 means enable multithreading */
	int32_t i_num_threads; /* number of threads, >= 1 , <= 8 */

	int32_t i_display_width; /* display width, for example 1920 */
	int32_t i_display_height; /* display height, for example 1080 */
	int32_t i_coded_width;   /* coded width, has to be multiple of 16, for example 1920 */
	int32_t i_coded_height; /* coded height, has to be multiple of 16, for example 1088 */
#define Y262_CHROMA_FORMAT_420  1
#define Y262_CHROMA_FORMAT_422  2
#define Y262_CHROMA_FORMAT_444  3
	int32_t i_coded_chroma_format;

#define Y262_PROFILE_DERIVE 256
#define Y262_PROFILE_SIMPLE 5
#define Y262_PROFILE_MAIN   4
#define Y262_PROFILE_HIGH   1
#define Y262_PROFILE_422    8
	int32_t i_profile;	/* profile, one of the profile defines  */

#define Y262_LEVEL_DERIVE   256
#define Y262_LEVEL_LOW      10
#define Y262_LEVEL_MAIN     8
#define Y262_LEVEL_HIGH1440 6
#define Y262_LEVEL_HIGH     4
#define Y262_LEVEL_422MAIN  5
#define Y262_LEVEL_422HIGH  2
	int32_t i_level;  /* level, one of the level defines */

#define Y262_VIDEOFORMAT_PAL     0
#define Y262_VIDEOFORMAT_NTSC    1
#define Y262_VIDEOFORMAT_SECAM   2
#define Y262_VIDEOFORMAT_709     3
#define Y262_VIDEOFORMAT_UNKNOWN 4
	int32_t i_videoformat; /* video format, one of the videoformat defines */

	int32_t i_frame_rate_code; /* framerate code, see mpeg2 spec or readme.txt for valid values */
	int32_t i_pulldown_frame_rate_code; /* framerate code to pull input up to ( field repeat has to be done by application ) */
	int32_t i_aspect_ratio_information; /* aspect ratio code, see mpeg2 spec or readme txt for valid values */

#define BITRATE_CONTROL_CQ    0
#define BITRATE_CONTROL_PASS1 1
#define BITRATE_CONTROL_PASS2 2
	int32_t i_rcmode; /* bitrate control mode, one of the bitrate control defines */
	int32_t i_bitrate; /* average bitrate, in kbps */
	int32_t i_vbv_rate; /* video buffer rate, in kbps, >= bitrate */
	int32_t i_vbv_size; /* video buffer size, in kbit */
	int32_t i_quantizer; /* quantizer for CQ mode, >= 1, <= 31 */

	int32_t b_interlaced; /* enable interlaced modes */
	int32_t b_top_field_first; /* field order of the first picture, due to field repeating this might change per picture */
	int32_t i_lookahead_pictures; /* number of lookahead pictures, >= 10, <= 50 */
	int32_t i_bframes;  /* number of b frames between reference frames, >= 0, <= 4 */
	int32_t i_keyframe_ref_distance; /* number of reference frames between keyframes */
	int32_t b_closed_gop; /* if this is != 0 then B frames directly following an I frame in a new GOP in decode order only reference the I frame */
	int32_t rgi_fcode[ 2 ]; /* fcode for motion vector range, max range is profile dependent, mpeg2: horizontal, vertical, mpeg1: all, ignored */

	int32_t b_non_default_intra_matrix; /* enable custom intra matrix stored directly below */
	uint8_t rgui8_non_default_intra_matrix[ 64 ]; /* custom intra matrix */

	int32_t b_non_default_inter_matrix; /* enable custom inter matrix stored directly below */
	uint8_t rgui8_non_default_inter_matrix[ 64 ]; /* custom inter matrix */

	void *p_cb_handle; /* user pointer callbacks get called with */
	pf_error_callback_t pf_error_callback; /* user supplied error callback function */
	pf_result_callback_t pf_result_callback; /* user supplied result callback function */
	pf_rcsample_callback_t pf_rcsample_callback; /* user supplied rc sample read callback function */

	int32_t b_variance_aq; /* if this is != 0 then variance based AQ is enabled */
	int32_t i_psyrd_strength; /* >= 0, <= 512, experimental */
	int32_t i_quality_for_speed; /* speed/quality tradeoff, negative gives more speed, positive more quality, >= -100, <= 100 */

	int32_t b_qscale_type; /* if this is != 0 then non linear qscale will be used */
	int32_t b_mpeg1;
	int32_t b_cbr_padding; /* guarantee vbvrate rate, insert zero byte stuffing if needed */

} y262_configuration_t;

typedef struct {
	uint8_t *pui8_luma; /* input picture luma plane, i_coded_width * i_coded_height size */
	uint8_t *pui8_cb;   /* input picture cb plane, ( i_coded_width >> 1 ) * ( i_coded_height size >> 1 ) */
	uint8_t *pui8_cr;   /* input picture cr plane, ( i_coded_width >> 1 ) * ( i_coded_height size >> 1 ) */
#define Y262_INPUT_PICTURE_FRAME_PROGRESSIVE        0
#define Y262_INPUT_PICTURE_FRAME_PROGRESSIVE_REPEAT 1
#define Y262_INPUT_PICTURE_FRAME_INTERLACED         2
	/* PROGRESSIVE means a progressive frame */
	/* PROGRESSIVE_REPEAT means a progressive frame of which one field is to be repeated for pulldown */
	/* INTERLACED means the frame is actually to interleaved fields */
	int32_t i_frame_structure; /* one of the Y262_INPUT_PICTURE_FRAME_* defines */

	int32_t b_start_new_gop; /* start a new gop at this picture ( force I ) */

	int32_t i_num_user_data; /* number of user data payloads to be inserted into the stream at this frame */
#define Y262_MAX_NUM_USER_DATA 4
	uint8_t *rgpui8_user_data[ Y262_MAX_NUM_USER_DATA ]; /* pointer to the user data payloads */
#define Y262_MAX_USER_DATA_SIZE ( 1 << 17 )
	int32_t rgi_user_data_len[ Y262_MAX_NUM_USER_DATA ]; /* user data payloads lengths */
#define Y262_USER_DATA_AFTER_SEQUENCE  0
#define Y262_USER_DATA_AFTER_GOP       1
#define Y262_USER_DATA_BEFORE_SLICES   2
	int32_t rgi_user_data_where[ Y262_MAX_NUM_USER_DATA ]; /* user data location, one of the Y262_USER_DATA_* defines */
} y262_input_picture_t;


/*
these functions should be used to allocate and deallocate the planes of a y262_input_picture_t
*/

void *y262_alloc( size_t s_size );
void y262_dealloc( void *p_ptr );

/*
creates an encoder context and initializes the configuration ps_config with some default values.
returns the encoder context.
*/
void *y262_create( y262_configuration_t *ps_config );

#define Y262_INIT_SUCCESS              0

/* coded size not multiple of 16 or < 16, > 4096 */
#define Y262_INIT_ERROR_CODED_SIZE    -1
/* display size > coded size */
#define Y262_INIT_ERROR_DISPLAY_SIZE  -2
/* fr code out of range */
#define Y262_INIT_ERROR_FRAMERATE     -3
/* pulldown fr code out of range */
#define Y262_INIT_ERROR_PFRAMERATE    -4
/* rc mode out of range */
#define Y262_INIT_ERROR_RCMODE        -5
/* bitrate < 20kbit */
#define Y262_INIT_ERROR_BITRATE       -6
/* vbv rate < 20kbit */
#define Y262_INIT_ERROR_VBVRATE       -7
/* vbv size < 20kbit */
#define Y262_INIT_ERROR_VBVSIZE       -8
/* quantizer out of range */
#define Y262_INIT_ERROR_QUANTIZER     -9
/* lookahead pic < 10 or > 50 */
#define Y262_INIT_ERROR_LOOKAHEADPICS -10
/* keyframe to ref distance < 0 */
#define Y262_INIT_ERROR_KEYFRAME_DIST -11
/* b frame count < 0 or > 4 */
#define Y262_INIT_ERROR_BFRAMES_COUNT -12
/* quality for speed < -100 or > 100 */
#define Y262_INIT_ERROR_QUALITY_SPEED -13
/* psyrd strength > 512 */
#define Y262_INIT_ERROR_PSYRD_STR     -14
/* aspect ratio code out of range */
#define Y262_INIT_ERROR_ASPECT        -15
/* thread count < 1 or > 8 */
#define Y262_INIT_ERROR_THREADS       -16
/* video format code out of range */
#define Y262_INIT_ERROR_VIDEO_FORMAT  -17
/* invalid profile */
#define Y262_INIT_ERROR_PROFILE       -18
/* invalid level */
#define Y262_INIT_ERROR_LEVEL         -19
/* invalid chroma format */
#define Y262_INIT_ERROR_CHROMA_FORMAT -20
/* generic mpeg1 constraint failure */
#define Y262_INIT_ERROR_MPEG1_CONSTRAINT -21
/* invalid chroma format for mpeg1 */
#define Y262_INIT_ERROR_MPEG1_CHROMA_FORMAT -22
/* mpeg1 does not support interlaced */
#define Y262_INIT_ERROR_MPEG1_INTERLACED -23
/* mpeg1 does not support interlaced */
#define Y262_INIT_ERROR_MPEG1_QSCALE -24
/* invalid fcode for mpeg1 ( only config.rgi_fcode[ 0 ] is used ) */
#define Y262_INIT_ERROR_MPEG1_FCODE -25
/* invalid b frame count for profile */
#define Y262_INIT_ERROR_PROFILE_BFRAMES_COUNT -26
/* invalid config for level maximums */
#define Y262_INIT_ERROR_LEVEL_LIMITS -27
/* internal resource error */
#define Y262_INIT_ERROR_RESOURCE_INTERNAL -28
/* invalid y262 context */
#define Y262_INIT_ERROR_CONTEXT -29
/* invalid chroma format for profile */
#define Y262_INIT_ERROR_PROFILE_CHROMA_FORMAT -30

/*
initializes the encoder context p_y262 with the encoding configuration ps_config.
returns Y262_INIT_SUCCESS or one of the Y262_INIT_* error codes above.
*/
int32_t y262_initialize( void *p_y262, y262_configuration_t *ps_config );


#define Y262_PUSH_INPUT_CONTINUE     0
#define Y262_PUSH_INPUT_FLUSHED      1
#define Y262_PUSH_INPUT_ERR_ARG     -1
#define Y262_PUSH_INPUT_ERR_STATE   -2

/*
pushes the input picture ps_picture into the encoder context p_y262 for encoding.
if the input picture ps_picture is NULL then this signals the encoder that no more
pictures are to be encoded and that the picture queue is to be flushed.

returns Y262_PUSH_INPUT_CONTINUE if the function can be called again ( with NULL as ps_picture if flushing ).
returns Y262_PUSH_INPUT_FLUSHED if flushing is done.
returns Y262_PUSH_INPUT_ERR_ARG if p_y262 or ps_picture is invalid.
returns Y262_PUSH_INPUT_ERR_STATE if the function was called with ps_picture as NULL before but then got supplied a non NULL ps_picture.
*/
int32_t y262_push_input_picture( void *p_y262, y262_input_picture_t *ps_picture, int32_t i_pon );

/*
destroys the encoder context p_y262 and frees resources
*/
void y262_deinitialize( void *p_y262 );
