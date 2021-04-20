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



#include "y262.h"

#if defined( WIN32 ) || defined( WIN64 )
#include <windows.h>
#include <process.h>
#elif defined( HAVE_LIBPTHREAD )
#include <pthread.h>

typedef struct
{
	pthread_t s_thread;
	y262_thread_f *pf_func;
	void *p_arg;
} y262_pthread_thread_t;

void *y262_pthread_func( void *p_arg )
{
	y262_pthread_thread_t *ps_thread = ( y262_pthread_thread_t * ) p_arg;
	ps_thread->pf_func( ps_thread->p_arg );
	return NULL;
}

#endif

bool_t y262_can_do_threads( )
{
#if defined( WIN32 ) || defined( WIN64 )
	return TRUE;
#elif defined( HAVE_LIBPTHREAD )
	return TRUE;
#else
	return FALSE;
#endif
}


void *y262_create_thread( y262_t *ps_y262, y262_thread_f *pf_func, void *p_arg )
{
#if defined( WIN32 ) || defined( WIN64 )
	if( _beginthread( pf_func, 0, p_arg ) == -1 )
	{
		return 0;
	}
	return (void *) 0x1;
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_thread_t *ps_thread;
	ps_thread = y262_alloc( sizeof( y262_pthread_thread_t ) );
	ps_thread->pf_func = pf_func;
	ps_thread->p_arg = p_arg;
	if( pthread_create( &ps_thread->s_thread, 0x0, y262_pthread_func, ps_thread ) )
	{
		y262_dealloc( ps_thread );
		return NULL;
	}
	return ps_thread;
#else

	return NULL;
#endif
}


void y262_join_thread( y262_t *ps_y262, void *p_thread )
{
#if defined( WIN32 ) || defined( WIN64 )
	/* windows cannot join threads */
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_thread_t *ps_thread = ( y262_pthread_thread_t *)p_thread;
	pthread_join( ps_thread->s_thread, NULL );
	y262_dealloc( ps_thread );
#endif
}


void *y262_create_mutex( y262_t *ps_y262 )
{
#if defined( WIN32 ) || defined( WIN64 )
	CRITICAL_SECTION *ps_cs;
	ps_cs = ( CRITICAL_SECTION * ) y262_alloc( sizeof( CRITICAL_SECTION ) );
	if( ps_cs != NULL )
	{
		InitializeCriticalSection( ps_cs );
	}
	else
	{
		return NULL;
	}
	return ( void * )ps_cs;
#elif defined( HAVE_LIBPTHREAD )
	pthread_mutex_t *ps_mutex;
	ps_mutex = ( pthread_mutex_t * )y262_alloc( sizeof( pthread_mutex_t ) );
	if( ps_mutex == NULL )
	{
		return NULL;
	}
	if( pthread_mutex_init( ps_mutex, NULL ) )
	{
		y262_dealloc( ps_mutex );
		return NULL;
	}
	return ps_mutex;
#else
	return NULL;
#endif
}


void y262_destroy_mutex( y262_t *ps_y262, void *p_cs )
{
#if defined( WIN32 ) || defined( WIN64 )
	CRITICAL_SECTION *ps_cs = ( CRITICAL_SECTION * ) p_cs;
	if( p_cs != NULL )
	{
		DeleteCriticalSection( ps_cs );
		y262_dealloc( p_cs );
	}
#elif defined( HAVE_LIBPTHREAD )
	pthread_mutex_t *ps_mutex = ( pthread_mutex_t * ) p_cs;
	if( ps_mutex )
	{
		pthread_mutex_destroy( ps_mutex );
		y262_dealloc( ps_mutex );
	}
#else
#endif
}


void y262_mutex_lock( y262_t *ps_y262, void *p_cs )
{
#if defined( WIN32 ) || defined( WIN64 )
	CRITICAL_SECTION *ps_cs = ( CRITICAL_SECTION * ) p_cs;
	if( ps_cs != NULL )
	{
		EnterCriticalSection( ps_cs );
	}
#elif defined( HAVE_LIBPTHREAD )
	pthread_mutex_t *ps_mutex = ( pthread_mutex_t * ) p_cs;
	if( ps_mutex )
	{
		pthread_mutex_lock( ps_mutex );
	}
#else
#endif
}


