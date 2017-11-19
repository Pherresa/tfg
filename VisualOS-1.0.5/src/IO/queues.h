/* VisualOS is an educational visual simulator of an operating system.   
   Copyright (C) 2000,200433anuel Estrada Sainz <ranty@debian.org>
   
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

#ifndef IO_QUEUE_H
#define IO_QUEUE_H
#include <glib.h>

/* encapsulation for IO lists */
/* #define IO_QUEN_ENCAP_CHECK */
#ifdef IO_QUEN_ENCAP_CHECK
/* hack to make the compiler cry when not using the encapsulation */
typedef int io_queue_t;
#define DECLARE_IO_QUEUE(queue) io_queue_t queue=0
#define io_queue_empty(queue) (queue == 0)
#define io_request_data(element) NULL
#define io_queue_next(element) ((io_queue_t)0)
#define io_queue_len(queue) (5)
#define io_queue_init(queue) (queue=0)
#define io_queue_foreach(queue, func, data) 0
#define io_queue_concat(dest, orig1, orig2) 0
#define io_queue_remove(queue, request) 0
#define io_queue_append(queue, request) 0
#define io_queue_insert(queue, request, i) 0
#define io_queue_erase(queue) 0
#define io_queue_dup(queue) 0
#else
/* true encapsulation */
typedef GSList * io_queue_t;
#define DECLARE_IO_QUEUE(queue) io_queue_t queue=NULL
#define io_queue_empty(queue) (queue == NULL)
#define io_request_data(element) ((io_request_t *)(element->data))
#define io_queue_next(element) (element->next)
#define io_queue_len(queue) g_slist_length(queue)
#define io_queue_init(queue) (queue=NULL)
#define io_queue_foreach(queue, func, data) g_slist_foreach(queue, \
							     (GFunc)func, data)
#define io_queue_concat(dest, orig1, orig2) \
(dest = g_slist_concat(orig1, orig2))
#define io_queue_remove(queue, request) (queue = g_slist_remove (queue, request))
#define io_queue_append(queue, request) (queue = g_slist_append (queue, request))
#define io_queue_insert(queue, request, pos) (queue = g_slist_insert(queue, request, pos))
#define io_queue_erase(queue) (g_slist_free(queue))
#define io_queue_dup(queue) (g_slist_dup(queue))
#endif

#define io_queue_end(element) io_queue_empty(element)

#endif
