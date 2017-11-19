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

#include <glib.h>
#include <stdlib.h>

#include <messaging.h>

#include "main.h"
#include "CLOCK.h"

static tick_callback_t *tick_callbacks = NULL;
static gint registered=FALSE;
static gint time=0;
/**
 * get_time:
 *
 * returns: the current time in "time units".
 */
gint get_time(void)
{
	return time;
}

static void clock_tick(const message_t *m)
{
	gint i;

	if (tick_callbacks==NULL)
		return;
	time = *(gint*)(m->data);
	for (i=0; tick_callbacks[i] != NULL; ++i)
			tick_callbacks[i](time);
}
/**
 * clock_register_tick:
 * @func: function to be called as time goes by.
 *
 * Instructs the CLOCK subsystem to call func for every "time unit".
 *
 * returns: nothing important.
 */
gint clock_register_tick(tick_callback_t func)
{
	static gint n_callbacks=0;
	
	tick_callbacks = g_renew(tick_callback_t, tick_callbacks, 
				 ++n_callbacks+1);
	tick_callbacks[n_callbacks-1] = func;
	tick_callbacks[n_callbacks] = NULL;

	if (!registered){
		mesg_callback_register(TICK, clock_tick);
		registered=TRUE;
	}
	return 0;
}
/**
 * CLOCK_stop:
 *
 * Tells the CLOCK to stop counting "time units".
 */
void CLOCK_stop(void)
{
	mesg_send(CLOCK, CLOCK_STOP, NULL, 0);
}
void CLOCK_server_init(void)
{
	mesg_callback_register(CLOCK_STOP, (receive_callback)clock_server_stop);
}
