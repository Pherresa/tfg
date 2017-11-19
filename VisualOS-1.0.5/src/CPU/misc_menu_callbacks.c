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
#include <gnome.h>

#include <messaging.h>
#include <interface.h>

#include "cpu_config.h"

#include "misc_menu_callbacks.h"

static void about(void)
{
	static GtkWidget *about_dialog = NULL;

	if (about_dialog == NULL){
		GladeXML *xml = glade_xml_new(get_xml_file(), "about-dialog");
		about_dialog = glade_xml_get_widget(xml, "about-dialog");
		gnome_dialog_close_hides(GNOME_DIALOG(about_dialog), TRUE);
	}

	gtk_widget_show(about_dialog);
}

static void stop_clock(GtkMenuItem *menuitem)
{
	cpu_config_t *config = get_CPU_config();

	config->stop_clock = GTK_CHECK_MENU_ITEM(menuitem)->active;
}
static void auto_fill_procs(GtkMenuItem *menuitem)
{
	cpu_config_t *config = get_CPU_config();

	config->auto_fill_procs = GTK_CHECK_MENU_ITEM(menuitem)->active;
}
static inline void set_CheckMenuItem(GladeXML *xml, const gchar *name,
					gboolean val)
{
	GtkWidget *widget = glade_xml_get_widget(xml, name);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), val);
}
static void show_REQUESTOR(void)
{
	mesg_send(REQUESTOR, MISC_SHOW, NULL, 0);
}
static void show_IO(void)
{
	mesg_send(IO, MISC_SHOW, NULL, 0);
}
static void show_CLOCK(void)
{
	mesg_send(CLOCK, MISC_SHOW, NULL, 0);
}
static void show_MEM(void)
{
	mesg_send(MEM, MISC_SHOW, NULL, 0);
}
static void reset_system(void)
{
	g_print("CPU: reseting\n");
	mesg_send(MESG, MESG_RESET_SYSTEM, NULL, 0);
}
void init_CPU_misc_menu_callbacks(GladeXML *xml)
{

	set_CheckMenuItem(xml, "CPU_menu_stop_clock", CPU_config->stop_clock);
	glade_xml_signal_connect (xml, "on_CPU_menu_stop_clock_activate",
					stop_clock);
	set_CheckMenuItem(xml, "CPU_menu_auto_fill_procs",
					CPU_config->auto_fill_procs);
	glade_xml_signal_connect (xml, "on_CPU_menu_auto_fill_procs_activate",
					auto_fill_procs);

	glade_xml_signal_connect (xml, "on_view_memory_activate",
					show_MEM);
	glade_xml_signal_connect (xml, "on_view_requestor_activate",
					show_REQUESTOR);
	glade_xml_signal_connect (xml, "on_view_disk_activate",
					show_IO);
	glade_xml_signal_connect (xml, "on_view_clock_activate",
					show_CLOCK);
	glade_xml_signal_connect (xml, "on_system_reset",
					reset_system);
	glade_xml_signal_connect (xml, "on_about_activate",
					about);
}
