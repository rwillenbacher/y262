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

#define _CRT_SECURE_NO_WARNINGS 1

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef TRUE
#define TRUE 1
#endif

#if defined( WIN32 ) || defined( WIN64 )
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#define DUMP_MEM_LEAKS 0

#if DUMP_MEM_LEAKS
#include <crtdbg.h>
#endif

#include "y262api.h"


FILE *f_rec, *f_out, *f_mpass_out = NULL, *f_mpass_in = NULL;
int64_t i64_bytes_num_out = 0;
int32_t i_accumulated_bytes;

typedef struct {
	int32_t i_don;
	int32_t i_len;
} y262app_rcsamplehdr_t;


double d_num_mean_psnr;
double rgd_mean_psnr_accum[ 3 ];

int32_t i_next_recon_output_pon = 0;
int32_t i_buffered_output_picture_pon = -1;
int32_t i_pad_x, i_pad_y, i_width = 720, i_height = 576, i_frcode = 1;
int32_t i_chroma_width, i_chroma_height, i_coded_chroma_width, i_coded_chroma_height;

static const int32_t rgi_framerate_code_duration[ 16 ] = { 
	3600, 1001, 1000, 3600, 1001, 3000, 1800, 1001, 1500, 3600, 3600, 3600, 3600, 3600, 3600, 3600
};

static const int32_t rgi_framerate_code_timescale[ 16 ] = { 
	90000, 24000, 24000, 90000, 30000, 90000, 90000, 60000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000
};

double d_picture_duration, d_timescale;

y262_input_picture_t s_buffered_output_picture;

y262_configuration_t s_config;

void y262app_error_cb( void *p_handle, int32_t i_code, int8_t *pi8_msg )
{
	fprintf( stderr, "error %d: '%s'\n" , i_code, pi8_msg );
}

void y262app_result_cb( void *p_handle, int32_t i_result_type, y262_result_t *ps_result )
{
	if( i_result_type == Y262_RESULT_BITSTREAM )
	{
		/*fprintf( stderr, "result %d ( p %d d %d )\n", i_result_type, ps_result->bitstream_unit.i_pon, ps_result->bitstream_unit.i_don );*/
		i64_bytes_num_out += ps_result->bitstream_unit.i_unit_length;
		i_accumulated_bytes += ps_result->bitstream_unit.i_unit_length;
		if( f_out != NULL )
		{
			fwrite( ps_result->bitstream_unit.pui8_unit, sizeof( uint8_t ), ps_result->bitstream_unit.i_unit_length, f_out );
		}
	}
	else if( i_result_type == Y262_RESULT_RECON )
	{
		/*fprintf( stderr, "result %d ( p %d d %d )\n", i_result_type, ps_result->recon.i_pon, ps_result->recon.i_don );*/
		rgd_mean_psnr_accum[ 0 ] += ps_result->recon.f64_psnr[ 0 ];
		rgd_mean_psnr_accum[ 1 ] += ps_result->recon.f64_psnr[ 1 ];
		rgd_mean_psnr_accum[ 2 ] += ps_result->recon.f64_psnr[ 2 ];
		d_num_mean_psnr += 1.0;

		fprintf( stderr, "%04d: T:%c Y=%.2f CB=%.2f CR=%.2f sz=%d\n", ps_result->recon.i_pon,
			ps_result->recon.i_frame_type == 1 ? 'I' : ps_result->recon.i_frame_type == 2 ? 'P' : 'B',
			ps_result->recon.f64_psnr[ 0 ], ps_result->recon.f64_psnr[ 1 ], ps_result->recon.f64_psnr[ 2 ], i_accumulated_bytes );
		fflush( stderr );
		i_accumulated_bytes = 0;

		if( f_rec )
		{
			if( ps_result->recon.i_pon == i_next_recon_output_pon )
			{
				fwrite( ps_result->recon.pui8_luma, s_config.i_coded_width * s_config.i_coded_height, 1, f_rec );
				fwrite( ps_result->recon.pui8_cb, i_coded_chroma_width * i_coded_chroma_height, 1, f_rec );
				fwrite( ps_result->recon.pui8_cr, i_coded_chroma_width * i_coded_chroma_height, 1, f_rec );
				i_next_recon_output_pon++;
			}
			else
			{
				memcpy( s_buffered_output_picture.pui8_luma, ps_result->recon.pui8_luma, s_config.i_coded_width * s_config.i_coded_height );
				memcpy( s_buffered_output_picture.pui8_cb, ps_result->recon.pui8_cb, i_coded_chroma_width * i_coded_chroma_height );
				memcpy( s_buffered_output_picture.pui8_cr, ps_result->recon.pui8_cr, i_coded_chroma_width * i_coded_chroma_height );
				i_buffered_output_picture_pon = ps_result->recon.i_pon;
			}
			if( i_buffered_output_picture_pon == i_next_recon_output_pon )
			{
				fwrite( s_buffered_output_picture.pui8_luma, s_config.i_coded_width * s_config.i_coded_height, 1, f_rec );
				fwrite( s_buffered_output_picture.pui8_cb, i_coded_chroma_width * i_coded_chroma_height, 1, f_rec );
				fwrite( s_buffered_output_picture.pui8_cr, i_coded_chroma_width * i_coded_chroma_height, 1, f_rec );
				i_next_recon_output_pon++;
				i_buffered_output_picture_pon = -1;
			}
		}
	}
	else if( i_result_type == Y262_RESULT_RC_SAMPLE && f_mpass_out != NULL )
	{
		y262app_rcsamplehdr_t s_sample;
		/*fprintf( stderr, "result %d ( d %d )\n", i_result_type, ps_result->rc_sample.i_don );*/
		s_sample.i_don = ps_result->rc_sample.i_don;
		s_sample.i_len = ps_result->rc_sample.i_data_length;
		fwrite( &s_sample, sizeof( s_sample ), 1, f_mpass_out );
		fwrite( ps_result->rc_sample.pui8_data, s_sample.i_len, 1, f_mpass_out );
	}
}


