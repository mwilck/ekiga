
/*  stats_drawing_area.c
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Julien Puydt <jpuydt@free.fr>
 */

#include "stats_drawing_area.h"

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif

/* internal details of the object */

struct _StatsDrawingArea {
  GtkDrawingArea parent;

  PangoLayout *pango_layout;
  PangoContext *pango_context;
  GdkGC *gc;
  GdkColormap *colormap;
  GdkColor colors [6];
  GdkSegment* grid;
  int numGridLines;

  int last_audio_octets_received;
  int last_video_octets_received;
  int last_audio_octets_transmitted;
  int last_video_octets_transmitted;

  float transmitted_audio_speeds[50];
  float received_audio_speeds[50];
  float transmitted_video_speeds[50];
  float received_video_speeds[50];
  int position;
};


struct _StatsDrawingAreaClass {
  GtkDrawingAreaClass parent;
};


/* helper functions' declarations */


/* the three usual functions for GObjects */
static void stats_drawing_area_finalize (GObject *self);
static void stats_drawing_area_init (StatsDrawingArea *self);


/* DESCRIPTION  : zeroes the cached data and the display
 * PRE          : a non-NULL StatsDrawingArea
 */
static void stats_drawing_area_data_reset (StatsDrawingArea *self);


/* DESCRIPTION  : gtk callback that triggers the redisplay of the
 *                drawing.
 * PRE          : the GtkWidget* should hide a StatsDrawingArea*
 */
static gboolean stats_drawing_area_exposed_cb (GtkWidget *,
					       GdkEventExpose *,
					       gpointer);
/* DESCRIPTION  :
 * PRE          :
 */
static void stats_drawing_area_size_allocate (GtkWidget *,
					      GtkAllocation *);



/* helper functions' implementation */


static void
stats_drawing_area_finalize (GObject *obj)
{
  GObjectClass *parent_class = NULL;
  StatsDrawingArea *self = STATS_DRAWING_AREA (obj);

  if (self->gc) {
    gdk_colormap_free_colors (self->colormap, self->colors, 6);
    gdk_gc_unref (self->gc);
    g_object_unref (G_OBJECT (self->pango_layout));
  }

  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (STATS_DRAWING_AREA_GET_CLASS (self)));
  if (parent_class->finalize)
    parent_class->finalize (obj);
}


static void
stats_drawing_area_init (StatsDrawingArea *self)
{
  self->colors [0].red = 0;
  self->colors [0].green = 0;
  self->colors [0].blue = 0;

  self->colors [1].red = 169;
  self->colors [1].green = 38809;
  self->colors [1].blue = 52441;

  self->colors [2].red = 0;
  self->colors [2].green = 65535;
  self->colors [2].blue = 0;
  
  self->colors [3].red = 65535;
  self->colors [3].green = 0;
  self->colors [3].blue = 0;

  self->colors [4].red = 0;
  self->colors [4].green = 0;
  self->colors [4].blue = 65535;

  self->colors [5].red = 65535;
  self->colors [5].green = 54756;
  self->colors [5].blue = 0;

  self->pango_context = gtk_widget_get_pango_context (GTK_WIDGET (self));
  self->pango_layout = pango_layout_new (self->pango_context);
  self->gc = NULL;

  self->grid = NULL;
  self->numGridLines = 0;

  g_signal_connect (G_OBJECT (self), "expose_event",
		    G_CALLBACK (stats_drawing_area_exposed_cb), 
		    NULL);

}


static void
stats_drawing_area_class_init (StatsDrawingAreaClass *klass)
{
  GObjectClass *gobject_klass = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

  gobject_klass->finalize = stats_drawing_area_finalize;
  widget_class->size_allocate = stats_drawing_area_size_allocate;
}

static void
stats_drawing_area_data_reset (StatsDrawingArea *self)
{
  int i = 0;

  g_return_if_fail (self != NULL);
  
  self->last_audio_octets_received = 0;
  self->last_audio_octets_transmitted = 0;
  self->last_video_octets_received = 0;
  self->last_video_octets_transmitted = 0;

  self->position = 0;
  for (i = 0; i < 50; i++) {
    self->transmitted_audio_speeds[i] = (float)0;
    self->received_audio_speeds[i] = (float)0;
    self->transmitted_video_speeds[i] = (float)0;
    self->received_video_speeds[i] = (float)0;
  }
}


