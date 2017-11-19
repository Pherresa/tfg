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
#include "scan.h"

static gint scan_select (void);
static gint scan_unselect (void);
static gint scan_event (io_request_t *request);
static gint scan_done_reading (void);

static io_algorithm_t scan_algorithm = {
	"Scan",
	scan_select,
	scan_unselect,
	NULL,/*properties WidGet*/
	scan_event,
	scan_done_reading
};

gint io_scan_init (void)
{
	register_IO_algorithm (&scan_algorithm);
	return 0;
}
static void insert_request(io_request_t *new_request, io_queue_t *queue)
{
	io_queue_t item=*queue;
	gint head_pos=get_IO_head_pos();
	io_request_t *request = io_queue_empty(item) ?
					NULL : io_request_data(item);
	gint next_pos = request!=NULL? request->track : head_pos;
	/* "dir" is a representation for the direction of the head*/
	gint dir = head_pos < next_pos? 1 : -1;
	gint i=0;
	/* if the new request is on the wrong side of the head pass over 
	 * all the requests which are on the right side */
	if (new_request->track*dir < head_pos*dir){
		while (!io_queue_end(item)){
			request = io_request_data(item);
			if (request->track*dir < head_pos*dir)
				break;
			++i;
			head_pos=request->track;
			item=io_queue_next(item);
		}
	}
	/* now we can just insert the request sorted */
	while (!io_queue_end(item)){
		request=io_request_data(item);
		if(abs(new_request->track - head_pos) <
				abs(request->track - head_pos)){
			break;
		}
		++i;
		head_pos=request->track;
		item=io_queue_next(item);
	}
	io_queue_insert(*queue, new_request, i);
}
static gint scan_select (void)
{
	DECLARE_IO_QUEUE(reading);
		
	io_queue_erase(get_IO_reading_queue());
	
	io_queue_foreach(get_IO_requested_queue(),
			insert_request, &reading);
	set_IO_reading_queue(reading);

	return 0;
}
static gint scan_unselect (void)
{
	return 0;
}
static gint scan_event (io_request_t *new_request)
{
	io_queue_t reading=get_IO_reading_queue();
	
	insert_request(new_request, &reading);

	set_IO_reading_queue(reading);
	return 0;
}
static gint scan_done_reading (void)
{
	return 0;
}



