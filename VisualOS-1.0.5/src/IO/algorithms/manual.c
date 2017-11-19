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
#include <gtk/gtk.h>

#include "algorithm_api.h"
#include "manual.h"

static gint algorithm_select (void);
static gint algorithm_unselect (void);
static gint event (io_request_t *request);
static gint done_reading (void);

static io_algorithm_t algorithm = {
	"Manual",
	algorithm_select,
	algorithm_unselect,
	NULL,/*properties WidGet*/
	event,
	done_reading
};
	
gint io_manual_init (void)
{
	register_IO_algorithm (&algorithm);
	return 0;
}
static gchar *block_select_msg=N_("select blocks");
static GtkCombo *combo = NULL;
static GtkList *list = NULL;
static GtkWidget *label = NULL;
static void insert_request_gtk(GtkWidget *item, io_request_t *request)
{
	io_queue_t reading=get_IO_reading_queue();
	/* curiosusly each signal calls this function twice so I have to
         * filter out one of them */
        static gint repeat=TRUE;

        repeat = !repeat;
        if (repeat)
                return;

	gtk_entry_set_text(GTK_ENTRY(combo->entry), _(block_select_msg));

	if (item != NULL){
		GList *item_list=NULL;
		item_list = g_list_append(item_list, item);
		gtk_list_remove_items(list, item_list);
	}	
	if(gtk_container_children(GTK_CONTAINER(list)) == NULL){
		gtk_widget_hide(GTK_WIDGET(combo));
		gtk_widget_show(label);
	}
	
	/* insert the request in the reading queue so it will be read */
	reading = io_queue_append(reading, request);
	set_IO_reading_queue(reading);
}

/* Defined but not used
static void insert_request(io_request_t *request)
{
	insert_request_gtk(NULL, request);
}
*/
static void note_request(io_request_t *request)
{
	GString *str = g_string_new(NULL);
	GtkWidget *item;
	
	g_string_sprintf(str, _("%d (track %d)"),
			 request->block, request->track);
	item = gtk_list_item_new_with_label(str->str);
	gtk_widget_show(GTK_WIDGET(item));
	gtk_container_add(GTK_CONTAINER(list), item);
	gtk_signal_connect(GTK_OBJECT(item), "select",
			   GTK_SIGNAL_FUNC(insert_request_gtk),
			   request);
	gtk_widget_show(GTK_WIDGET(combo));
	gtk_widget_hide(label);
	g_string_free(str, TRUE);
}
static void copy_queue_to_combo(io_queue_t queue, GtkCombo *combo)
{
	while (!io_queue_empty(queue)){
		io_request_t *request = io_request_data(queue);
		note_request(request);
		queue = io_queue_next(queue);
	}
}

static gint algorithm_select (void)
{
	io_queue_t requested=get_IO_requested_queue();

	io_queue_erase(get_IO_reading_queue());
	set_IO_reading_queue(NULL);

	combo = GTK_COMBO(gtk_combo_new());
	list = GTK_LIST(GTK_COMBO(combo)->list);
	gtk_list_set_selection_mode(list, GTK_SELECTION_SINGLE);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry), FALSE);


	label = gtk_label_new(_("no more accesses"));
	gtk_widget_show(label);

	algorithm.properties = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(algorithm.properties),
			  GTK_WIDGET(combo));
	gtk_container_add(GTK_CONTAINER(algorithm.properties),
			  GTK_WIDGET(label));
	
	copy_queue_to_combo(requested, GTK_COMBO(combo));
	gtk_entry_set_text(GTK_ENTRY(combo->entry), _(block_select_msg));

	return 0;
}
static gint algorithm_unselect (void)
{
	gtk_widget_destroy(GTK_WIDGET(algorithm.properties));
	algorithm.properties = NULL;
	return 0;
}
static gint event (io_request_t *request)
{
	note_request(request);
	return 0;
}
static gint done_reading (void)
{
	return 0;
}