static gboolean 
stats_drawing_area_exposed_cb (GtkWidget *widget, 
			       GdkEventExpose *event,
			       gpointer data)
{
  StatsDrawingArea *self = NULL;
  
  gchar *pango_text = NULL;
  
  int cpt = 0;
  int pos = 0;
  GdkPoint points [50];
  int width_step = 0;
  int allocation_height = 0;
  float height_step = 0;

  float max_speed = 1;
  gboolean success [6];

  g_return_val_if_fail (IS_STATS_DRAWING_AREA (widget), FALSE);

  self = STATS_DRAWING_AREA (widget);

  if (self->gc == NULL) {
    self->gc = gdk_gc_new (GDK_DRAWABLE (GTK_WIDGET (self)->window));
    
    self->colormap = gdk_drawable_get_colormap (GDK_DRAWABLE (GTK_WIDGET (self)->window));
    gdk_colormap_alloc_colors (self->colormap, self->colors,
			       6, FALSE, TRUE, success);
  }

  allocation_height = GTK_WIDGET (widget)->allocation.height;

  width_step = (int) GTK_WIDGET (widget)->allocation.width / 40;

  gdk_gc_set_foreground (self->gc, &self->colors [0]);
  gdk_draw_rectangle (widget->window,
		      self->gc,
		      TRUE, 0, 0, 
		      GTK_WIDGET (widget)->allocation.width,
		      allocation_height);

  gdk_gc_set_foreground (self->gc, &self->colors [1]);
  gdk_gc_set_line_attributes (self->gc, 1, GDK_LINE_SOLID, 
			      GDK_CAP_ROUND, GDK_JOIN_BEVEL);
  
  gdk_draw_segments (GDK_DRAWABLE (widget->window), self->gc, self->grid, self->numGridLines);
  gdk_window_set_background (widget->window, &self->colors [0]);


  /* Compute the height_step */
  for (cpt = 0 ; cpt < 50 ; cpt++) {
    
    if (self->transmitted_audio_speeds [cpt] > max_speed)
      max_speed = self->transmitted_audio_speeds [cpt];
    if (self->received_audio_speeds [cpt] > max_speed)
      max_speed = self->received_audio_speeds [cpt];
    if (self->transmitted_video_speeds [cpt] > max_speed)
      max_speed = self->transmitted_video_speeds [cpt];
    if (self->received_video_speeds [cpt] > max_speed)
      max_speed = self->received_video_speeds [cpt];    
  }
  height_step = allocation_height / max_speed;
 
  gdk_gc_set_line_attributes (self->gc, 2, GDK_LINE_SOLID, 
			      GDK_CAP_ROUND, GDK_JOIN_BEVEL);
  
  /* Transmitted audio */
  gdk_gc_set_foreground (self->gc, &self->colors [3]);
  pos = self->position;
  for (cpt = 0 ; cpt < 50 ; cpt++) {
    
    points [cpt].x = cpt * width_step;
    
    points [cpt].y = allocation_height -
      (gint) (self->transmitted_audio_speeds [pos] * height_step);
    pos++;
    
    if (pos >= 50) pos = 0;
  }
  gdk_draw_lines (GDK_DRAWABLE (widget->window), self->gc, points, 50);
  
  
  /* Received audio */
  gdk_gc_set_foreground (self->gc, &self->colors [5]);
  pos = self->position;
  for (cpt = 0 ; cpt < 50 ; cpt++) {
    
    points [cpt].x = cpt * width_step;
    
    points [cpt].y = allocation_height -
      (gint) (self->received_audio_speeds [pos] * height_step);
    pos++;
    
    if (pos >= 50) pos = 0;
  }
  gdk_draw_lines (GDK_DRAWABLE (widget->window), self->gc, points, 50);


  /* Transmitted video */
  gdk_gc_set_foreground (self->gc, &self->colors [4]);
  pos = self->position;
  for (cpt = 0 ; cpt < 50 ; cpt++) {
    
    points [cpt].x = cpt * width_step;
    
    points [cpt].y = allocation_height -
      (gint) (self->transmitted_video_speeds [pos] * height_step);
    pos++;
    
    if (pos >= 50) pos = 0;
  }
  gdk_draw_lines (GDK_DRAWABLE (widget->window), self->gc, points, 50);
  
  
  /* Received video */
  gdk_gc_set_foreground (self->gc, &self->colors [2]);
  pos = self->position;
  for (cpt = 0 ; cpt < 50 ; cpt++) {
    
    points [cpt].x = cpt * width_step;
    
    points [cpt].y = allocation_height -
      (gint) (self->received_video_speeds [pos] * height_step);
    pos++;
    
    if (pos >= 50) pos = 0;
  }
  gdk_draw_lines (GDK_DRAWABLE (widget->window), self->gc, points, 50);
  
  
  /* Text */
  
  pango_text =
    g_strdup_printf (_("Total: %.2f MB"),
		     (float) (self->last_video_octets_transmitted
			      + self->last_audio_octets_transmitted
			      + self->last_video_octets_received
			      + self->last_audio_octets_received)
		     / (1024*1024));
  pango_layout_set_text (self->pango_layout, pango_text, strlen (pango_text));
  gdk_draw_layout_with_colors (GDK_DRAWABLE (widget->window),
			       self->gc, 5, 2,
			       self->pango_layout,
			       &self->colors [5], 0);
  g_free (pango_text);

  return TRUE;
}


