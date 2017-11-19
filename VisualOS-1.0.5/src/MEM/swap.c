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

#include <IO/geometry.h>
#include <IO/IO.h>
#include <CPU/CPU.h>
#include <bitops.h>

#include "swap.h"
#include "page_info.h"
#include "info.h"

void need_memory (gint npages);
enum swap_type {SWAPPING_IN, SWAPPING_OUT};
struct swapping {
	enum swap_type type; 
	gint pid;
	gint page;
	gint frame;
	gint block;
};
struct to_swap_in {
	gint pid;
	gint page;
	guint32 set_flags;
};

GHashTable *swapping = NULL;
GSList *to_swap_in = NULL;

GSList *free_swap = NULL;
void put_swap_block(gint block)
{
	free_swap = g_slist_prepend(free_swap, GINT_TO_POINTER(block));
}
gint get_swap_block(void)
{
	gint block;
	GSList *item = free_swap;

	while (item){
	       	block = GPOINTER_TO_INT(item->data);
		/* make sure we don't return a block which is active:
		   we may have freed a swap block at process termination
		   and left the disk reading */
		if (g_hash_table_lookup(swapping, GINT_TO_POINTER(block))
		    == NULL){
			free_swap = g_slist_remove (free_swap, item->data);
#ifdef DEBUG
			g_print("get_swap_block: block %d\n", block);
#endif
			return block;
		}
		item = item->next;
		
	}		
#ifdef DEBUG
	g_print("get_swap_block: no blocks available\n");
#endif
	return NO_BLOCK;
}
static void put_to_swap_in(gint pid, gint page, guint32 set_flags)
{
	struct to_swap_in *data = g_new(struct to_swap_in, 1);
	
	data->pid = pid;
	data->page = page;
	data->set_flags = set_flags;
	to_swap_in = g_slist_append (to_swap_in, data);
	
	return;
}
static struct to_swap_in *get_to_swap_in(void)
{
	GSList *item = to_swap_in;
	
	while(item){
		struct to_swap_in *data = item->data;
		proc_pages_info_t *pages = get_proc_pages(data->pid, FALSE);

		if(pages->frame[data->page] == NULL
		   || !FRAME_LOCKED(pages->frame[data->page])){
			to_swap_in = g_slist_remove(to_swap_in, data);
			return data;
		}
		item = item->next;
	}
	return NULL;
}
static struct to_swap_in *find_to_swap_in(gint pid, gint page)
{
	GSList *item = to_swap_in;
	while (item){
		struct to_swap_in *data = item->data;
		item = item->next;
		if (data->pid == pid && data->page == page){
			return data;
		}
	}
	return NULL;
}

static void block_ready_callback (gint block)
{
	struct swapping *data;
	frame_info_t *frame;
	proc_pages_info_t *pages;

	data = g_hash_table_lookup(swapping, GINT_TO_POINTER(block));
	g_return_if_fail(data != NULL);
	g_hash_table_remove(swapping, GINT_TO_POINTER(block));
	
	pages = get_proc_pages(data->pid, FALSE);
	frame = get_frame_info(data->frame);

	if (data->type == SWAPPING_IN){
		g_return_if_fail(pages->frame[data->page]!=NULL);
		mem_page_valid(data->pid, data->page);
		clear_bit(MEM_FRAME_LOCKED, &frame->flags);
		set_bit(MEM_FRAME_REFERENCED, &frame->flags);
	} else if (data->type == SWAPPING_OUT){
		struct to_swap_in *data;
		put_free_frame (frame->frame);
		clear_bit(MEM_FRAME_LOCKED, &frame->flags);
		while (have_free_frame() && (data = get_to_swap_in())){
			MEM_swapin_page(data->pid, data->page,
					data->set_flags);
			g_free(data);
		}
	} else
		g_assert(FALSE);

	g_free(data);
}
/**
 * MEM_swap_init:
 *
 * Gets things ready to be able to swap memory in and out.
 *
 */
void MEM_swap_init(void)
{
	gint block = get_IO_max_swap_block();

	swapping = g_hash_table_new (NULL,NULL);

	/*fill free_swap*/
	for (; block >= 0; --block)
		free_swap = g_slist_append(free_swap, GINT_TO_POINTER(block));

	io_register_block_ready(block_ready_callback);
}
/**
 * MEM_swapout_page: 
 * @pid: The Process Identification number.
 * @page: Which one of the process' pages we what back.
 *
 * Writes @page from Proces' @pid virtual memory address space from memory
 * into a free block in the swap device.
 *
 */
