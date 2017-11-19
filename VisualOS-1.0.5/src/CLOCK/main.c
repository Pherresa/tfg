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
#include <stdio.h>

#include <messaging.h>
#include <interface.h>

#include "main.h"
#include "CLOCK.h"

GnomeAppBar *appbar;
GnomeApp *app;
GString *str;
	
static gint clock_time=0;

void clock_server_start(void);

static gboolean delete_event (GtkWidget * window, GdkEvent * event, 
		gpointer pointer)
{
	gtk_widget_hide(GTK_WIDGET(app));
	return TRUE;
}
static void misc_show(void)
{
	gtk_widget_show(GTK_WIDGET(app));
}
static gint clock_tick (void)
{
	++clock_time;
	g_string_sprintf(str, _("current time: %d"), clock_time);
	gnome_appbar_set_status(appbar, str->str);
	mesg_broadcast (TICK, &clock_time, sizeof(clock_time));
	/* we return true because this function is directly used by
	 * gtk_timeout_add and otherwise the timeout would be removed */
	return TRUE;
}
static gint time_unit=100;
static gint timeout=-1;

static void time_unit_changed (GtkWidget *spin)
{
	time_unit=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

	if (timeout >= 0){
		gtk_timeout_remove(timeout);
		timeout = gtk_timeout_add(time_unit, (GtkFunction)clock_tick,
				NULL);
	}
}
/*
 * Create and show a Pop-up menu
 */
static void do_popup_menu (GtkWidget *window, GdkEventButton *gpointer pointer)
{
  GtkWidget *menu;
  int button, event_time;

  menu = gtk_menu_new ();
  g_signal_connect (menu, "deactivate", 
                    G_CALLBACK (gtk_widget_destroy), NULL);

  /* ... add menu items ... */
  /* TODO */

  if (event)
    {
      button = event->button;
      event_time = event->time;
    }
  else
    {
      button = 0;
      event_time = gtk_get_current_event_time ();
    }

  gtk_menu_attach_to_widget (GTK_MENU (menu), my_widget, NULL);
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 
                  button, event_time);
}

void clock_server_stop(void)
{
	if (timeout < 0)
		return;
	gtk_timeout_remove(timeout);
	timeout=-1;
}
void clock_server_start(void)
{
	if (timeout >= 0)
		return;
	timeout = gtk_timeout_add( time_unit, (GtkFunction)clock_tick, NULL);
}
void CLOCK_main(void)
{
	GladeXML *xml;

	mesg_subsystem_setup (CLOCK, MESG_WITH_GTK);

	xml = glade_xml_new(get_xml_file(), "CLOCK");
	str = g_string_new(NULL);
	glade_xml_signal_connect(xml, "CLOCK_start",
				 GTK_SIGNAL_FUNC(clock_server_start));
	glade_xml_signal_connect(xml, "CLOCK_stop",
				 GTK_SIGNAL_FUNC(clock_server_stop));
	glade_xml_signal_connect(xml, "CLOCK_tick",
				 GTK_SIGNAL_FUNC(clock_tick));
	glade_xml_signal_connect(xml, "on_CLOCK_timeunit_spin_changed",
				 GTK_SIGNAL_FUNC(time_unit_changed));
	glade_xml_signal_connect(xml, "on_CLOCK_delete_event",
				 GTK_SIGNAL_FUNC(delete_event));
	appbar = GNOME_APPBAR(glade_xml_get_widget(xml, "CLOCK_appbar"));
	app = GNOME_APP(glade_xml_get_widget(xml, "CLOCK"));

	mesg_callback_register(MISC_SHOW, (receive_callback)misc_show);
	CLOCK_server_init();
	gtk_main();
	return;
}

