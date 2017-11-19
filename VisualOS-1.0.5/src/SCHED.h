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

#ifndef SCHED_H
#define SCHED_H
/** sched_callback_t:
 * @time: current time.
 * @data: data pointer specified when requesting the event.
 *
 * Funtion pointer type for the callbacks used in the sched facility.
 *
 */

typedef void (*sched_callback_t) (gint time, gpointer data);

typedef enum {
	FREE_SCHED_DATA	= 1<<0, /* The data pointer will be used
				 * as argument to g_free() when done*/

	SCHED_RELOAD	= 1<<1  /* The callback will be called every "delay"
				 * time units */
} sched_flags_t;

gint sched_init(void);
gpointer sched_event(gint sched_time, sched_callback_t func, gpointer data,
		gint flags);
gpointer sched_delay(gint delay, sched_callback_t func, gpointer data,
		gint flags);
#endif
