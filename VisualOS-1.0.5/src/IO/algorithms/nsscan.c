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
#include "nsscan.h"

static gint nsscan_select (void);
static gint nsscan_unselect (void);
static gint nsscan_event (io_request_t *request);
static gint nsscan_done_reading (void);

static io_algorithm_t nsscan_algorithm = {
	"N Step Scan",
	nsscan_select,
	nsscan_unselect,
	NULL,/*properties WidGet*/
	nsscan_event,
	nsscan_done_reading
};
static const gint num_algorithm_params = 1;
static property_t algorithm_params[] = {
	{N_("Step:"), 1, 99, 1},
};
static const gfloat default_params[]={
	4,	/* Step */
};
static properties_t *properties;
static gint step=4;
static void nsscan_property_change_notify(void)
{
	gfloat *value;

	value = properties_get_values(properties);
	step=value[0];
	
}
gint io_nsscan_init (void)
{
	register_IO_algorithm (&nsscan_algorithm);
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
static gint insert_n_requests(io_queue_t requested, io_queue_t * reading, 
				 gint n)
{
	gint count=0;

	while (!io_queue_end(requested) && count++ < step){
		insert_request(io_request_data(requested), reading);
		requested = io_queue_next(requested);
	}
	return count;
}	
static gint nsscan_select (void)
{
	io_queue_t requested=get_IO_requested_queue();
	DECLARE_IO_QUEUE(reading);
		
	io_queue_erase(get_IO_reading_queue());
	
	insert_n_requests(requested, &reading, step);
	set_IO_reading_queue(reading);

	properties = properties_create (algorithm_params,
			num_algorithm_params,
			nsscan_property_change_notify);
	properties_set_values(properties, default_params);
	nsscan_algorithm.properties = properties_get_widget(properties);
	return 0;
}
static gint nsscan_unselect (void)
{
	properties_destroy (properties);
	nsscan_algorithm.properties = NULL;
	return 0;
}
static gint nsscan_event (io_request_t *new_request)
{
	return 0;
}
static gint nsscan_done_reading (void)
{
	io_queue_t requested=get_IO_requested_queue();
	DECLARE_IO_QUEUE(reading);
	g_return_val_if_fail(io_queue_empty(get_IO_reading_queue()), 0);
	
	insert_n_requests(requested, &reading, step);

	set_IO_reading_queue(reading);
	return 0;
}
