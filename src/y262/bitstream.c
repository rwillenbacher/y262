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


void y262_bitstream_init( y262_bitstream_t *ps_bitstream, int32_t i_length )
{
	ps_bitstream->pui8_bitstream = ( uint8_t * )y262_alloc( sizeof( uint8_t ) * i_length );
	ps_bitstream->i_length = i_length;

	ps_bitstream->i_byte_count = 0;
	ps_bitstream->i_next_bit = 7;

	ps_bitstream->pui8_codeword_ptr = ps_bitstream->pui8_bitstream;
	ps_bitstream->ui_codeword = 0;
	ps_bitstream->i_codeword_fill = 0;
}

void y262_bitstream_deinit( y262_bitstream_t *ps_bitstream )
{
	y262_dealloc( ps_bitstream->pui8_bitstream );
}

void y262_bitstream_advance( y262_bitstream_t *ps_bitstream )
{
	ps_bitstream->pui8_codeword_ptr = ps_bitstream->pui8_bitstream;
}

void y262_bitstream_reset( y262_bitstream_t *ps_bitstream )
{
	ps_bitstream->pui8_codeword_ptr = ps_bitstream->pui8_bitstream;
	ps_bitstream->ui_codeword = 0;
	ps_bitstream->i_codeword_fill = 0;
	ps_bitstream->i_byte_count = 0;
	ps_bitstream->i_next_bit = 7;
}


void y262_bitstream_write( y262_bitstream_t *ps_bitstream, uint32_t ui_code, uint32_t ui_length )
{
	const uint32_t rgui_mask[ 25 ] = {
		0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f,
		0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff,
		0xffff, 0x1ffff, 0x3ffff, 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff,
		0xffffff
	};
	ps_bitstream->ui_codeword |= ( ui_code & rgui_mask[ ui_length ] ) << ( 32 - ui_length - ps_bitstream->i_codeword_fill );
	ps_bitstream->i_codeword_fill += ui_length;

	while( ps_bitstream->i_codeword_fill > 7 )
	{
		*( ps_bitstream->pui8_codeword_ptr++ ) = ( ps_bitstream->ui_codeword >> 24 );
		ps_bitstream->ui_codeword <<= 8;
		ps_bitstream->i_codeword_fill -= 8;
	}
}


void y262_bitstream_get( y262_bitstream_t *ps_bitstream, uint8_t **ppui8_bitstream, uint32_t *pui_length )
{
	*ppui8_bitstream = ps_bitstream->pui8_bitstream;
	*pui_length = ( int32_t )( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream );
}

void y262_bitstream_flush( y262_bitstream_t *ps_bitstream, uint8_t **ppui8_bitstream, uint32_t *pui_length )
{
	*ppui8_bitstream = ps_bitstream->pui8_bitstream;

	if( ps_bitstream->i_codeword_fill )
	{
		*( ps_bitstream->pui8_codeword_ptr++ ) = ( ps_bitstream->ui_codeword >> 24 );
		*pui_length = ( int32_t )( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream );
	}
	else
	{
		*pui_length = ( int32_t )( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream );
	}
}


int32_t y262_bitstream_bits( y262_bitstream_t *ps_bitstream )
{
	return ( int32_t )( ( ps_bitstream->pui8_codeword_ptr - ps_bitstream->pui8_bitstream ) * 8 ) + ( ps_bitstream->i_codeword_fill );
}

void y262_bitstream_bytealign( y262_bitstream_t *ps_bitstream )
{
	if( ps_bitstream->i_codeword_fill )
	{
		*( ps_bitstream->pui8_codeword_ptr++ ) = ( ps_bitstream->ui_codeword >> 24 );
		ps_bitstream->i_codeword_fill = 0;
		ps_bitstream->ui_codeword <<= 8;
	}
}

