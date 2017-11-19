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

#ifndef EVENTS_H
#define EVENTS_H
#include <glib.h>
#include <process.h>
typedef enum {
	SYS_EVENT_PROC_CREATE=0,	/* there is a new process */
	SYS_EVENT_PROC_DESTROY,		/* a process terminated */
	SYS_EVENT_PROC_READY,		/* a process is now ready to run */
	SYS_EVENT_PROC_QUEUED,		/* a process put in a ready queue */
	SYS_EVENT_PROC_RUNNING,		/* a process is now running */
	SYS_EVENT_PROC_WAITING,		/* a process is now blocked */
	SYS_EVENT_PROC_SELECT,		/* a process has been selected by
					   the user */
	SYS_EVENT_FRAME_SELECT,		/* the user selected a memory frame */
	SYS_EVENT_QUITTING		/* the subsystem is quitting */
}sys_event_t;

/**
 * sys_event_callback:
 * @type: type of event.
 * @data: Argument of the event. In most cases it is a process
 * pointer, but not always.
 *
 * Function pointer type to use with @system_event_receive.
 */
typedef void *(sys_event_callback)(sys_event_t type, gpointer data);

void system_event(sys_event_t type, gpointer data);
void system_event_receive(sys_event_t type, sys_event_callback func);
#endif
