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

bool_t y262_ratectrl_init( y262_t *ps_y262 );
void y262_ratectrl_deinit( y262_t *ps_y262 );
void y262_ratectrl_start_picture( y262_t *ps_y262, int32_t i_bits_baseline );
void y262_ratectrl_end_picture( y262_t *ps_y262, int32_t i_bits );
bool_t y262_ratectrl_check_for_reencode( y262_t *ps_y262, int32_t i_bits );

void y262_ratectrl_start_slice_encoder( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc, int32_t i_start_mb_addr, int32_t i_end_mb_addr );
void y262_ratectrl_end_slice_encoder( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc );
int32_t y262_ratectrl_get_slice_mb_quantizer( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc, int32_t i_mb_addr );
void y262_ratectrl_update_slice_mb( y262_t *ps_y262, y262_slice_encoder_bitrate_control_t *ps_slice_rc, int32_t i_mb_addr, int32_t i_mb_bits );

int32_t y262_ratectrl_stuffing_bits_needed( y262_t *ps_y262 );
void y262_ratectrl_commit_stuffing_bits( y262_t *ps_y262, int32_t i_bits );