/* VisualOS is an educational visual simulator of an operating system.   
   Copyright (C) 2000,2004 Manuel Estrada Sainz <ranty@debian.org>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

*/

#include <stdlib.h>
#include <glib.h>

#include <messaging.h>
#include <SCHED.h>

#include "drawings/main.h"
#include "geometry.h"
#include "simulation.h"
#include "queues.h"
#include "info.h"
#include "IO.h"

/* this is the list of all requested blocks */
static DECLARE_IO_QUEUE(requested);
/* this is the list of blocks to be read */
static DECLARE_IO_QUEUE(reading);
/* current head position */
static gint head_pos=0;
/* track of next block to read */
static gint next_track=0;

/**
 * get_IO_head_pos:
 *
 * returns: the track number over which the head is currently flying.
 */
gint get_IO_head_pos(void)
{
	return head_pos;
}
/**
 * get_IO_reading_queue:
 *
 * Get all pending requests as ordered by the current algoritym.
 *
 * returns: the request's "reading" queue (ordered by the current algorithm).
 */
io_queue_t get_IO_reading_queue(void)
{
	return reading;
}
/**
 * set_IO_reading_queue:
 * @new_reading: newly ordered "reading" queue.
 *
 * Sets the request's "reading" queue.
 *
 * This function should be called when ever the reading queue is modified by
 * external means, even if the pointer to the queue is not changed.
 */
void set_IO_reading_queue(io_queue_t new_reading)
{
	reading=new_reading;
	if (!io_queue_empty(reading))
		next_track = io_request_data(reading)->track;

	redraw_IO_drawing();
	update_queue_entries();
}
/**
 * get_IO_requested_queue:
 *
 * Get all pending requests in order of arrival.
 *
 * returns: the request's "requested" queue (in chronological ordered).
 */
io_queue_t get_IO_requested_queue(void)
{
	return requested;
}

static gint simulation_sched (void)
{
	if (io_queue_empty(reading)){
		get_IO_current_algorithm()->done_reading();
		return 0;
	}

	if (head_pos == next_track){
		io_request_t * request = io_request_data(reading);

		io_queue_remove(reading, request);
		io_queue_remove(requested, request);
		io_block_ready_server(request);
		update_queue_entries();
		g_free(request);
		if (!io_queue_empty(reading))
			next_track = io_request_data(reading)->track;
	} else if (head_pos < next_track)
		++head_pos;
	else
		--head_pos;
	redraw_IO_drawing();
		
	return 0;
}
/**
 * init_IO_simulation:
 *
 * initialices the IO simulation code.
 *
 * returns: nothing important.
 */
gint init_IO_simulation (void)
{
	gint ntracks = get_IO_ntracks();

	head_pos = ntracks/2;
	redraw_IO_drawing();
	sched_delay(1, (sched_callback_t) simulation_sched, NULL,
			SCHED_RELOAD);
	return 0;
}
/**
 * IO_algorithm_event:
 * @request: request to be inserted.
 *
 * Insert a new request in the IO subsystem using the current algorith.
 *
 * Mainly passes @request over to the current algorithm and puts it the the queue
 * of requested blocks.
 *
 * returns: nothing important.
 */
gint IO_algorithm_event(io_request_t *request)
{
	io_algorithm_t *current = get_IO_current_algorithm();

	io_queue_append(requested, request);
	current->event(request);
	update_queue_entries();
	return 0;
}
