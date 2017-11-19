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
#include <gnome.h>
#include <glade/glade.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <messaging.h>
#include <interface.h>
#include <MEM/MEM.h>
#include <IO/IO.h>

#include "main.h"

GnomeAppBar *appbar;
GString *str;

void block_ready(gint block);
void page_ready(gint pid, gint page);

/* IO widgets */
GtkSpinButton *block_spin = NULL;
GtkWidget *block_label = NULL;
gboolean using_random_blocks = FALSE;
/* MEM widgets */
GtkAdjustment *page_adj = NULL;
GtkSpinButton *process_spin = NULL;

static gboolean delete_event (GtkWidget *window, GdkEvent * event, 
		gpointer pointer)
{
	gtk_widget_hide(GTK_WIDGET(window));
	return TRUE;
}
GtkWidget *app;
static void misc_show(void)
{
	gtk_widget_show(GTK_WIDGET(app));
}

static void use_random_blocks (GtkWidget * toggle)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle))){
		gint block=1+(gint) ((gfloat)get_IO_max_data_block()*rand()
				     /(RAND_MAX+1.0));
		gtk_spin_button_set_value(block_spin, block);
		gtk_widget_set_sensitive (GTK_WIDGET(block_spin), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(block_label), FALSE);
		using_random_blocks = TRUE;
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET(block_spin), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(block_label), TRUE);
		using_random_blocks = FALSE;
	}
}
static void request_block(void)
{
	io_request_block(gtk_spin_button_get_value_as_int(block_spin));
	
	if(using_random_blocks){
		gint new_block=1+(gint) ((gfloat)get_IO_max_data_block()*rand()
				    /(RAND_MAX+1.0));
		gtk_spin_button_set_value(block_spin, new_block);
	}
}
static void read_memory(void)
{
	proc_t proc;
	gint pid = gtk_spin_button_get_value_as_int(process_spin);
	gint page = page_adj->value;

	proc.pid=pid;
	mem_touch_page(&proc, page, FALSE);
}
static void write_memory(void)
{
	proc_t proc;
	gint pid = gtk_spin_button_get_value_as_int(process_spin);
	gint page = page_adj->value;

	proc.pid=pid;
	mem_touch_page(&proc, page, TRUE);
}

void block_ready(gint block)
{
	g_string_sprintf(str, _("got block %d"), block);
	gnome_appbar_set_status(appbar, str->str);
}
void page_ready(gint pid, gint page)
{
	g_string_sprintf(str, _("got page %d from process %d"), page, pid);
	gnome_appbar_set_status(appbar, str->str);
}
void REQUESTOR_main(void)
{
	GladeXML *xml;
	GtkAdjustment *adj;

	srand(time(NULL));

	mesg_subsystem_setup (REQUESTOR, MESG_WITH_GTK);

	xml = glade_xml_new(get_xml_file(), "REQUESTOR");
	str = g_string_new(NULL);
	glade_xml_signal_connect(xml, "on_REQUESTOR_delete_event",
				 GTK_SIGNAL_FUNC(delete_event));
	appbar = GNOME_APPBAR(glade_xml_get_widget(xml, "REQUESTOR_appbar"));
	app = glade_xml_get_widget(xml, "REQUESTOR");

	block_spin = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "REQUESTOR_block"));
	adj = (GtkAdjustment *)gtk_adjustment_new(1, 0,
						  get_IO_max_data_block(),
						  1, 10, 0);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(block_spin), adj);
	block_label = glade_xml_get_widget(xml, "REQUESTOR_block_label");

	page_adj = GTK_ADJUSTMENT(GTK_RANGE(GTK_SCALE(
		glade_xml_get_widget(xml, "REQUESTOR_page")))
				  ->adjustment);
	process_spin = GTK_SPIN_BUTTON(
		glade_xml_get_widget(xml, "REQUESTOR_process"));

	glade_xml_signal_connect(xml, "on_REQUESTOR_IO_access",
				 request_block);
	glade_xml_signal_connect(xml, "on_REQUESTOR_MEM_write",
				 write_memory);
	glade_xml_signal_connect(xml, "on_REQUESTOR_MEM_read",
				 read_memory);
	glade_xml_signal_connect(xml, "on_REQUESTOR_IO_use_random",
				 use_random_blocks);
	
	mem_register_page_ready(page_ready);
	io_register_block_ready(block_ready);

	mesg_callback_register(MISC_SHOW, (receive_callback)misc_show);
	
	gtk_main();
	return;
}

