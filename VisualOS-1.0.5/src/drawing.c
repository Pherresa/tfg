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
#include <stdio.h>
#include <gtk/gtk.h>

#include <drawing.h>
#include <messaging.h>

/* descrives a drawing with all it's styles */
typedef struct drawing_s {
	drawing_style_t **styles;
	gint n_styles;
} drawing_t;
static void switch_page (GtkWidget *notebook, GtkNotebookPage *page,
				gint new_page)
{
	drawing_t *data=gtk_object_get_user_data(GTK_OBJECT(notebook));

        if (GTK_WIDGET_REALIZED(data->styles[new_page]->widget))
		data->styles[new_page]->update(data->styles[new_page]->widget);
}
/**
 * create_drawing:
 *
 * Creates a widget capable of containing multiple representations for a 
 * certain subsystem.
 *
 * returns: a widget ready to be handled with the apropiate GTK+ functions.
 */
GtkWidget *create_drawing(void)
{
	drawing_t *data=g_new(drawing_t, 1);
	GtkWidget *notebook=gtk_notebook_new();

	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

	data->styles=NULL;
	data->n_styles=0;

	gtk_object_set_user_data(GTK_OBJECT(notebook), data);
	gtk_object_set_data(GTK_OBJECT(notebook), "widget", notebook);
	gtk_signal_connect_after (GTK_OBJECT (notebook), "switch_page",
				GTK_SIGNAL_FUNC (switch_page), NULL);
	return notebook;
}
static GtkWidget *new_container(GtkWidget *widget, drawing_flags_t flags)
{
	if (flags & DRAWING_FIXED_SIZE){
		GtkWidget *scrolled_window;

		scrolled_window = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (
				GTK_SCROLLED_WINDOW (scrolled_window),
				GTK_POLICY_AUTOMATIC,
				GTK_POLICY_AUTOMATIC);

		gtk_scrolled_window_add_with_viewport(
				GTK_SCROLLED_WINDOW(scrolled_window),
				widget);
		return scrolled_window;
	}else  {
		GtkWidget *frame;
		if (flags & DRAWING_FIXED_RATIO){
			gfloat total=widget->requisition.width
					+ widget->requisition.height;
			frame = gtk_aspect_frame_new(NULL,
					widget->requisition.width/total,
					widget->requisition.height/total,
					1, TRUE);
		} else
			frame = gtk_frame_new(NULL);

		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_container_add(GTK_CONTAINER(frame), widget);
		return frame;
	}
}
static void style_detach(GtkButton *button, drawing_style_t *style)
{
	GtkWidget *style_box = gtk_object_get_data(GTK_OBJECT(style->widget),
						   "style_box");
	GtkWidget *widget = gtk_object_get_data(GTK_OBJECT(style->widget),
						"style_container");
	GtkWidget *window = gtk_object_get_data(GTK_OBJECT(style->widget),
						"style_window");
	g_return_if_fail(style_box != NULL);
	g_return_if_fail(widget != NULL);
	g_return_if_fail(window != NULL);

	gtk_window_set_default_size(GTK_WINDOW(window),
				    widget->allocation.width,
				    widget->allocation.height);

	/* reparent the style */
	gtk_widget_ref(widget);
	gtk_container_remove(GTK_CONTAINER(style_box), widget);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_widget_unref(widget);

	gtk_widget_show_all(GTK_WIDGET(window));
	gtk_widget_hide(GTK_WIDGET(style_box));
}
static gboolean style_attach(GtkWidget *window, GdkEvent *event,
			     drawing_style_t *style)
{
	GtkWidget *style_box = gtk_object_get_data(GTK_OBJECT(style->widget),
						   "style_box");
	GtkWidget *widget = gtk_object_get_data(GTK_OBJECT(style->widget),
						"style_container");
	g_return_val_if_fail(style_box != NULL, TRUE);
	g_return_val_if_fail(widget != NULL, TRUE);

	gtk_widget_ref(GTK_WIDGET(widget));
	gtk_container_remove(GTK_CONTAINER(window), widget);
	gtk_box_pack_end(GTK_BOX(style_box), widget, TRUE, TRUE, 0);
	gtk_widget_unref(GTK_WIDGET(widget));
	gtk_widget_hide(GTK_WIDGET(window));
	gtk_widget_show_all(GTK_WIDGET(style_box));
	gtk_widget_show(GTK_WIDGET(style_box));
	return TRUE;
}
static void setup_detaching(drawing_style_t *style, GtkBox *style_box,
			    GtkWidget *container)
{
	static GtkTooltips *detach_tips = NULL;
	GtkWindow *style_window;
	GtkWidget *detach_button;
	GString *str;

	if(detach_tips == NULL)
		detach_tips = gtk_tooltips_new();
	
	gtk_object_set_data(GTK_OBJECT(style->widget), "style_container",
			    container);
	gtk_object_set_data(GTK_OBJECT(style->widget), "style_box",
			    style_box);

	style_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_object_set_data(GTK_OBJECT(style->widget), "style_window",
			    style_window);
	str = g_string_new(NULL);
	g_string_sprintf(str, "%s:%s", mesg_subsystem_name(mesg_who_am_i()),
			 _(style->name));
	gtk_window_set_title(style_window, str->str);
	g_string_free(str, TRUE);
	gtk_signal_connect(GTK_OBJECT(style_window), "delete_event",
			   GTK_SIGNAL_FUNC(style_attach),
			   style);

	detach_button = gtk_button_new();
	gtk_tooltips_set_tip(GTK_TOOLTIPS (detach_tips), detach_button,
			     _("Detach the drawing style to its own window"),
			     "");

	gtk_box_pack_start(style_box, GTK_WIDGET(detach_button),
			   FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(detach_button), "clicked",
			   GTK_SIGNAL_FUNC(style_detach),
			   style);
}
/**
 * register_drawing_style:
 * @drawing: a widget returned by @create_drawing.
 * @style: the structure descriving a drawing style.
 *
 * Adds @style to @drawing.
 */