static void
stats_drawing_area_size_allocate (GtkWidget *widget,
				  GtkAllocation *allocation)
{
  StatsDrawingArea *self = NULL;
  int y = 0;
  int x = 0;
  int cpt = 0;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (IS_STATS_DRAWING_AREA (widget));
  g_return_if_fail (allocation != NULL);

  self = STATS_DRAWING_AREA (widget);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget)) {
    gdk_window_move_resize (widget->window,
			    allocation->x, allocation->y,
			    allocation->width, allocation->height);
  }

  /* Calculate Grid Segments */
  self->numGridLines = ((20+allocation->height) / 21) + ((20+allocation->width) / 21);
  GdkSegment* s = (GdkSegment*) malloc(self->numGridLines * sizeof(GdkSegment));
  free(self->grid);
  self->grid = s;

  while ( y < allocation->height) {

    s [cpt].x1 = 0;
    s [cpt].x2 = allocation->width;
    s [cpt].y1 = y;
    s [cpt].y2 = y;
      
    y = y + 21;
    cpt++;
  }
 
  while (x < allocation->width) {

    s [cpt].x1 = x;
    s [cpt].x2 = x;
    s [cpt].y1 = 0;
    s [cpt].y2 = allocation->height;
      
    x = x + 21;
    cpt++;
  }
}


/* implementation of the external api */


GType
stats_drawing_area_get_type ()
{
  static GType my_type = 0;

  if (my_type == 0) {
         static const GTypeInfo my_info = {
          sizeof (StatsDrawingAreaClass),
          NULL,
          NULL,
          (GClassInitFunc)stats_drawing_area_class_init,
          NULL,
          NULL,
          sizeof(StatsDrawingArea),
          0,
          (GInstanceInitFunc)stats_drawing_area_init,
         };
         my_type = g_type_register_static(GTK_TYPE_DRAWING_AREA ,
                                          "StatsDrawingArea", &my_info,
					  (GTypeFlags)0);
  }
  return my_type;
}


GtkWidget *
stats_drawing_area_new ()
{
  return GTK_WIDGET (g_object_new (STATS_DRAWING_AREA_TYPE, NULL));
}


void
stats_drawing_area_clear (GtkWidget *widget)
{
  StatsDrawingArea *self = NULL;
  int i;

  g_return_if_fail (IS_STATS_DRAWING_AREA (widget));

  self = STATS_DRAWING_AREA (widget);

  stats_drawing_area_data_reset (self);
  
  gtk_widget_queue_draw (GTK_WIDGET (self));
}


void
stats_drawing_area_new_data (GtkWidget *widget,
			     int new_video_octets_received,
			     int new_video_octets_transmitted,
			     int new_audio_octets_received,
			     int new_audio_octets_transmitted)
{
  StatsDrawingArea *self = NULL;
  float received_audio_speed = 0;
  float received_video_speed = 0;
  float transmitted_audio_speed = 0;
  float transmitted_video_speed = 0;

  g_return_if_fail (IS_STATS_DRAWING_AREA (widget));

  self = STATS_DRAWING_AREA (widget);

  /* compute speeds */
  received_audio_speed = (float) (new_audio_octets_received - self->last_audio_octets_received)/ 1024;
  transmitted_audio_speed = (float) (new_audio_octets_transmitted - self->last_audio_octets_transmitted)/ 1024;
  received_video_speed = (float) (new_video_octets_received - self->last_video_octets_received)/ 1024;
  transmitted_video_speed = (float) (new_video_octets_transmitted - self->last_video_octets_transmitted)/ 1024;

  /* fill in the speeds */
  self->transmitted_audio_speeds [self->position] = transmitted_audio_speed;
  self->received_audio_speeds [self->position] = received_audio_speed;
  self->transmitted_video_speeds [self->position] = transmitted_video_speed;
  self->received_video_speeds [self->position] = received_video_speed;

  /* get forth in the buffer, looping if needed */
  self->position++;
  if (self->position >= 50) self->position = 0;

  /* remember the current values */
  self->last_audio_octets_transmitted = new_audio_octets_transmitted;
  self->last_audio_octets_received = new_audio_octets_received;
  self->last_video_octets_transmitted = new_video_octets_transmitted;
  self->last_video_octets_received = new_video_octets_received;

  if (GTK_WIDGET_REALIZED (self))
	  gtk_widget_queue_draw (GTK_WIDGET (self));
}
