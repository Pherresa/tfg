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
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gnome.h>

#include <messaging.h>
#include <SCHED.h>
#include <CLOCK/CLOCK.h>

#include "mem_config.h"
#include "algorithms/algorithms.h"
#include "drawings/main.h"
#include "info.h"
#include "MEM.h"
#include "page_info.h"
#include "swap.h"
#include "status.h"

struct page_fault_s {
	gint pid;
	gint8 page;
	gint8 write;
};
typedef struct page_fault_s page_fault_t;
struct page_bitmap_update_s {
	gint pid;
	gint bitmap;
};
typedef struct page_bitmap_update_s page_bitmap_update_t;

static guint32 *page_bitmaps=NULL;
static gint n_bitmaps=0;

struct pending_clients {
	gint pid;
	gint8 page;
	subsystem_t client;
};

static GSList *pending_clients = NULL;

static page_ready_callback_t page_ready_callback = NULL;

static inline void enlarge_bitmaps(gint pid)
{
	gint i;

	if (pid < n_bitmaps)
		return;

	page_bitmaps = g_renew(guint32, page_bitmaps, pid+1);
	for (i=n_bitmaps; i < pid+1; ++i)
		page_bitmaps[i] = 0;
	n_bitmaps = pid+1;
}
static void mem_page_ready(const message_t *m)
{
	page_fault_t *fault = (page_fault_t *)m->data;
	enlarge_bitmaps(fault->pid);
	
	page_bitmaps[fault->pid] |= 1<<fault->page;
	if (page_ready_callback)
		page_ready_callback(fault->pid, fault->page);
}
static void mem_page_bitmap_update(const message_t *m)
{
	page_bitmap_update_t *update = (page_bitmap_update_t *)m->data;
	enlarge_bitmaps(update->pid);
	page_bitmaps[update->pid] = update->bitmap;
}
/**
 * mem_register_page_ready:
 * @func: function to be called.
 *
 * Registers @func to be called whenever a processes page becomes available
 * in physical memory.
 */
void mem_register_page_ready(page_ready_callback_t func)
{
	page_ready_callback = func;
	mesg_callback_register(PAGE_READY, mem_page_ready);
	mesg_callback_register(PAGE_BITMAP_UPDATE, mem_page_bitmap_update);
}
/**
 * mem_touch_page:
 * @proc: process involved.
 * @page: page we pretend to use.
 * @write: are we writing to the specified page?
 *
 * Access the page number @page of process @proc. The access will be for
 * writing if @write is TRUE.
 *
 * Returns: TRUE if the page was available and FALSE if a page fault ocurred
 * and the process has to wait.
 */
