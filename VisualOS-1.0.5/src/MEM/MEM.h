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

#ifndef MEM_H
#define MEM_H
#include <glib.h>
#include <glade/glade.h>

#include <messaging.h>
#include <process.h>

enum {PAGE_FAULT=MEM<<8, PAGE_READY, PAGE_ACCESS, PAGE_BITMAP_UPDATE};

#define MAX_FRAMES 40	/* total number of frames available */
#define MAX_PAGES 32	/* maximun number of pages per process */
/**
 * page_ready_callback_t:
 * @pid: the process involved.
 * @page: the process' page which has become ready.
 *
 * Function pointer type for the callback used on @mem_register_page_ready.
 */

typedef void (*page_ready_callback_t)(gint pid, gint page);
void mem_register_page_ready(page_ready_callback_t func); 
gboolean mem_touch_page(proc_t *proc, gint page, gboolean write);
gint mem_server_init(GladeXML *xml);
gint mem_page_ready_server(gint pid, gint page);
gint mem_page_bitmap_update_server(gint pid, guint32 new_page_bitmap);
#endif