void y262_mutex_unlock( y262_t *ps_y262, void *p_cs )
{
#if defined( WIN32 ) || defined( WIN64 )
	CRITICAL_SECTION *ps_cs = ( CRITICAL_SECTION * ) p_cs;
	if( ps_cs != NULL )
	{
		LeaveCriticalSection( ps_cs );
	}
#elif defined( HAVE_LIBPTHREAD )
	pthread_mutex_t *ps_mutex = ( pthread_mutex_t * ) p_cs;
	if( ps_mutex )
	{
		pthread_mutex_unlock( ps_mutex );
	}
#else
#endif
}


#if defined( HAVE_LIBPTHREAD )
typedef struct {
	volatile int i_triggered;
	pthread_cond_t s_cond;
	pthread_mutex_t s_mutex;
} y262_pthread_event_t;
#endif


/* pthread impl, if any, will suffer */
void *y262_create_event( y262_t *ps_y262 )
{
#if defined( WIN32 ) || defined( WIN64 )
	HANDLE ps_event;
	ps_event = CreateEvent( NULL, FALSE, FALSE, NULL );
	return ps_event;
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_event_t *ps_event;
	ps_event = y262_alloc( sizeof( y262_pthread_event_t ) );
	if( ps_event == NULL )
	{
		return NULL;
	}
	ps_event->i_triggered = 0;
	if( pthread_cond_init( &ps_event->s_cond, NULL ) )
	{
		free( ps_event );
		return NULL;
	}
	if( pthread_mutex_init( &ps_event->s_mutex, NULL ) )
	{
		pthread_cond_destroy( &ps_event->s_cond );
		free( ps_event );
		return NULL;
	}
	return ps_event;
#else
	return NULL;
#endif
}

void y262_destroy_event( y262_t *ps_y262, void *p_event )
{
#if defined( WIN32 ) || defined( WIN64 )
	CloseHandle( ( HANDLE )p_event );
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_event_t *ps_event = ( y262_pthread_event_t * )p_event;;
	pthread_mutex_destroy( &ps_event->s_mutex );
	pthread_cond_destroy( &ps_event->s_cond );
	y262_dealloc( ps_event );
#else
#endif
}

void y262_event_wait_( y262_t *ps_y262, void *p_event )
{
#if defined( WIN32 ) || defined( WIN64 )
	WaitForSingleObject( ( HANDLE )p_event, INFINITE );
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_event_t *ps_event = ( y262_pthread_event_t * )p_event;;
	pthread_mutex_lock( &ps_event->s_mutex );
	if( ps_event->i_triggered == 0 )
	{
		pthread_cond_wait( &ps_event->s_cond, &ps_event->s_mutex );
	}
	ps_event->i_triggered = 0;
	pthread_mutex_unlock( &ps_event->s_mutex );
#else
#endif
}

void y262_event_set_( y262_t *ps_y262, void *p_event )
{
#if defined( _WIN32 ) || defined( _WIN64 )
	SetEvent( ( HANDLE )p_event );
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_event_t *ps_event = ( y262_pthread_event_t * )p_event;;
	pthread_mutex_lock( &ps_event->s_mutex );
	ps_event->i_triggered = 1;
	pthread_cond_broadcast( &ps_event->s_cond );
	pthread_mutex_unlock( &ps_event->s_mutex );
#else
#endif
}

void y262_event_wait_g( y262_t *ps_y262, void *p_event )
{
#if defined( WIN32 ) || defined( WIN64 )
	WaitForSingleObject( ( HANDLE ) p_event, INFINITE );
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_event_t *ps_event = ( y262_pthread_event_t * ) p_event;;
	pthread_mutex_lock( ps_y262->p_resource_mutex );
	if( ps_event->i_triggered == 0 )
	{
		pthread_cond_wait( &ps_event->s_cond, ps_y262->p_resource_mutex );
	}
	ps_event->i_triggered = 0;
	pthread_mutex_unlock( ps_y262->p_resource_mutex );
#else
#endif
}

