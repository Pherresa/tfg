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

#include <MEM/info.h>
#include <drawing.h>

#include "virtual.h"
#include "physical.h"

static GtkWidget *drawing = NULL;

GtkWidget *new_MEM_drawing(void);
void update_MEM_drawing (void);


GtkWidget *new_MEM_drawing(void)
{
	drawing = create_drawing();

	init_MEM_drawing_virtual(drawing);
	init_MEM_drawing_physical(drawing);

        return drawing;
}

void update_MEM_drawing (void)
{
	/* we are called before the drawing gets created so we have to make
	 * sure it exists befor doing anything*/
	if (drawing==NULL)
		return;
	update_drawing(drawing);
	return;
}
