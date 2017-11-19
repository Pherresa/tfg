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

#include <glib.h>
#include <glade/glade.h>
#include <gtk/gtk.h>

#include "io_config.h"

static io_config_t io_config = {
	FALSE,
	FALSE
};
const io_config_t *IO_config = &io_config;
io_config_t *get_IO_config (void)
{
	return &io_config;
}
static void enable_disable(GtkMenuItem *menuitem, gboolean *value)
{
	*value = GTK_CHECK_MENU_ITEM(menuitem)->active;
}

void init_IO_config(GladeXML *xml)
{
	GtkWidget *widget;

	widget = glade_xml_get_widget(xml, "IO_menu_stop_clock");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget),
						IO_config->stop_clock);
	gtk_signal_connect (GTK_OBJECT(widget), "activate",
				       GTK_SIGNAL_FUNC(enable_disable),
				       &io_config.stop_clock);

	widget = glade_xml_get_widget(xml, "IO_disable_subsystem");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget),
						IO_config->disabled);
	gtk_signal_connect (GTK_OBJECT(widget), "activate",
				       GTK_SIGNAL_FUNC(enable_disable),
				       &io_config.disabled);
}
