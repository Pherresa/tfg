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

#define _ISOC99_SOURCE
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <gnome.h>
#include <glade/glade.h>

#include "status.h"

static GnomeAppBar *appbar;

void init_CPU_status(GladeXML * xml)
{
	char welcome[1024];
	appbar = GNOME_APPBAR(glade_xml_get_widget(xml, "CPU_appbar"));	
	snprintf(welcome, 1024, _("Welcome to %s version %s"),
		 PACKAGE, VERSION);
	gnome_appbar_set_default(appbar, welcome);
}

void CPU_status_push(gchar * msg)
{
	gnome_appbar_push(appbar, msg);
}

void CPU_status_set(gchar * msg)
{
	gnome_appbar_set_status(appbar, msg);
}
void CPU_status_pop(void)
{
	gnome_appbar_pop(appbar);
}
