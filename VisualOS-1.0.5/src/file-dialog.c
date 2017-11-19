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

#include <string.h>
#include <gnome.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "file-dialog.h"

static GtkWidget *file_sel = NULL;
void (*file_selected)(const gchar *file);
static gboolean selection_ok = FALSE;

static gint delete_event (GtkWidget *widget)
{
	gtk_widget_hide(GTK_WIDGET(widget));
	return TRUE;
}
static void file_selected_cancel(GtkWidget *button, GtkWidget *file_sel)
{
	gtk_widget_hide(GTK_WIDGET(file_sel));
}
static void file_selected_callback(GtkWidget *button, GtkWidget *file_sel)
{
	selection_ok = TRUE;
	gtk_widget_hide(GTK_WIDGET(file_sel));
}
static void create_CPU_prop_file(void)
{
	file_sel=gtk_file_selection_new(_("Select a file"));
	gtk_window_set_modal(GTK_WINDOW(file_sel), TRUE);
	gtk_signal_connect (GTK_OBJECT (file_sel), "delete_event",
			    GTK_SIGNAL_FUNC (delete_event), NULL);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(
				file_sel)->ok_button), "clicked",
				GTK_SIGNAL_FUNC (file_selected_callback),
				file_sel);
	gtk_signal_connect (GTK_OBJECT(file_sel), "show",
				GTK_SIGNAL_FUNC (gtk_main),
				file_sel);
	gtk_signal_connect (GTK_OBJECT(file_sel), "hide",
				GTK_SIGNAL_FUNC (gtk_main_quit),
				file_sel);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(file_sel)
				->cancel_button), "clicked",
				GTK_SIGNAL_FUNC (file_selected_cancel),
				file_sel);
}
const gchar * select_file(void)
{
	if (!file_sel)
		create_CPU_prop_file();
	selection_ok = FALSE;
	gtk_widget_show(GTK_WIDGET(file_sel));
	if (!selection_ok)
		return NULL;
	return gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_sel));
}

