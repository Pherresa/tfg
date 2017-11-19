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
#include <gnome.h>

#include <CPU/CPU.h>
#include <bitops.h>

#include "drawings/main.h"
#include "mem_config.h"
#include "page_info.h"
#include "status.h"
#include "swap.h"

/**
 * FRAME_LOCKED:
 * @frame: frame involved.
 *
 * Test whether @frame is locked.
 */
/**
 * FRAME_REFERENCED:
 * @frame: frame involved.
 *
 * Test whether @frame has been referenced.
 */
/**
 * FRAME_MODIFIED:
 * @frame: frame involved.
 *
 * Test whether @frame has been modified.
 */
/**
 * PAGE_VALID:
 * @proc_pages: process memory information.
 * @page: page involved.
 *
 * Test whether @page is valid in @proc_pages.
 */

static frame_info_t *frames;
static gint num_frames = MAX_FRAMES;
static GSList *free_frames = NULL;

static GSList *proc_pages_list = NULL;
static GHashTable *proc_pages_hash = NULL;

static void proc_creat(gint pid)
{
	gint j;
	proc_pages_info_t *new_pages;

	if (get_proc_pages(pid, FALSE) != NULL)
		/* someone has been playing with the REQUESTOR and the
		   proc_pages_info is already there */
		return;

	new_pages = g_new(proc_pages_info_t, pid);
	new_pages->pid=pid;
	new_pages->n_pages=0;
	new_pages->bitmap=0;
	for (j=0; j<MAX_PAGES; ++j){
		new_pages->frame[j] = NULL;
		new_pages->block[j] = NO_BLOCK;
	}
	g_hash_table_insert(proc_pages_hash, GINT_TO_POINTER(pid),
			    new_pages);
	proc_pages_list = g_slist_append(proc_pages_list, new_pages);
	new_pages->node = g_slist_find(proc_pages_list, new_pages);
	if (MEM_config->disabled){
		guint32 bitmap;
		memset(&bitmap, 0xFF, sizeof(bitmap));
		mem_page_bitmap_update_server(pid, bitmap);
		return;
	}
}
static void proc_finish(gint pid)
{
	gint i;
	proc_pages_info_t *pages;
	pages = get_proc_pages(pid, FALSE);
	g_return_if_fail(pages != NULL);
	for (i=0; i<MAX_PAGES; ++i){
		if(pages->block[i] != NO_BLOCK)
			put_swap_block(pages->block[i]);
		if(test_bit(i, &pages->bitmap)){
			g_return_if_fail(pages->frame[i]->frame >= 0);
			put_free_frame(pages->frame[i]->frame);
		}
	}
	g_hash_table_remove(proc_pages_hash, GINT_TO_POINTER(pid));
	proc_pages_list = g_slist_remove(proc_pages_list, pages);
	g_free(pages);
	update_MEM_drawing();
}
/**
 * init_page_info:
 *
 * Initialize the code which keeps track of pages and frames.
 */
void init_page_info(void)
{
	gint i;

	proc_pages_hash = g_hash_table_new (NULL, NULL);

	frames = g_new(frame_info_t, num_frames);
	for (i=0; i<num_frames; ++i){
		frames[i].frame = i;
		frames[i].proc = NO_PROC;
		frames[i].page = NO_PAGE;
		frames[i].flags = 0;
		free_frames = g_slist_append (free_frames, &frames[i]);
	}

	cpu_register_proc_finish(proc_finish);
	cpu_register_proc_creat(proc_creat);
}
/**
 * virt_to_phys:
 * @pid: process involved.
 * @page: a page in @pid's virtual memory.
 *
 * returns: the frame number coresponding to @page in @pid's address space
 * or NO_FRAME if there is non assigned.
 */
gint virt_to_phys (gint pid, gint page)
{
	proc_pages_info_t *pages = get_proc_pages(pid, FALSE);
	
	g_return_val_if_fail(pages != NULL, NO_FRAME);
	
	if (pages->frame[page] == NULL)
		return NO_FRAME;
	return pages->frame[page]->frame;
}
/**
 * have_free_frame:
 *
 * returns: TRUE if we have free memory frames available.
 */
gboolean have_free_frame(void)
{
	return free_frames != NULL;
}
/**
 * get_free_frame:
 *
 * returns: a free frame if any, NULL otherwise.
 */
frame_info_t *get_free_frame (void)
{
	frame_info_t *frame;

	if (free_frames == NULL)
		return NULL;

	frame = free_frames->data;
	free_frames = g_slist_remove (free_frames, frame);
	return frame;
}
/**
 * put_free_frame:
 * @frame: frame to return.
 *
 * Give back a frame to be returned by @get_free_frame later.
 */
void put_free_frame (gint frame)
{
	proc_pages_info_t *pages = get_proc_pages(frames[frame].proc, FALSE);

	if (pages != NULL)
		pages->frame[frames[frame].page] = NULL;
	frames[frame].proc=NO_PROC;
	frames[frame].page=NO_PAGE;
	frames[frame].flags=0;

	free_frames = g_slist_append (free_frames, &frames[frame]);
}
/**
 * get_frame_info:
 * @frame: frame involved.
 *
 * returns: @frame's information data.
 */
