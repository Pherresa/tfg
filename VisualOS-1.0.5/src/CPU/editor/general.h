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

#ifndef CPU_SIMULATION_GENERAL_EDITOR_H
#define CPU_SIMULATION_GENERAL_EDITOR_H
#include <process.h>

GtkWidget *create_CPU_prop_general_editor(GladeXML *xml);
void write_CPU_prop_general(simul_data_t *data);
void read_CPU_prop_general(proc_t *proc);
void autofill_CPU_prop_general(proc_t *proc);
#endif
