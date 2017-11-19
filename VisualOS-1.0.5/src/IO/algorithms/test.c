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
#include "test.h"

static gint test_select (void);
static gint test_unselect (void);
static gint test_event (io_request_t *request);
static gint test_done_reading (void);

static io_algorithm_t test_algorithm = {
	"Test",
	test_select,
	test_unselect,
	NULL,/*properties WidGet*/
	test_event,
	test_done_reading
};
static const gint num_algorithm_params = 2;
static property_t algorithm_params[] = {
	{"Property 0:", 1, 10, 1},
	{"Property 1:", 1, 99, 1},
};
static const gfloat default_params[]={
	4,	/* Property 0 */
	4,	/* Property 1 */
};
static properties_t *properties;
	
static void test_property_change_notify(void)
{
	gint i;
	gfloat *value;

	value = properties_get_values(properties);
	g_print("test: current param values are:\n");
	for (i=0; i < num_algorithm_params; ++i)
			g_print("\t %s %f\n",algorithm_params[i].label,
					value[i]);
}

gint io_test_init (void)
{
	register_IO_algorithm (&test_algorithm);
	return 0;
}
static gint test_select (void)
{
	properties = properties_create (algorithm_params,
			num_algorithm_params,
			test_property_change_notify);
	properties_set_values(properties, default_params);

	test_algorithm.properties = properties_get_widget(properties);
	return 0;
}
static gint test_unselect (void)
{
	properties_destroy (properties);
	test_algorithm.properties = NULL;
	return 0;
}
static gint test_event (io_request_t *request)
{
	io_queue_t reading=get_IO_reading_queue();
#ifdef DEBUG
	g_print ("test_event: event for block %d\n",request->block);
#endif
	reading = io_queue_append(reading, request);
	set_IO_reading_queue(reading);
	return 0;
}
static gint test_done_reading (void)
{
#ifdef DEBUG
	g_print ("test_event: read queue empty\n");
#endif
	return 0;
}
