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
#include <glib.h>
#include <stdlib.h>

#include <CLOCK/CLOCK.h>
#include <SCHED.h>

typedef struct sched_s {
	gint time;
	gint flags;
	gint delay;
	sched_callback_t func;
	gpointer data;
} sched_t;

static GSList *wait_queue=NULL;
static gint time=0;

static gint time_compare (sched_t *a, sched_t *b)
{ 
	return a->time - b->time;
}
static gint sched_clock_callback(gint clock_time)
{
	time=clock_time;
	while (wait_queue != NULL){
		sched_t *sched=(sched_t *)wait_queue->data;
#ifdef DEBUG
		if(sched->time < time)
			g_warning("sched: time in the past was found\n");
#endif
		if(sched->time <= time){
			sched->func(time, sched->data);
			wait_queue = g_slist_remove(wait_queue, sched);
			
			if(sched->flags & FREE_SCHED_DATA)
				g_free(sched->data);
			if((sched->flags & SCHED_RELOAD)
					&& (sched->delay!=0)){
				sched->time=time+sched->delay;
				wait_queue = g_slist_insert_sorted(wait_queue,
					sched, (GCompareFunc)time_compare);
			} else
				g_free(sched);
				
		} else
			break;
	}
	return 0;
}
/**
 * sched_init: Initializes the scheduling facility.
 *
 * Initializes the scheduling facility. Mainly registering a callback for the 
 * clock subsystem.
 *
 * Returns: nothing interesting.
 */
gint sched_init(void)
{
	clock_register_tick(sched_clock_callback);
	return 0;
}
/**
 * sched_event: schedules an event at absolute time.
 * @sched_time: absolute time as seen by the clock subsystem.
 * @func: function to be called when the time comes.
 * @data: will be used as the second argument to @func.
 * @flags: can be @FREE_SCHED_DATA from #sched_flags_t.
 *
 * Function @func will be called at time @sched_time with arguments @sched_time
 * and @data.
 *
 * Returns: a pointer which is now useless from outsite but in the future
 * may allow for an event to be modified or canceled.
 */
gpointer sched_event(gint sched_time, sched_callback_t func, gpointer data,
		gint flags)
{
	sched_t *new_sched = g_new(sched_t, 1);
	new_sched->time=sched_time;
	new_sched->flags=flags;
	new_sched->delay=0;
	new_sched->func=func;
	new_sched->data=data;
	wait_queue = g_slist_insert_sorted(wait_queue, new_sched,
			(GCompareFunc)time_compare);
	return new_sched;
}
/**
 * sched_delay: schedules an event at relative time from now.
 * @delay: relative time from now.
 * @func: function to be called when the time comes.
 * @data: will be used as the second argument to @func.
 * @flags: can be @FREE_SCHED_DATA and @SCHED_RELOAD from #sched_flags_t.
 *
 * function @func will be called within @delay time units with the current
 * time as first argument and @data as the second.
 * 
 * RETURNS: a pointer which is now useless from outsite but in the future
 * may allow for an event to be modified or canceled.
 */
gpointer sched_delay(gint delay, sched_callback_t func, gpointer data,
		gint flags)
{
	sched_t *new_sched = sched_event(time+delay, func, data, flags);

	if (flags & SCHED_RELOAD)
		new_sched->delay=delay;

	return new_sched;
}