int32_t y262app_rcsample_cb( void *p_handle, int32_t i_don, uint8_t *pui8_data, int32_t i_data_length )
{
	if( f_mpass_in )
	{
		int32_t i_seeked = 0;
		int32_t i_ret;
		y262app_rcsamplehdr_t s_sample;

		while( 1 )
		{
			i_ret = fread( &s_sample, sizeof( s_sample ), 1, f_mpass_in );
			if( i_ret == 1 )
			{
				if( i_data_length == s_sample.i_len )
				{
					fread( pui8_data, s_sample.i_len, 1, f_mpass_in );
				}
				else
				{
					break; /* version missmatch ? */
				}
				if( s_sample.i_don == i_don && i_data_length == s_sample.i_len )
				{
					return s_sample.i_len;
				}
				else if( s_sample.i_don > i_don )
				{
					if( !i_seeked )
					{
						fseek( f_mpass_in, SEEK_SET, 0 );
						i_seeked = 1;
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				if( !i_seeked )
				{
					fseek( f_mpass_in, SEEK_SET, 0 );
					i_seeked = 1;
				}
				else
				{
					break;
				}
			}
		}
	}
	return 0;
}


void usage( )
{
	fprintf( stderr, "y262 usage:\n");
	fprintf( stderr, "\ty262app -in <420yuv> -size <width> <height> -out <m2vout>\n\n");
	fprintf( stderr, "\t-frames <number>    : number of frames to encode, 0 for all\n");
	fprintf( stderr, "\t-threads <on> <cnt> :  threading enabled and number of concurrent slices\n" );
	fprintf( stderr, "\t-profile <profile>  :  simple or main profile\n");
	fprintf( stderr, "\t-level <level>      :  low main high1440 or high level\n");
	fprintf( stderr, "\t-chromaf            :  chroma format, 420, 422 or 444\n");
	fprintf( stderr, "\t-rec <reconfile>    :  write reconstructed frames to <reconfile>\n");
	fprintf( stderr, "\t-rcmode <pass>      :  0 = CQ, 1 = 1st pass, 2 = subsequent pass\n");
	fprintf( stderr, "\t-mpin <statsfile>   :  stats file of previous pass\n");
	fprintf( stderr, "\t-mpout <statsfile>  :  output stats file of current pass\n");
	fprintf( stderr, "\t-bitrate <kbps>     :  average bitrate\n");
	fprintf( stderr, "\t-vbvrate <kbps>     :  maximum bitrate\n");
	fprintf( stderr, "\t-vbv <kbps>         :  video buffer size\n");
	fprintf( stderr, "\t-quant <quantizer>  :  quantizer for CQ\n");
	fprintf( stderr, "\t-interlaced         :  enable field macroblock modes\n");
	fprintf( stderr, "\t-bff                :  first input frame is bottom field first\n");
	fprintf( stderr, "\t-pulldown_frcode <num>:frame rate code to pull input up to\n");
	fprintf( stderr, "\t-quality <number>   :  encoder complexity, negative faster, positive slower\n");
	fprintf( stderr, "\t-frcode <number>    :  frame rate code, see mpeg2 spec\n");
	fprintf( stderr, "\t-arinfo <number>    :  aspect ratio information, see mpeg2 spec\n");
	fprintf( stderr, "\t-qscale0            :  use more linear qscale type\n");
	fprintf( stderr, "\t-nump <number>      :  number of p frames between i frames\n");
	fprintf( stderr, "\t-numb <number>      :  number of b frames between i/p frames\n");
	fprintf( stderr, "\t-closedgop          :  bframes after i frames use only backwards prediction\n");
	fprintf( stderr, "\t-noaq               :  disable variance based quantizer modulation\n");
	fprintf( stderr, "\t-psyrd <number>     :  psy rd strength\n");
	fprintf( stderr, "\t-avamat6            :  use avamat6 quantization matrices\n");
	fprintf( stderr, "\t-flatmat            :  use flat quantization matrices <for high rate>\n");
	fprintf( stderr, "\t-intramat <textfile>:  use the 64 numbers in the file as intra matrix\n");
	fprintf( stderr, "\t-intermat <textfile>:  use the 64 numbers in the file as inter matrix\n");
	fprintf( stderr, "\t-videoformat <fmt>  :  pal, secam, ntsc, 709 or unknown \n");
	fprintf( stderr, "\t-mpeg1              :  output mpeg1 instead mpeg2, constraints apply\n" );
}

int32_t read_mat( uint8_t *pui8_filename, uint8_t *pui8_mat )
{
	FILE *f_in;
	char rgc_number[ 0x200 ];
	int32_t i_midx, i_nidx, i_r, i_ret;

	f_in = fopen( ( const char *)pui8_filename, "rt" );

	if( f_in == NULL )
	{
		fprintf( stderr, "error reading quant matrix, cannot open file '%s'\n", pui8_filename );
		return -1;
	}

	i_ret = -1;
	i_midx = i_nidx = 0;
	while( 1 )
	{
		i_r = fgetc( f_in );
		if( i_r < '0' || i_r > '9' )
		{
			if( !( i_r < 0 ) && i_r != '\n' && i_r != '\t' && i_r != '\r' && i_r != ' ' )
			{
				fprintf( stderr, "error reading quant matrix, invalid char '%c'\n", i_r );
				goto err;
			}
			if( i_nidx > 0 && i_midx < 64 )
			{
				/* flush number */
				rgc_number[ i_nidx ] = 0;
				i_r = atoi( rgc_number );
				if( i_r < 8 || i_r > 255 )
				{
					fprintf( stderr, "error reading quant matrix %s, value %d is outside 8-255 range\n", pui8_filename, i_r );
					goto err;
				}
				pui8_mat[ i_midx++ ] = ( uint8_t )i_r;
				i_nidx = 0;
			}
			else if( i_nidx > 0 && i_midx >= 64 )
			{
				fprintf( stderr, "error reading quant matrix %s, more than 64 values\n", pui8_filename );
				goto err;
			}
			if( i_r < 0 )
			{
				break;
			}
		}
		else
		{
			if( i_nidx >= 0x200 )
			{
				fprintf( stderr, "error reading quant matrix %s, please just check whats in the file\n", pui8_filename );
				goto err;
			}
			rgc_number[ i_nidx++ ] = ( char )i_r;
		}
	}
	if( i_midx == 64 )
	{
		i_ret = 0;
	}
	else
	{
		fprintf( stderr, "error reading quant matrix %s, file does not contain 64 numbers\n", pui8_filename );
	}
err:
	fclose( f_in );
	return i_ret;
}

int32_t main( int32_t i_argc, char *rgpi8_argv[] )
{
	FILE *f_in;
	int32_t i_idx, i_ret, i_input_picture_duration, i_input_timescale, i_sequence_picture_duration, i_sequence_timescale, i_field_count, i_frames;
	int64_t i64_input_frac_ticks, i64_input_frac_picture_duration;
	
	void *p_y262;
	int8_t *pi8_infile = NULL, *pi8_outfile = NULL, *pi8_reconfile = NULL, *pi8_mp_in = NULL, *pi8_mp_out = NULL;
	uint8_t rgui8_avamat6_intra[ 64 ] = {  8,16,19,22,26,27,29,34,16,16,22,24,27,29,34,35,19,22,26,27,29,34,35,38,22,22,26,27,29,34,35,40,22,26,27,29,32,35,40,48,26,27,29,32,35,40,48,50,26,27,29,35,40,48,50,60,27,29,35,40,48,50,60,62};
	uint8_t rgui8_avamat6_inter[ 64 ] = { 16,20,24,28,32,36,40,44,20,24,28,32,36,40,44,48,24,28,32,36,40,44,48,52,28,32,36,40,44,48,52,56,32,36,40,44,48,52,56,58,36,40,44,48,52,56,58,60,40,44,48,52,56,58,60,62,44,48,52,56,58,60,62,62};
	uint8_t rgui8_flatmat_intra[ 64 ] = {  8,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};
	uint8_t rgui8_flatmat_inter[ 64 ] = { 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};

	y262_input_picture_t s_picture;

	i_frames = 0;

	p_y262 = y262_create( &s_config );

	s_config.b_top_field_first = 1;
	s_config.i_pulldown_frame_rate_code = 0;
	s_config.b_qscale_type = 1;

	for( i_idx = 1; i_idx < i_argc; i_idx++ )
	{
		if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-in" ) == 0 )
		{
			pi8_infile = rgpi8_argv[ ++i_idx ];
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-frames" ) == 0 )
		{
			i_frames = atoi( ( char * ) rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-size" ) == 0 )
		{
			i_width = atoi( ( char * ) rgpi8_argv[ ++i_idx ] );
			i_height = atoi( ( char * ) rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-threads" ) == 0 )
		{
			s_config.i_multithreading = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
			s_config.i_num_threads = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-profile" ) == 0 )
		{
			i_idx++;
			if( strcmp( ( char *)rgpi8_argv[ i_idx ], "simple" ) == 0 )
			{
				s_config.i_profile = Y262_PROFILE_SIMPLE;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "main" ) == 0 )
			{
				s_config.i_profile = Y262_PROFILE_MAIN;
			}
			else
			{
				fprintf( stderr, "unknown profile specified on commandline\n");
			}
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-level" ) == 0 )
		{
			i_idx++;
			if( strcmp( ( char *)rgpi8_argv[ i_idx ], "low" ) == 0 )
			{
				s_config.i_level = Y262_LEVEL_LOW;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "main" ) == 0 )
			{
				s_config.i_level = Y262_LEVEL_MAIN;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "high1440" ) == 0 )
			{
				s_config.i_level = Y262_LEVEL_HIGH1440;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "high" ) == 0 )
			{
				s_config.i_level = Y262_LEVEL_HIGH;
			}
			else
			{
				fprintf( stderr, "unknown level specified on commandline\n" );
				return -1;
			}
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-chromaf" ) == 0 )
		{
			i_idx++;
			if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "420" ) == 0 )
			{
				s_config.i_coded_chroma_format = Y262_CHROMA_FORMAT_420;
			}
			else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "422" ) == 0 )
			{
				s_config.i_coded_chroma_format = Y262_CHROMA_FORMAT_422;
			}
			else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "444" ) == 0 )
			{
				s_config.i_coded_chroma_format = Y262_CHROMA_FORMAT_444;
			}
			else
			{
				fprintf( stderr, "unknown chroma format specified on commandline\n" );
				return -1;
			}
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-out" ) == 0 )
		{
			pi8_outfile = rgpi8_argv[ ++i_idx ];
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-rec" ) == 0 )
		{
			pi8_reconfile = rgpi8_argv[ ++i_idx ];
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-mpin" ) == 0 )
		{
			pi8_mp_in = rgpi8_argv[ ++i_idx ];
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-mpout" ) == 0 )
		{
			pi8_mp_out = rgpi8_argv[ ++i_idx ];
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-bitrate" ) == 0 )
		{
			s_config.i_bitrate = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-vbvrate" ) == 0 )
		{
			s_config.i_vbv_rate = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-vbv" ) == 0 )
		{
			s_config.i_vbv_size = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-rcmode" ) == 0 )
		{
			s_config.i_rcmode = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-quant" ) == 0 )
		{
			s_config.i_quantizer = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-interlaced" ) == 0 )
		{
			s_config.b_interlaced = 1;
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-bff" ) == 0 )
		{
			s_config.b_top_field_first = 0;
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-pulldown_frcode" ) == 0 )
		{
			s_config.i_pulldown_frame_rate_code = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-quality" ) == 0 )
		{
			s_config.i_quality_for_speed = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-frcode" ) == 0 )
		{
			s_config.i_frame_rate_code = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-arinfo" ) == 0 )
		{
			s_config.i_aspect_ratio_information = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-qscale0" ) == 0 )
		{
			s_config.b_qscale_type = 0;
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-nump" ) == 0 )
		{
			s_config.i_keyframe_ref_distance = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-numb" ) == 0 )
		{
			s_config.i_bframes = atoi( ( char * ) rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-closedgop" ) == 0 )
		{
			s_config.b_closed_gop = TRUE;
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-noaq" ) == 0 )
		{
			s_config.b_variance_aq = 0;
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-psyrd" ) == 0 )
		{
			s_config.i_psyrd_strength = atoi( ( char *)rgpi8_argv[ ++i_idx ] );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-avamat6" ) == 0 )
		{
			s_config.b_non_default_intra_matrix = 1;
			memcpy( &s_config.rgui8_non_default_intra_matrix, rgui8_avamat6_intra, sizeof( rgui8_avamat6_intra ) );
			s_config.b_non_default_inter_matrix = 1;
			memcpy( &s_config.rgui8_non_default_inter_matrix, rgui8_avamat6_inter, sizeof( rgui8_avamat6_inter ) );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-flatmat" ) == 0 )
		{
			s_config.b_non_default_intra_matrix = 1;
			memcpy( &s_config.rgui8_non_default_intra_matrix, rgui8_flatmat_intra, sizeof( rgui8_flatmat_intra ) );
			s_config.b_non_default_inter_matrix = 1;
			memcpy( &s_config.rgui8_non_default_inter_matrix, rgui8_flatmat_inter, sizeof( rgui8_flatmat_inter ) );
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-intramat" ) == 0 )
		{
			s_config.b_non_default_intra_matrix = 1;
			if( read_mat( ( uint8_t * )rgpi8_argv[ ++i_idx ], &s_config.rgui8_non_default_intra_matrix[ 0 ] ) < 0 )
			{
				return -1;
			}
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-intermat" ) == 0 )
		{
			s_config.b_non_default_inter_matrix = 1;
			if( read_mat( ( uint8_t * )rgpi8_argv[ ++i_idx ], &s_config.rgui8_non_default_inter_matrix[ 0 ] ) < 0 )
			{
				return -1;
			}
		}
		else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "-videoformat" ) == 0 )
		{
			i_idx++;
			if( strcmp( ( char *)rgpi8_argv[ i_idx ], "pal" ) == 0 )
			{
				s_config.i_videoformat = Y262_VIDEOFORMAT_PAL;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "secam" ) == 0 )
			{
				s_config.i_videoformat = Y262_VIDEOFORMAT_SECAM;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "ntsc" ) == 0 )
			{
				s_config.i_videoformat = Y262_VIDEOFORMAT_NTSC;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "709" ) == 0 )
			{
				s_config.i_videoformat = Y262_VIDEOFORMAT_709;
			}
			else if( strcmp( ( char *)rgpi8_argv[ i_idx ], "unknown" ) == 0 )
			{
				s_config.i_videoformat = Y262_VIDEOFORMAT_UNKNOWN;
			}
			else
			{
				fprintf( stderr, "unknown video format specified on commandline\n");
				return -1;
			}
		}
		else if( strcmp( ( char* ) rgpi8_argv[ i_idx ], "-mpeg1" ) == 0 )
		{
			s_config.b_mpeg1 = 1;
		}
		else if( strcmp( ( char * ) rgpi8_argv[ i_idx ], "-cbr" ) == 0 )
		{
			s_config.b_cbr_padding = 1;
		}
		else
		{
			fprintf( stderr, "unknown commandline argument '%s'\n", rgpi8_argv[ i_idx ] );
			return -1;
		}
	}

	if( i_argc < 2 || !pi8_infile )
	{
		usage();
		return -1;
	}


	s_config.pf_error_callback = y262app_error_cb;
	s_config.pf_result_callback = y262app_result_cb;
	s_config.pf_rcsample_callback = y262app_rcsample_cb;

	if( pi8_infile )
	{
		if( strcmp( ( char * ) pi8_infile, "-" ) == 0 )
		{
#ifdef WIN32
			_setmode( _fileno( stdin ), _O_BINARY );
#else
			stdin = freopen( NULL, "rb", stdin );
#endif
			f_in = stdin;
		}
		else
		{
			f_in = fopen( ( char *)pi8_infile, "rb" );
		}
		if( !f_in )
		{
			fprintf( stderr, "could not open input file\n");
			return -1;
		}
	}
	else
	{
		fprintf( stderr, "need -in <ifile> commandline argument\n");
	}

	if( pi8_outfile )
	{
		f_out = fopen( ( char *)pi8_outfile, "wb" );
		if( !f_out )
		{
			return -1;
		}
	}
	else
	{
		f_out = NULL;
		fprintf( stderr, "no -out <ofile> commandline argument found, no output\n");
	}

	if( pi8_reconfile )
	{
		f_rec = fopen( ( char *)pi8_reconfile, "wb" );
		if( !f_rec )
		{
			return -1;
		}
	}
	else
	{
		f_rec = NULL;
		fprintf( stderr, "no -rec <file> commandline argument found, no recon output\n");
	}

	if( pi8_mp_in )
	{
		f_mpass_in = fopen( ( char *)pi8_mp_in, "rb" );
		if( !f_mpass_in )
		{
			fprintf( stderr, "could not open mpass in file %s\n", ( char *)pi8_mp_in );
			return -1;
		}
	}
	else
	{
		f_mpass_in = NULL;
		if( s_config.i_rcmode == 2 )
		{
			fprintf( stderr, "no multipass input file ( -mpin ) but -rcmode is 2\n");
			return -1;
		}
	}

	if( pi8_mp_out )
	{
		f_mpass_out = fopen( ( char *)pi8_mp_out, "wb" );
		if( !f_mpass_out )
		{
			fprintf( stderr, "could not open mpass out file %s\n", ( char *)pi8_mp_out );
			return -1;
		}
	}
	else
	{
		f_mpass_out = NULL;
	}

	if( s_config.i_pulldown_frame_rate_code == 0 )
	{
		s_config.i_pulldown_frame_rate_code = s_config.i_frame_rate_code;
	}
	if( s_config.i_pulldown_frame_rate_code < s_config.i_frame_rate_code )
	{
		fprintf( stderr, "error: pulldown frame rate lower than frame rate\n");
		return -1;
	}

	i_frcode = s_config.i_frame_rate_code;
	i_input_picture_duration = rgi_framerate_code_duration[ i_frcode ];
	i_input_timescale = rgi_framerate_code_timescale[ i_frcode ];

	i_frcode = s_config.i_pulldown_frame_rate_code;
	d_picture_duration = i_sequence_picture_duration = rgi_framerate_code_duration[ i_frcode ];
	d_timescale = i_sequence_timescale = rgi_framerate_code_timescale[ i_frcode ];
	

	s_config.i_display_width = i_width;
	s_config.i_display_height = i_height;
	i_pad_x = ( ( ( i_width + 15 ) / 16 ) * 16 ) - i_width;
	i_pad_y = ( ( ( i_height + 15 ) / 16 ) * 16 ) - i_height;

	s_config.i_coded_width = i_width + i_pad_x;
	s_config.i_coded_height = i_height + i_pad_y;
	switch( s_config.i_coded_chroma_format )
	{
		case Y262_CHROMA_FORMAT_420:
			i_coded_chroma_width = s_config.i_coded_width >> 1;
			i_coded_chroma_height = s_config.i_coded_height >> 1;
			i_chroma_width = i_width >> 1;
			i_chroma_height = i_height >> 1;
			break;
		case Y262_CHROMA_FORMAT_422:
			i_coded_chroma_width = s_config.i_coded_width >> 1;
			i_coded_chroma_height = s_config.i_coded_height;
			i_chroma_width = i_width >> 1;
			i_chroma_height = i_height;
			break;
		case Y262_CHROMA_FORMAT_444:
			i_coded_chroma_width = s_config.i_coded_width;
			i_coded_chroma_height = s_config.i_coded_height;
			i_chroma_width = i_width;
			i_chroma_height = i_height;
			break;
	}

	s_picture.pui8_luma = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * s_config.i_coded_width * s_config.i_coded_height );
	s_picture.pui8_cb = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );
	s_picture.pui8_cr = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );

	i_buffered_output_picture_pon = -1;
	s_buffered_output_picture.pui8_luma = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * s_config.i_coded_width * s_config.i_coded_height );
	s_buffered_output_picture.pui8_cb = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );
	s_buffered_output_picture.pui8_cr = ( uint8_t * ) y262_alloc( sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );

	memset( s_picture.pui8_luma, 0, sizeof( uint8_t ) * s_config.i_coded_width * s_config.i_coded_height );
	memset( s_picture.pui8_cb, 0, sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );
	memset( s_picture.pui8_cr, 0, sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );

	memset( s_buffered_output_picture.pui8_luma, 0, sizeof( uint8_t ) * s_config.i_coded_width * s_config.i_coded_height );
	memset( s_buffered_output_picture.pui8_cb, 0, sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );
	memset( s_buffered_output_picture.pui8_cr, 0, sizeof( uint8_t ) * i_coded_chroma_width * i_coded_chroma_height );

	i_ret = y262_initialize( p_y262, &s_config );
	if( i_ret != Y262_INIT_SUCCESS )
	{
		fprintf( stderr, "y262 init failure ( errc: %d ), exiting\n", i_ret );
		return 1;
	}
	else
	{
		fprintf( stderr, "y262 init ok, %dx%d @ %f fps\n", i_width, i_height, d_timescale / d_picture_duration );
	}

	i64_input_frac_ticks = 0;

	i_field_count = i_idx = 0;
	while( 1 )
	{
		uint8_t rgui8_user_data[ 100 ];

		if( i_pad_x != 0 )
		{
			int32_t i_y;

			for( i_y = 0; i_y < i_height; i_y++ )
			{
				i_ret = fread( s_picture.pui8_luma + i_y * s_config.i_coded_width, sizeof( uint8_t ) * i_width, 1, f_in );
			}
			for( i_y = 0; i_y < i_chroma_height; i_y++ )
			{
				i_ret = fread( s_picture.pui8_cb + i_y * i_coded_chroma_width, sizeof( uint8_t ) * i_chroma_width, 1, f_in );
			}
			for( i_y = 0; i_y < i_chroma_height; i_y++ )
			{
				i_ret = fread( s_picture.pui8_cr + i_y * i_coded_chroma_width, sizeof( uint8_t ) * i_chroma_width, 1, f_in );
			}
		}
		else
		{
			i_ret = fread( s_picture.pui8_luma, sizeof( uint8_t ) * s_config.i_coded_width * i_height, 1, f_in );
			i_ret = fread( s_picture.pui8_cb, sizeof( uint8_t ) * i_coded_chroma_width * i_chroma_height, 1, f_in );
			i_ret = fread( s_picture.pui8_cr, sizeof( uint8_t ) * i_coded_chroma_width * i_chroma_height, 1, f_in );
		}
		if( i_pad_y != 0 )
		{
			int32_t i_y;
			for( i_y = i_height; i_y < s_config.i_coded_height; i_y++ )
			{
				memset( s_picture.pui8_luma + i_y * s_config.i_coded_width, 0, sizeof( uint8_t ) * s_config.i_coded_width );
			}
			for( i_y = i_chroma_height; i_y < i_coded_chroma_height; i_y++ )
			{
				memset( s_picture.pui8_cb + i_y * i_coded_chroma_width, 128, sizeof( uint8_t ) * i_coded_chroma_width );
				memset( s_picture.pui8_cr + i_y * i_coded_chroma_width, 128, sizeof( uint8_t ) * i_coded_chroma_width );
			}
		}
		if( i_ret <= 0 )
		{
			break;
		}

		if( s_config.i_frame_rate_code != s_config.i_pulldown_frame_rate_code )
		{
			int64_t i64_tinput_picture_duration, i64_tsequence_picture_duration;

			i64_tinput_picture_duration = ( i_input_picture_duration * i_sequence_timescale );
			i64_tsequence_picture_duration = ( i_sequence_picture_duration * i_input_timescale );
			i64_input_frac_picture_duration = i64_tsequence_picture_duration - i64_tinput_picture_duration;
			i64_tinput_picture_duration *= 2;
			i64_tsequence_picture_duration *= 2;
			i64_input_frac_picture_duration *= 2;

			i64_input_frac_ticks += i64_tsequence_picture_duration;

			if( i64_input_frac_ticks < i64_tinput_picture_duration )
			{
				i64_input_frac_ticks += i64_tsequence_picture_duration / 2;
				if( i64_input_frac_ticks < i64_tinput_picture_duration )
				{
					fprintf( stderr, "error: pulldown frame rate too high, ran out of repeatable fields\n");
					return -1;
				}
				else
				{
					i64_input_frac_ticks -= i64_tinput_picture_duration;
				}
				s_picture.i_frame_structure = Y262_INPUT_PICTURE_FRAME_PROGRESSIVE_REPEAT;
				i_field_count += 3;
			}
			else
			{
				i64_input_frac_ticks -= i64_tinput_picture_duration;
				s_picture.i_frame_structure = Y262_INPUT_PICTURE_FRAME_PROGRESSIVE;
				i_field_count += 2;
			}
		}
		else
		{
			if( s_config.b_interlaced )
			{
				s_picture.i_frame_structure = Y262_INPUT_PICTURE_FRAME_INTERLACED;
			}
			else
			{
				s_picture.i_frame_structure = Y262_INPUT_PICTURE_FRAME_PROGRESSIVE;
			}
			i_field_count += 2;
		}

		s_picture.i_num_user_data = 0;
		s_picture.b_start_new_gop = 0;

		/*
		if( i_idx == 50 )
		{
			s_picture.b_start_new_gop = 1;
			s_picture.i_num_user_data = 1;
			s_picture.rgi_user_data_len[ 0 ] = 100;
			memset( rgui8_user_data, 0xef, 100 );
			s_picture.rgpui8_user_data[ 0 ] = rgui8_user_data;
			s_picture.rgi_user_data_where[ 0 ] = Y262_USER_DATA_BEFORE_SLICES;
		}
		*/

		if( y262_push_input_picture( p_y262, &s_picture, i_idx ) != Y262_PUSH_INPUT_CONTINUE )
		{
			fprintf( stderr, "push input picture: some error\n");
			break;
		}
		i_idx++;
		if( i_idx == i_frames && i_frames > 0 )
		{
			break;
		}
	}

	do
	{
		i_ret = y262_push_input_picture( p_y262, NULL, i_idx );
		if( i_ret == Y262_PUSH_INPUT_FLUSHED )
		{
			break;
		}
	} while( i_ret == Y262_PUSH_INPUT_CONTINUE );
	
	if( i_ret != Y262_PUSH_INPUT_FLUSHED )
	{
		fprintf( stderr, "push input picture NULL: some error while flushing\n" );
	}

	y262_deinitialize( p_y262 );

	y262_dealloc( s_picture.pui8_luma );
	y262_dealloc( s_picture.pui8_cb );
	y262_dealloc( s_picture.pui8_cr );
	y262_dealloc( s_buffered_output_picture.pui8_luma );
	y262_dealloc( s_buffered_output_picture.pui8_cb );
	y262_dealloc( s_buffered_output_picture.pui8_cr );

	fprintf( stderr, "over %d frames:\n", i_idx );
	fprintf( stderr, "mean psnr: Y=%.2f CB=%.2f CR=%.2f\n", rgd_mean_psnr_accum[ 0 ] / d_num_mean_psnr, rgd_mean_psnr_accum[ 1 ] / d_num_mean_psnr, rgd_mean_psnr_accum[ 2 ] / d_num_mean_psnr );
	fprintf( stderr, "%.0f bytes out, %.2f kbit/sec @ %.4f fps\n", ( double )i64_bytes_num_out, ( ( ( ( ( double )i64_bytes_num_out ) * 8 ) / ( double )i_field_count ) * ( d_timescale / ( d_picture_duration / 2.0 ) ) ) / 1000.0, d_timescale / d_picture_duration );
	fflush( stderr );

#if DUMP_MEM_LEAKS
	_CrtDumpMemoryLeaks( );
#endif

	if( strcmp( ( char * ) pi8_infile, "-" ) != 0 )
	{
		fclose( f_in );
	}

	return 0;
}





