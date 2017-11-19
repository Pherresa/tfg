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

#include "gdk-helper.h"
/**
 * new_gdk_GC_with_color:
 * @red: red component of the color.
 * @green: green component of the color.
 * @blue: blue component of the color.
 *
 * Creates a new GdkGC (Graphic Context) with the foreground color set to
 * the color descrived by @red, @green and @blue components.
 *
 * returns: the new Graphic Context.
 */
GdkGC *new_gdk_GC_with_color(guint8 red, guint8 green, guint8 blue)
{
	static GdkPixmap *window=NULL;
	GdkColor *color;
	GdkGC *gc;

	if (!window)
		window=gdk_pixmap_new(NULL, 1, 1,
				gdk_visual_get_system()->depth);
	/* first, create a GC to draw on */
	gc = gdk_gc_new(window);

	/* the color we want to use */
	color = (GdkColor *) g_new(GdkColor, 1);

	/* red, green, and blue are passed values, indicating the RGB triple
	 * of the color we want to draw. Note that the values of the RGB components
	 * within the GdkColor are taken from 0 to 65535, not 0 to 255.
	 */
	color->red = red * (65535 / 255);
	color->green = green * (65535 / 255);
	color->blue = blue * (65535 / 255);

	/* the pixel value indicates the index in the colormap of the color.
	 * it is simply a combination of the RGB values we set earlier
	 */
	color->pixel = (gulong) (red * 65536 + green * 256 + blue);

	/* However, the pixel valule is only truly valid on 24-bit (TrueColor)
	 * displays. Therefore, this call is required so that GDK and X can
	 * give us the closest color available in the colormap
	 */
	gdk_color_alloc(gdk_colormap_get_system(), color);

	/* set the foreground to our color */
	gdk_gc_set_foreground(gc, color);

	return gc;
}
static void resize_pixmap_true(GdkPixmap **pixmap, gint width, gint height, GdkGC *fill)
{
	GdkPixmap *new_pixmap;
	gint old_width, old_height;

	g_return_if_fail(pixmap != NULL);
	g_return_if_fail(fill != NULL);

	gdk_window_get_size(*pixmap, &old_width, &old_height);

	if (width <= 0) width = 1;
	if (height <= 0) height = 1;
	/* get new pixmap */
	new_pixmap = gdk_pixmap_new(NULL, width, height,
				gdk_visual_get_system()->depth);
	/*clear the new pixmap*/
	gdk_draw_rectangle(new_pixmap, fill, TRUE,
			   0, 0,
			   width,
			   height);
	/* copy old image */
	gdk_window_copy_area (new_pixmap,
			fill,
			0, 0, *pixmap,
			0, 0, old_width, old_height);
	gdk_pixmap_unref(*pixmap);
	*pixmap=new_pixmap;
}
/**
 * resize_gdk_pixmap:
 * @pixmap: pixmap to resize.
 * @new_width: the new width of the pixmap.
 * @new_height: the new height of the pixmap.
 * @fill: a GdkGC with the foreground color set.
 *
 * Will resize *@pixmap and, if enlarging, will fill the extra area with the
 * foreground color of @fill.
 *
 * If the size realy changes, it will copy the old pixmap to a new one,
 * destroing it and setting *@pixmap to the new pixmap.
 */
void resize_gdk_pixmap(GdkPixmap **pixmap, gint new_width, gint new_height, GdkGC *fill)
{
	gint old_width, old_height;

	gdk_window_get_size(*pixmap, &old_width, &old_height);

	if (new_width == old_width && new_height == old_height)
		return;
	
	resize_pixmap_true(pixmap, new_width, new_height, fill);
}
/**
 * enlarge_gdk_pixmap:
 * @pixmap: pixmap to resize.
 * @new_width: the new width of the pixmap.
 * @new_height: the new height of the pixmap.
 * @fill: a GdkGC with the foreground color set.
 *
 * Same as @resize_gdk_pixmap but will only do the resizing if the
 * requested size is bigger then the current size.
 *
 * This is usefull for eficiency. See
 * src/MEM/drawings/virtual.c:draw_page_tables for an example.
 */
void enlarge_gdk_pixmap(GdkPixmap **pixmap, gint new_width, gint new_height, GdkGC *fill)
{
	gint old_width, old_height;
	gint width, height;

	gdk_window_get_size(*pixmap, &old_width, &old_height);

	if (new_width <= old_width && new_height <= old_height)
		return;
	
	width = new_width <= old_width? old_width : new_width;
	height = new_height <= old_height? old_height : new_height;

	resize_pixmap_true(pixmap, width, height, fill);
}
/**
 * fill_gdk_window:
 * @pixmap: pixmap to fill.
 * @fill: a GdkGC with foreground color set.
 *
 * Paint all the surface of @pixmap with the foreground color of @fill.
 */
void fill_gdk_window(GdkPixmap *pixmap, GdkGC *fill)
{
	gint width, height;

	gdk_window_get_size(pixmap, &width, &height);

	gdk_draw_rectangle(pixmap, fill, TRUE, 0, 0, width, height);

}
/**
 * draw_gdk_text_centered:
 * @drawable: a GdkDrawable.
 * @font: any font.
 * @gc: a GdkGC with the foreground color set.
 * @x: x coordinate of the area.
 * @y: y coordinate of the area.
 * @width: width of the area.
 * @height:height of the area.
 * @text: text to draw.
 * @text_length: length of @text.
 *
 * Draw @text of length @text_length centered in the area defined by @x,
 * @y, @width and @height of @drawable using font @font and the foreground
 * color of @gc.
 */
void draw_gdk_text_centered (GdkDrawable *drawable, GdkFont *font, GdkGC *gc,
				gint x, gint y, gint width, gint height,
				const gchar *text, gint text_length)
{
	gint text_width = gdk_text_width(font, text, text_length);
	gint text_height = gdk_text_height(font, text, text_length);
	
	gdk_draw_text (drawable, font, gc,
			x+width/2-text_width/2, y+height/2+text_height/2,
			text, text_length);
}
