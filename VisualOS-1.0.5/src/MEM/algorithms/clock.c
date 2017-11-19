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
#include <gtk/gtk.h>

#include <SCHED.h>

#include "algorithm_api.h"
#include "second_chance.h"

void mem_clock_init (void);
static void algorithm_select (void);
static void algorithm_unselect (void);
static void page_access (gint pid, gint page);
static gint select_frame (void);

static mem_algorithm_t algorithm = {
	"Clock",
	algorithm_select,
	algorithm_unselect,
	NULL,/*properties WidGet*/
	page_access,
	select_frame
};
	
void mem_clock_init (void)
{
	register_MEM_algorithm (&algorithm);
	return ;
}
static void algorithm_select (void)
{
	return ;
}
static void algorithm_unselect (void)
{
	return ;
}
static void page_access(gint pid, gint page)
{
}
static frame_info_t *next_frame(frame_info_t *frame)
{
	frame = mem_frames_next(frame);
	if (!frame)
		frame = get_frames_list();
	return frame;
}
static gint select_frame (void)
{
	static frame_info_t *frame = NULL;
	frame_info_t *last_frame = frame;
	gint round;

	if (frame==NULL)
		/* we have to start somewhere */
		frame = last_frame = get_frames_list();

	for(round=0; round < 2; round++){
		/* first try with not used not modified frames */
		do{
			frame = next_frame(frame);
			if(!FRAME_REFERENCED(frame) && !FRAME_MODIFIED(frame)
			   && !FRAME_LOCKED(frame))
				return frame->frame; /* found a good frame */
		} while (frame != last_frame);

		/* if that didn't work try not used modified frames
		   and clear reference bit */
		do {
			frame = next_frame(frame);
			if(!FRAME_REFERENCED(frame) && !FRAME_LOCKED(frame))
				return frame->frame; /* found a good frame */

			clear_bit(MEM_FRAME_REFERENCED, &frame->flags);
		} while (frame != last_frame);
	}
	return NO_FRAME;
}
