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

#ifndef MEM_PAGE_INFO_H
#define MEM_PAGE_INFO_H
#include <MEM/MEM.h>
#include <bitops.h>

#define NO_FRAME -1	/* frame number when there is no frame */
#define NO_PAGE -1	/* page number when there is no page */
#define NO_PROC -1	/* process number when there is no process */
#define NO_BLOCK -1	/* block number when there is no block */

typedef enum {			/* bit indexes for frame flags */
	MEM_FRAME_LOCKED=0,	/* frame is locked and should not be
				   stolen of assigned */
	MEM_FRAME_REFERENCED,	/* frame has been referenced recently */
	MEM_FRAME_MODIFIED	/* frame is modified and should be writen to
				   swap if stolen */
} mem_frame_flags_t;

typedef struct {		/* what there is to know about a frame */
	gint frame;		/* frame number */
	gint proc;		/* process it belongs to or NO_PROC */
	gint page;		/* page it belongs to of NO_PAGE */
	guint32 flags;		/* See mem_frame_flags_t */
	guint32 private_flags;	/* algorithm dependet flags */
}frame_info_t;

#define FRAME_LOCKED(frame) (test_bit(MEM_FRAME_LOCKED, &frame->flags))
#define FRAME_REFERENCED(frame) (test_bit(MEM_FRAME_REFERENCED, &frame->flags))
#define FRAME_MODIFIED(frame) (test_bit(MEM_FRAME_MODIFIED, &frame->flags))

typedef struct {			/* memory related information for a
					   process */
	gint pid;			/* process id of the process */
	gint n_pages;			/* number of pages its using */
	guint32 bitmap;			/* bitmap of valid pages */
	frame_info_t *frame[MAX_PAGES];	/* frames where the pages are stored*/
	gint block[MAX_PAGES];		/* swap blocks assigned to pages */
	GSList *node;			/* GSList link this struct is hanging
					   from */
} proc_pages_info_t;

#define PAGE_VALID(proc_pages, page) (proc_pages->bitmap & (1<<page))

void init_page_info(void);
gint virt_to_phys (gint pid, gint page);
frame_info_t *get_free_frame (void);
void put_free_frame (gint frame);
gboolean have_free_frame (void);
frame_info_t *get_frame_info (gint frame);
proc_pages_info_t *get_proc_pages(gint pid, gboolean creat);
proc_pages_info_t *get_proc_pages_list(void);
proc_pages_info_t *proc_pages_next(proc_pages_info_t *pages);
gint mem_assign_frame(gint pid, gint page, gint frame);
void mem_page_valid(gint pid, gint page);
void mem_page_invalid(gint pid, gint page);
frame_info_t *get_frames_list(void);
frame_info_t *mem_frames_next(frame_info_t *frame);
#endif







