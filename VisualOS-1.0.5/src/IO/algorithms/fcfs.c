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

#include "algorithm_api.h"
#include "fcfs.h"

static gint fcfs_select (void);
static gint fcfs_unselect (void);
static gint fcfs_event (io_request_t *request);
static gint fcfs_done_reading (void);

static io_algorithm_t fcfs_algorithm = {
	"First Come First Served",
	fcfs_select,
	fcfs_unselect,
	NULL,/*properties WidGet*/
	fcfs_event,
	fcfs_done_reading
};
	
gint io_fcfs_init (void)
{
	register_IO_algorithm (&fcfs_algorithm);
	return 0;
}
static gint fcfs_select (void)
{
	io_queue_t requested=io_queue_dup(get_IO_requested_queue());
	io_queue_erase(get_IO_reading_queue());
	set_IO_reading_queue(requested);
	return 0;
}
static gint fcfs_unselect (void)
{
	return 0;
}
static gint fcfs_event (io_request_t *request)
{
	io_queue_t reading=get_IO_reading_queue();

	reading = io_queue_append(reading, request);
	set_IO_reading_queue(reading);
	return 0;
}
static gint fcfs_done_reading (void)
{
	return 0;
}
