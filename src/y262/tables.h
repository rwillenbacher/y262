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

extern int32_t rgi_y262_framerate_code_duration[ 16 ];
extern int32_t rgi_y262_framerate_code_timescale[ 16 ];

extern uint8_t rgui8_y262_default_intra_matrix[ 64 ];
extern uint8_t rgui8_y262_default_non_intra_matrix[ 64 ];

extern int32_t rgui_y262_luma_blk_offsets[ 4 ][ 2 ];
extern int32_t rgui_y262_luma_il_blk_offsets[ 4 ][ 2 ];
extern int32_t rgui_num_chroma_blk[ 4 ];
extern int32_t rgui_y262_chroma_blk_offsets[ 4 ][ 4 ][ 2 ];
extern int32_t rgui_y262_chroma_il_blk_offsets[ 4 ][ 4 ][ 2 ];
extern int32_t rgi_y262_block_type_dims[  ][ 2 ];

extern y262_vlc_t rgs_y262_macroblock_address_increment_table[ ];
extern y262_vlc_t rgs_y262_macroblock_type_i_picture_table[ ];
extern int32_t rgui_y262_macroblock_type_i_picture_flags_table[ ];
extern y262_vlc_t rgs_y262_macroblock_type_p_picture_table[ ];
extern int32_t rgui_y262_macroblock_type_p_picture_flags_table[ ];
extern y262_vlc_t rgs_y262_macroblock_type_b_picture_table[ ];
extern int32_t rgui_y262_macroblock_type_b_picture_flags_table[ ];

extern y262_vlc_t rgs_y262_dct_dc_size_luminance_table[ ];
extern int32_t rgi_y262_dct_dc_size_luminance_lookup_table[ ];
extern y262_vlc_t rgs_y262_dct_dc_size_chrominance_table[ ];
extern int32_t rgi_y262_dct_dc_size_chrominance_lookup_table[ ];

extern y262_vlc_t rgs_y262_dct_coefficients_table_zero[ ];
extern y262_run_level_t rgs_y262_dct_coefficients_lookup_table_zero[ ];
extern int8_t rgi_y262_run_level_bits_zero[ 32 ][ 41 ];

extern y262_vlc_t rgs_y262_dct_coefficients_table_one[ ];
extern y262_run_level_t rgs_y262_dct_coefficients_lookup_table_one[ ];

extern int8_t rgi8_y262_quantiser_scale_table[ 2 ][ 32 ];

extern uint8_t rgui8_y262_scan_0_table[ ];
extern uint8_t rgui8_y262_scan_1_table[ ];

extern y262_vlc_t rgs_y262_motion_code_table[ ];
extern int32_t rgi_y262_motion_bits[ 17 ];
extern int32_t rgi_y262_motion_delta_lookup_table[ ];

extern y262_vlc_t rgs_y262_coded_block_pattern_table[ ];
extern int32_t rgi_y262_coded_block_pattern_lookup_table[ ];

