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
#include <gdk-helper.h>


#include "shared.h"

GdkGC *cpu_running_gc=NULL;
GdkGC *cpu_ready_gc=NULL;
GdkGC *cpu_ready_susp_gc=NULL;
GdkGC *cpu_waiting_gc=NULL;
GdkGC *cpu_waiting_susp_gc=NULL;

void init_CPU_drawing_shared(void)
{
	cpu_running_gc = new_gdk_GC_with_color(0, 250, 0);
	
	cpu_ready_gc = new_gdk_GC_with_color(0, 150, 0);
	cpu_ready_susp_gc = new_gdk_GC_with_color(108, 105, 41);

	cpu_waiting_gc = new_gdk_GC_with_color(200, 0, 0);
	cpu_waiting_susp_gc = new_gdk_GC_with_color(151, 69, 10);
}