void y262_event_set_g( y262_t *ps_y262, void *p_event )
{
#if defined( _WIN32 ) || defined( _WIN64 )
	SetEvent( ( HANDLE ) p_event );
#elif defined( HAVE_LIBPTHREAD )
	y262_pthread_event_t *ps_event = ( y262_pthread_event_t * ) p_event;;
	pthread_mutex_lock( ps_y262->p_resource_mutex );
	ps_event->i_triggered = 1;
	pthread_cond_broadcast( &ps_event->s_cond );
	pthread_mutex_unlock( ps_y262->p_resource_mutex );
#else
#endif
}



void y262_slice_thread( void *p_arg )
{
	int32_t i_idx;
	y262_slice_thread_t *ps_slice_thread;
	y262_t *ps_slice_encoder;

	ps_slice_thread = ( y262_slice_thread_t * ) p_arg;
	ps_slice_encoder = ps_slice_thread->ps_y262->rgps_slice_encoders[ ps_slice_thread->i_slice_encoder_idx ];

	while( 1 )
	{
		y262_event_wait_g( ps_slice_thread->ps_y262, ps_slice_thread->p_start_event );
		if( ps_slice_thread->i_command == Y262_SLICE_THREAD_CMD_LOOKAHEAD )
		{
			y262_lookahead_analyze_slice( ps_slice_thread->ps_y262, ps_slice_thread->ps_pic, ps_slice_thread->ps_fw_ref,
				ps_slice_thread->ps_bw_ref, ps_slice_thread->i_first_slice_row, ps_slice_thread->i_last_slice_row );
			y262_event_set_g( ps_slice_thread->ps_y262, ps_slice_thread->p_finished_event );
		}
		else if( ps_slice_thread->i_command == Y262_SLICE_THREAD_CMD_ENCODE )
		{
			y262_bitstream_reset( &ps_slice_encoder->s_bitstream );
			for( i_idx = ps_slice_thread->i_first_slice_row; i_idx <= ps_slice_thread->i_last_slice_row; i_idx++ )
			{
				y262_encode_unit_slice( ps_slice_encoder, ps_slice_thread->i_picture_type, i_idx );
				y262_bitstream_bytealign( &ps_slice_encoder->s_bitstream );
			}
			y262_event_set_g( ps_slice_thread->ps_y262, ps_slice_thread->p_finished_event );
		}
		else if( ps_slice_thread->i_command == Y262_SLICE_THREAD_CMD_EXIT )
		{
			break;
		}
	}
	y262_event_set_g( ps_slice_thread->ps_y262, ps_slice_thread->p_finished_event );
	return;
}


void y262_lookahead_thread( void *p_arg )
{
	y262_slice_thread_t *ps_slice_thread;

	ps_slice_thread = ( y262_slice_thread_t * ) p_arg;

	while( 1 )
	{
		y262_event_wait_g( ps_slice_thread->ps_y262, ps_slice_thread->p_start_event );
		if( ps_slice_thread->i_command == Y262_SLICE_THREAD_CMD_LOOKAHEAD )
		{
			y262_lookahead_analyze_slice( ps_slice_thread->ps_y262, ps_slice_thread->ps_pic, ps_slice_thread->ps_fw_ref,
				ps_slice_thread->ps_bw_ref, ps_slice_thread->i_first_slice_row, ps_slice_thread->i_last_slice_row );
			y262_event_set_g( ps_slice_thread->ps_y262, ps_slice_thread->p_finished_event );
		}
		else if( ps_slice_thread->i_command == Y262_SLICE_THREAD_CMD_EXIT )
		{
			break;
		}
	}
	y262_event_set_g( ps_slice_thread->ps_y262, ps_slice_thread->p_finished_event );
	return;
}

void y262_main_lookahead_thread( void *p_arg )
{
	y262_t *ps_y262 = ( y262_t *)p_arg;

	while( 1 )
	{
		y262_event_wait_g( ps_y262, ps_y262->s_lookahead_thread.p_start_event );
		if( ps_y262->s_lookahead_thread.i_command == Y262_LOOKAHEAD_THREAD_CMD_LOOKAHEAD )
		{
			y262_process_lookahead_internal( ps_y262 );
			y262_event_set_g( ps_y262, ps_y262->s_lookahead_thread.p_finished_event );
		}
		else if( ps_y262->s_lookahead_thread.i_command == Y262_SLICE_THREAD_CMD_EXIT )
		{
			break;
		}
	}
	y262_event_set_g( ps_y262, ps_y262->s_lookahead_thread.p_finished_event );
	return;

}

