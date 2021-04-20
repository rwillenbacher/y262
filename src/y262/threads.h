/*
Copyright (c) 2013,2016, Ralf Willenbacher
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



bool_t y262_can_do_threads( );

void *y262_create_thread( y262_t *ps_y262, y262_thread_f *pf_func, void *p_arg );
void y262_join_thread( y262_t *ps_y262, void *p_thread );

void *y262_create_mutex( y262_t *ps_y262 );
void y262_destroy_mutex( y262_t *ps_y262, void *p_cs );
void y262_mutex_lock( y262_t *ps_y262, void *p_cs );
void y262_mutex_unlock( y262_t *ps_y262, void *p_cs );

void *y262_create_event( y262_t *ps_y262 );
void y262_destroy_event( y262_t *ps_y262, void *p_event );
void y262_event_wait_( y262_t *ps_y262, void *p_event );
void y262_event_set_( y262_t *ps_y262, void *p_event );
void y262_event_wait_g( y262_t *ps_y262, void *p_event );
void y262_event_set_g( y262_t *ps_y262, void *p_event );

void y262_slice_thread( void *p_arg );
void y262_lookahead_thread( void *p_arg );
void y262_main_lookahead_thread( void *p_arg );
void y262_encode_unit_slice( y262_t *ps_y262, int32_t i_picture_type, int32_t i_slice_row ); /* not in threads.c */