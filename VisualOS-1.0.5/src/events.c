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

#include "events.h"

struct cb_funcs{
	sys_event_callback *func;
	struct cb_funcs *next;
};
static struct list{
	sys_event_t type;
	struct cb_funcs *funcs;
	struct list *next;
} *list = NULL;
	
/**
 * system_event:
 * @type: type of the event.
 * @data: agument of the event.
 *
 * Generate a system event of type @type with argument @data.
 */
void system_event(sys_event_t type, gpointer data)
{
	struct list *item=list;
	struct cb_funcs *funcs;
	
	while (item != NULL && item->type != type)
		item = item->next;
	if (item == NULL)
		return;
	funcs = item->funcs;
	while (funcs != NULL){
		funcs->func(type, data);
		funcs=funcs->next;
	}
}
/**
 * system_event_receive:
 * @type: type of event to receive.
 * @func: function to call.
 *
 * Instruct the system events code to call @func when ever an event of
 * type @type ocurs.
 */
void system_event_receive(sys_event_t type, sys_event_callback func)
{
	struct list **item=&list;
	struct cb_funcs **funcs;
	
	g_return_if_fail(func != NULL);

	while (*item != NULL && (*item)->type != type)
		item = &((*item)->next);
	if (*item == NULL){
		*item = g_new(struct list, 1);
		(*item)->type=type;
		(*item)->funcs=NULL;
		(*item)->next=NULL;
	}
	funcs = &((*item)->funcs);
	while (*funcs != NULL)
		funcs = &((*funcs)->next);
	*funcs = g_new(struct cb_funcs, 1);
	(*funcs)->func=func;
	(*funcs)->next=NULL;
}