gboolean mem_touch_page(proc_t *proc, gint page, gboolean write)
{
	page_fault_t fault;

	enlarge_bitmaps(proc->pid);
	fault.pid=proc->pid;
	fault.page=page;
	fault.write=write;

	g_return_val_if_fail(page < MAX_PAGES, TRUE);
	if (page_bitmaps[proc->pid] & 1<<page){
		mesg_send(MEM, PAGE_ACCESS, &fault, sizeof(fault));
		return TRUE;
	} else {
		mesg_send(MEM, PAGE_FAULT, &fault, sizeof(fault));
		return FALSE;
	}
}
static void add_pending_client(gint pid, gint page, subsystem_t sender)
{
	struct pending_clients *client;
	client = g_new(struct pending_clients, 1);

	client->pid = pid;
	client->page = page;
	client->client = sender;
	pending_clients = g_slist_append(pending_clients, client);
}
static void page_fault(const message_t *m)
{
	page_fault_t *fault = (page_fault_t *)m->data;
	GString *str;
	guint32 set_flags=0;

	if (MEM_config->disabled){
		guint32 bitmap;
		memset(&bitmap, 0xFF, sizeof(bitmap));
		add_pending_client(fault->pid, fault->page, m->sender);
		mem_page_ready_server(fault->pid, fault->page);
		mem_page_bitmap_update_server(fault->pid, bitmap);
		return;
	}

	str = g_string_new(NULL);

	g_string_sprintf(str, _("Process %d pagefaulted on page %d "), fault->pid,
								fault->page);
	if(fault->write)
		g_string_sprintfa(str, _("(WRITE)"));
	else
		g_string_sprintfa(str, _("(READ)"));
	MEM_status_set(str->str);
	g_string_free(str, TRUE);

	set_bit(MEM_FRAME_REFERENCED, &set_flags);
	if (fault->write)
		set_bit(MEM_FRAME_MODIFIED, &set_flags);

	/* if owr client is the REQUESTOR force the creation of
	   proc_pages_info for this process as the REQUESTOR can use
	   non existent processes */
	if (m->sender == REQUESTOR)
		get_proc_pages(fault->pid, TRUE);

	add_pending_client(fault->pid, fault->page, m->sender);
	
	MEM_swapin_page(fault->pid, fault->page, set_flags);
	if (MEM_config->stop_clock)
		CLOCK_stop();
	
	return;
}
static void page_access(const message_t *m)
{
	mem_algorithm_t *algorithm = get_MEM_current_algorithm();
	page_fault_t *access = (page_fault_t *)m->data;
	proc_pages_info_t *pages;
	frame_info_t *frame;
	GString *str;

	pages = get_proc_pages(access->pid, m->sender == REQUESTOR);
	frame = pages->frame[access->page];
	if(!test_bit(access->page, &pages->bitmap))
		/* this can happen, our client didn't get the
		   PAGE_BITMAP_UPDATE message yet but he will 
		   get it soon, let it be*/
		return;

	g_return_if_fail(frame != NULL);

	if(MEM_config->disabled)
		return;

	str = g_string_new(NULL);
	g_string_sprintf(str, _("Process %d accessed page %d "), access->pid,
								access->page);
	if(access->write)
		g_string_sprintfa(str, _("(WRITE)"));
	else
		g_string_sprintfa(str, _("(READ)"));
	MEM_status_set(str->str);
	g_string_free(str, TRUE);

	set_bit(MEM_FRAME_REFERENCED, &frame->flags);
	if (access->write)
		set_bit(MEM_FRAME_MODIFIED, &frame->flags);
	algorithm->page_access(access->pid, access->page);
	return;
}
static void disable_memory(GtkWidget *check_button)
{
	proc_pages_info_t *pages = get_proc_pages_list();
	mem_config_t *config = get_MEM_config();

	if(GTK_CHECK_MENU_ITEM(check_button)->active){
		guint32 bitmap;
		/* make the CPU think that all processes have all
		   pages available */
		/* set the bitmap to ones */
		memset(&bitmap, 0xFF, sizeof(bitmap));
		while(pages != NULL){
			mem_page_bitmap_update_server(pages->pid,
						      bitmap);
			pages = proc_pages_next(pages);
		}
		config->disabled = TRUE;
	}else{
		/* tell the CPU the truth of live, memory is limited
                   :) */
		while(pages != NULL){
			mem_page_bitmap_update_server(pages->pid,
						      pages->bitmap);
			pages = proc_pages_next(pages);
		}
		config->disabled = FALSE;
	}
}
/**
 * mem_server_init:
 * @xml: Glade interface object.
 *
 * Called from the subsystem's code to initialize the interface code.
 *
 * returns: nothing important.
 */
gint mem_server_init(GladeXML *xml)
{
	mesg_callback_register(PAGE_FAULT, page_fault);
	mesg_callback_register(PAGE_ACCESS, page_access);
	glade_xml_signal_connect(xml,
				 "on_MEM_disable_subsystem_activate",
				 GTK_SIGNAL_FUNC(disable_memory));
	return 0;
}
/**
 * mem_page_ready_server:
 * @pid: process involved.
 * @page: ready page.
 *
 * Called from the subsystem's code to tell the client about a fulfilled
 * page fault.
 * 
 * returns: nothing important.
 */
gint mem_page_ready_server(gint pid, gint page)
{
	page_fault_t pagefault;
	proc_pages_info_t *pages = get_proc_pages(pid, FALSE);
	GSList *item = pending_clients;

	if (MEM_config->stop_clock)
		CLOCK_stop();
	pagefault.pid = pid;
	pagefault.page = page;

	mem_page_bitmap_update_server(pid, pages->bitmap);

	while(item != NULL){
		struct pending_clients *client=item->data;
		
		item = item->next;

		if (client->pid != pid || client->page != page)
			continue;
		pending_clients = g_slist_remove(pending_clients, client);
		mesg_send(client->client, PAGE_READY, &pagefault,
			  sizeof(pagefault));
		g_free(client);
	}
	return 0;
}
/**
 * mem_page_bitmap_update_server:
 * @pid: process involved.
 * @new_page_bitmap: valid page bitmap.
 *
 * Called from the subsystem's code to update the valid page bitmap on the
 * client.
 * 
 * returns: nothing important.
 */
gint mem_page_bitmap_update_server(gint pid, guint32 new_page_bitmap)
{
	page_bitmap_update_t update;
	
	if (MEM_config->stop_clock)
		CLOCK_stop();
	update.pid = pid;
	update.bitmap = new_page_bitmap;
	return mesg_broadcast(PAGE_BITMAP_UPDATE,
			      &update, sizeof(update));
}
