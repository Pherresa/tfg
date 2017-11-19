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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gtk/gtk.h>
#include <math.h>

#include "algorithm_api.h"
#include "sstf.h"

static gint sstf_select (void);
static gint sstf_unselect (void);
static gint sstf_event (io_request_t *request);
static gint sstf_done_reading (void);

static io_algorithm_t sstf_algorithm = {
	"Shortest Seek Time First",
	sstf_select,
	sstf_unselect,
	NULL,/*properties WidGet*/
	sstf_event,
	sstf_done_reading
};

gint io_sstf_init (void)
{
	register_IO_algorithm (&sstf_algorithm);
	return 0;
}
static gint sstf_select (void)
{
	sstf_event(NULL);
	return 0;
}
static gint sstf_unselect (void)
{
	return 0;
}
/* this function is called with a NULL argument from "sstf_select" */
static gint sstf_event (io_request_t *new_request)
{
	io_queue_t requested=io_queue_dup(get_IO_requested_queue());
	DECLARE_IO_QUEUE(reading);
	io_queue_t item=reading;
	gint head_pos=get_IO_head_pos();

	io_queue_erase(get_IO_reading_queue());
	/* until requested gets empty */
	while (!io_queue_end(requested)){
		io_request_t *request=io_request_data(requested);
		item = requested;
		/* search for the shortest seek */
		while (!io_queue_end(item)){
			io_request_t *aux_request=io_request_data(item);
			if(abs(aux_request->track - head_pos) <
					abs(request->track - head_pos))
				request=aux_request;
			item=io_queue_next(item);
		}
		/* note that we are done with this one */
		requested=io_queue_remove(requested, request);
		/* put it in the list to be read */
		reading=io_queue_append(reading, request);
		head_pos=request->track;
	}

	set_IO_reading_queue(reading);
	return 0;
}
static gint sstf_done_reading (void)
{
	return 0;
}
