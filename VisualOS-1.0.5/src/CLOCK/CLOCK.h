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

#ifndef CLOCK_H
#define CLOCK_H
#include <glib.h>

#include <messaging.h>

enum {
	TICK=CLOCK<<8,	/* Common time reference broadcasting*/
	CLOCK_STOP	/* when we get this we stop the clock*/
};
/**
 * tick_callback_t:
 * @time: current time in "time units".
 *
 * function pointer to be used with @clock_reguster_tick.
 *
 * returns: nothing important.
 */
typedef gint (*tick_callback_t)(gint time);
gint clock_register_tick(tick_callback_t func); 
gint get_time(void);
void CLOCK_stop(void);
void CLOCK_server_init(void);
#endif