void register_drawing_style(GtkWidget *drawing, drawing_style_t * style)
{
	drawing_t *data;
	GtkWidget *notebook;
	GtkWidget *label;
	GtkWidget *container;
	GtkBox *style_box;
	
	g_return_if_fail(drawing != NULL);
	g_return_if_fail(style != NULL);
	g_return_if_fail(style->widget != NULL);
	g_return_if_fail(style->update != NULL);
	
	label = gtk_label_new(_(style->name));
	data=gtk_object_get_user_data(GTK_OBJECT(drawing));
	notebook=gtk_object_get_data(GTK_OBJECT(drawing), "widget");

	g_return_if_fail(data != NULL);
	g_return_if_fail(notebook != NULL);

	data->styles=g_renew(drawing_style_t *, data->styles,
			     ++data->n_styles);
	data->styles[data->n_styles-1] = style;

	style_box = GTK_BOX(gtk_vbox_new(FALSE, 0));

	container = new_container(style->widget, style->flags);

	setup_detaching(style, style_box, container);
#if 0
	gtk_signal_connect (GTK_OBJECT (style->widget), "show",
			    GTK_SIGNAL_FUNC (style->update), NULL);
#endif
	gtk_box_pack_end(style_box, GTK_WIDGET(container), TRUE, TRUE, 0);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
				 GTK_WIDGET(style_box), label);


	gtk_widget_show_all(GTK_WIDGET(style_box));
}
/**
 * update_drawing:
 * @drawing: a widget returned by create_drawing.
 *
 * Updates the styles (representations) in a "drawing".
 */
void update_drawing(GtkWidget *drawing)
{
	drawing_t *data=gtk_object_get_user_data(GTK_OBJECT(drawing));
	gint i;

	g_return_if_fail(data!=NULL);
	
	for (i=0; i < data->n_styles; ++i)
		if(GTK_WIDGET_REALIZED(data->styles[i]->widget))
			data->styles[i]->update(data->styles[i]->widget);
}