void MEM_swapout_page(gint pid, gint page)
{
	frame_info_t *frame = get_frame_info(virt_to_phys(pid, page));
	
	mem_page_invalid(pid, page);

	if(test_bit(MEM_FRAME_MODIFIED, &frame->flags)){
		struct swapping *data = g_new(struct swapping, 1);
		proc_pages_info_t *pages = get_proc_pages(pid, FALSE);

		data->type=SWAPPING_OUT;
		data->pid=pid;
		data->page=page;
		data->frame=frame->frame;
#if 1
		g_return_if_fail(pages->block[page] != NO_BLOCK);
#else
		if(pages->block[page] == NO_BLOCK)
			pages->block[page] = get_swap_block();
		if(pages->block[page] == NO_BLOCK){
			CPU_terminate_proc(pid);
			return;
		}
#endif
		data->block = pages->block[page];
		g_return_if_fail(g_hash_table_lookup(
			swapping, GINT_TO_POINTER(data->block)) == NULL);
		g_hash_table_insert(swapping, GINT_TO_POINTER(data->block),
				    data);
		set_bit(MEM_FRAME_LOCKED, &frame->flags);
		io_request_swap_block(data->block);
	} else {
		struct to_swap_in *data;

		put_free_frame (frame->frame);
		if ((data = get_to_swap_in())){
			MEM_swapin_page(data->pid, data->page,
					data->set_flags);
			g_free(data);
		}
	}
}

/**
 * MEM_swapin_page: get a pages content from swap.
 * @pid: The Process Identification number
 * @page: Which one of the process' pages we what back
 * @set_flags: This flags will be set on the frame when when one is assigned
 *
 * Reads @page from Proces' @pid virtual memory address space into memory from
 * the swap device
 *
 * If the page has never been swapped out we will suppose it is in the
 * first free swap block and request an IO access to it.
 * 
 * If the page is already swaping in then we will take note of 
 * @set_flags and let it be.  
 */
void MEM_swapin_page(gint pid, gint page, guint32 set_flags)
{
	proc_pages_info_t *pages = get_proc_pages(pid, FALSE);
	mem_algorithm_t *algorithm = get_MEM_current_algorithm();
	frame_info_t *frame_info;
	struct swapping *swapping_data;
	struct to_swap_in *to_swap_in_data;

	/* Make sure we are not already swapping it */
	
	if ((swapping_data = g_hash_table_lookup(
		swapping, GINT_TO_POINTER(pages->block[page])))){
		g_return_if_fail(swapping_data->pid == pid);
		g_return_if_fail(swapping_data->page == page);
		g_return_if_fail(pages->frame[page] != NULL);

		switch (swapping_data->type){
		case SWAPPING_IN:
#ifdef DEBUG
			g_print("we are swapping it in\n");
#endif
			pages->frame[page]->flags |= set_flags;
			return;
		case SWAPPING_OUT:{
			gint victim_frame = algorithm->select_frame();
#ifdef DEBUG
			g_print("we are swapping it out\n");
#endif		   
			if (victim_frame < 0){
#ifdef DEBUG
				g_print("couldn't get a victim frame"
					" terminating process %d\n", pid);
#endif
				CPU_terminate_proc(pid);
				return;
			}
			put_to_swap_in(pid, page, set_flags);
			frame_info = get_frame_info(victim_frame);
			if(frame_info->proc == NO_PROC)
				/* a frame has become available while the
				   algorithm made a choise so we don't need
				   to swap out*/
				return;
			MEM_swapout_page(frame_info->proc, frame_info->page);
			return;
		}}
	}
	/* Make sure it is not already pending to swap in */
	
	if((to_swap_in_data = find_to_swap_in(pid, page))){
#ifdef DEBUG
		g_print("it is already pending to swap in\n");
#endif
		to_swap_in_data->set_flags |= set_flags;
		return;
	}

	frame_info = get_free_frame();
	/* if we don't have free frames take note and ask for a swap
           out */
	if (frame_info == NULL){
		gint victim_frame = algorithm->select_frame();
		
		if(victim_frame < 0){
#ifdef DEBUG
			g_print("couldn't get a victim frame terminating "
				"process %d\n", pid);
#endif
			CPU_terminate_proc(pid);
			return;
		}
		frame_info = get_frame_info(victim_frame);
		if (frame_info->proc != NO_PROC){
			put_to_swap_in(pid, page, set_flags);
			MEM_swapout_page(frame_info->proc, frame_info->page);
		} else {
			/* a frame has become available while the algorithm
			   made a choise */
			MEM_swapin_page(pid, page, set_flags);
		}
		return;
	}

	/* we got a frame, arrange things to get the page back */

	if(mem_assign_frame (pid, page, frame_info->frame) < 0){
		/* assignment failed, give the frame back */
		put_free_frame(frame_info->frame);
		return;
	}

	swapping_data = g_new(struct swapping, 1);
	swapping_data->type=SWAPPING_IN;
	swapping_data->pid=pid;
	swapping_data->page=page;
	swapping_data->frame=frame_info->frame;

#if 1
	g_return_if_fail(pages->block[page] != NO_BLOCK);
#else
	if (pages->block[page] == NO_BLOCK)
		pages->block[page] = get_swap_block();
	if (pages->block[page] == NO_BLOCK){
		CPU_terminate_proc(pid);
		g_free(swapping_data);
		return;
	}
#endif
	swapping_data->block = pages->block[page];

	g_return_if_fail(g_hash_table_lookup(
		swapping, GINT_TO_POINTER(swapping_data->block)) == NULL);
	g_hash_table_insert(swapping, GINT_TO_POINTER(swapping_data->block),
			    swapping_data);

	set_bit(MEM_FRAME_LOCKED, &frame_info->flags);
	frame_info->flags |= set_flags;
	io_request_swap_block(swapping_data->block);
}
