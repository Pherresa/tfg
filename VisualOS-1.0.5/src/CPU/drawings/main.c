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

#include <gtk/gtk.h>
#include <stdio.h>

#include <CPU/queues.h>
#include <CPU/info.h>
#include <drawing.h>

#include "main.h"
#include "shared.h"
#include "new4.h"
#include "bars.h"
#include "overlapped_bars.h"
#include "state.h"

static GtkWidget *drawing = NULL;

GtkWidget *new_CPU_drawing(void)
{
	drawing = create_drawing();

	init_CPU_drawing_shared();
	init_CPU_drawing_new4(drawing);
	init_CPU_drawing_bars(drawing);
	init_CPU_drawing_overlapped_bars(drawing);
	init_CPU_drawing_state(drawing);

	return drawing;
}

/* drawing_locked starts TRUE so that we don't try to draw anything 
 * when there is no queues, then the queues get created the drawing 
 * will get unlocked*/
static gint drawing_locked = TRUE;

gint lock_CPU_drawing(void)
{
	drawing_locked = TRUE;
	return 0;
}
gint unlock_CPU_drawing(void)
{
	drawing_locked = FALSE;
	return 0;
}

void redraw_CPU_drawing (void)
{
	/* we are called before the drawing gets created so we have to make
	 * sure it exists befor doing anything*/
	if (drawing==NULL)
		return;
	g_return_if_fail (!drawing_locked);

	update_drawing(drawing);
	return;
}

