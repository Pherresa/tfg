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
#include <glade/glade.h>

#include "mem_config.h"

#include "misc_menu_callbacks.h"

static void stop_clock(GtkMenuItem *menuitem)
{
	mem_config_t *config = get_MEM_config();

	config->stop_clock = GTK_CHECK_MENU_ITEM(menuitem)->active;
}

void init_MEM_misc_menu_callbacks(GladeXML *xml)
{
	GtkWidget *widget;

	widget = glade_xml_get_widget(xml, "MEM_menu_stop_clock");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget),
						MEM_config->stop_clock);
	glade_xml_signal_connect (xml, "on_MEM_menu_stop_clock_activate",
			stop_clock);
}