frame_info_t *get_frame_info (gint frame)
{
	if (frame >= num_frames || frame < 0)
		return NULL;

	return &frames[frame];
}
/**
 * get_proc_pages:
 * @pid: process involved.
 * @creat: it TRUE a process information structure will be created.
 *
 * Retrives the memory related information for @pid.
 *
 * If there is no memory information for @pid and @creat is TRUE the
 * information will be created.
 * 
 * returns: memory information for @pid if applyable of NULL otherwise.
 */
proc_pages_info_t *get_proc_pages(gint pid, gboolean creat)
{
	proc_pages_info_t *pages;

	g_return_val_if_fail(pid >= 1, NULL);

	pages = g_hash_table_lookup(proc_pages_hash, GINT_TO_POINTER(pid));
	if(pages == NULL && creat){
		proc_creat(pid);
		pages = g_hash_table_lookup(proc_pages_hash,
					    GINT_TO_POINTER(pid));
	}
	return pages;
}
/**
 * get_proc_pages_list:
 *
 * returns: the first element of the memory infomation structures.
 */
proc_pages_info_t *get_proc_pages_list(void)
{
	if (proc_pages_list == NULL)
		return NULL;

	return proc_pages_list->data;
}
/**
 * mem_page_valid:
 * @pid: process involved.
 * @page: valid page.
 *
 * Makes @page of @pid's address space valid for @pid to use.
 */
void mem_page_valid(gint pid, gint page)
{
	proc_pages_info_t *pages=get_proc_pages(pid, FALSE);
	GString *str;

	g_return_if_fail (pages->frame[page]!=NULL);

	str = g_string_new(NULL);
	set_bit(page, &pages->bitmap);
	mem_page_ready_server(pid, page);

	g_string_sprintf(str, _("Process %d got page %d (frame %d)"), pid, page,
				pages->frame[page]->frame);
	MEM_status_set(str->str);
	g_string_free(str, TRUE);
	update_MEM_drawing();
}
/**
 * mem_page_invalid:
 * @pid: process involved.
 * @page: invalid page.
 *
 * Makes @page of @pid's address space invalid so @pid will incure a page
 * fault if it tryes to use it.
 */
void mem_page_invalid(gint pid, gint page)
{
	proc_pages_info_t *pages=get_proc_pages(pid, FALSE);
	GString *str = g_string_new(NULL);
	gint old_frame;

	g_return_if_fail (pages != NULL);
	g_return_if_fail (pages->frame[page] != NULL);
	
	old_frame = pages->frame[page]->frame;
	
	clear_bit(page, &pages->bitmap);
	mem_page_bitmap_update_server(pages->pid, pages->bitmap);
	g_string_sprintf(str, _("Process %d lost frame %d (page %d)"),
				pid, old_frame, page);
	MEM_status_set(str->str);
	update_MEM_drawing();
}
/**
 * mem_assign_frame:
 * @pid: process involved.
 * @page: a page in @pid's address space.
 * @frame: a free frame.
 *
 * Assings @frame to @page in @pid's address space.
 *
 * returns: 0 if all went well -1 otherwise.
 */
gint mem_assign_frame(gint pid, gint page, gint frame)
{
	proc_pages_info_t *pages;
	frame_info_t *frame_info=get_frame_info(frame);

	g_return_val_if_fail (frame_info != NULL, -1);

	frame_info->proc = pid;
	frame_info->page = page;

	pages=get_proc_pages(pid, FALSE);
	pages->frame[page] = frame_info;
	if(pages->block[page] == NO_BLOCK)
		pages->block[page] = get_swap_block();

	pages->n_pages = MAX(pages->n_pages, page +1);

	if(pages->block[page] == NO_BLOCK){
#ifdef DEBUG
		g_print("not enogh swap space, terminating proc\n");
#endif
		CPU_terminate_proc(pid);
		return -1;
	}

	update_MEM_drawing();
	return 0;
}
/**
 * proc_pages_next:
 * @pages: a process' memory information structure.
 *
 * returns: then next memory information structure or NULL if @pages is
 * the last element.
 */
proc_pages_info_t *proc_pages_next(proc_pages_info_t *pages)
{
	if (pages->node->next == NULL)
		return NULL;
	return pages->node->next->data;
}
/**
 * get_frames_list:
 *
 * returns: the first element on the list of frame information structures.
 */
frame_info_t *get_frames_list(void)
{
	return frames;
}
/**
 * mem_frames_next:
 * @frame: a frame information structure.
 *
 * returns: the next frame information structure or NULL if @frame is
 * the last element.
 */
frame_info_t *mem_frames_next(frame_info_t *frame)
{
	if (++frame >= &frames[num_frames])
		return NULL;
	return frame;
}
