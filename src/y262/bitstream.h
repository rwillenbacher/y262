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



void y262_bitstream_init( y262_bitstream_t *ps_bitstream, int32_t i_length );
void y262_bitstream_advance( y262_bitstream_t *ps_bitstream );
void y262_bitstream_reset( y262_bitstream_t *ps_bitstream );
void y262_bitstream_deinit( y262_bitstream_t *ps_bitstream );

void y262_bitstream_write( y262_bitstream_t *ps_bitstream, uint32_t ui_code, uint32_t ui_length );

void y262_bitstream_get( y262_bitstream_t *ps_bitstream, uint8_t **ppui8_bitstream, uint32_t *ui_length );

int32_t y262_bitstream_bits( y262_bitstream_t *ps_bitstream );

void y262_bitstream_flush( y262_bitstream_t *ps_bitstream, uint8_t **ppui8_bitstream, uint32_t *pui_length );

void y262_bitstream_bytealign( y262_bitstream_t *ps_bitstream );