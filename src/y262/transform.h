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

int32_t y262_quant8x8_intra_fw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias );
void y262_quant8x8_intra_bw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat );


int32_t y262_quant8x8_inter_fw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat );
void y262_quant8x8_inter_bw( y262_t *ps_y262, int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, uint8_t *pui8_qmat );

int32_t y262_quant8x8_intra_fw_mpeg2( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat, uint16_t *pui16_bias );
int32_t y262_quant8x8_inter_fw_mpeg2( int16_t *pi_coeffs, int32_t i_stride, uint16_t *pui16_qmat );

int32_t y262_quant8x8_trellis_fw( y262_t *ps_y262, y262_slice_t *ps_slice, int16_t *pi_coeffs, int32_t i_stride, int32_t i_quantizer, bool_t b_intra );

void y262_fdct_c( int16_t *pi16_block, int16_t *pi16_dst );
void y262_idct_c( int16_t *pi16_block, int16_t *pi16_dst );

