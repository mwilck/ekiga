#include "stats_drawing_area.h"

/* internal details of the object */

struct _StatsDrawingArea {
  GtkDrawingArea parent;

  PangoLayout *pango_layout;
  PangoContext *pango_context;
  GdkGC *gc;
  GdkColormap *colormap;
  GdkColor colors [6];

  int last_audio_octets_received;
  int last_video_octets_received;
  int last_audio_octets_transmitted;
  int last_video_octets_transmitted;

  float transmitted_audio_speeds[100];
  float received_audio_speeds[100];
  float transmitted_video_speeds[100];
  float received_video_speeds[100];
  int position;
};


struct _StatsDrawingAreaClass {
  GtkDrawingAreaClass parent;
};


/* helper functions' declarations */

static void stats_drawing_area_dispose (GObject *self);
static void stats_drawing_area_finalize (GObject *self);
static void stats_drawing_area_init (StatsDrawingArea *self);
static void stats_drawing_area_data_reset (StatsDrawingArea *self);
static gboolean stats_drawing_area_exposed_cb (GtkWidget *,
					       GdkEventExpose *,
					       gpointer);

/* helper functions' definitions */


static void
stats_drawing_area_dispose (GObject *self)
{
  GObjectClass *parent_class = NULL;

  g_print ("Test object dispose\n");

  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (STATS_DRAWING_AREA_GET_CLASS (self)));
  parent_class->dispose (G_OBJECT (self));
}


static void
stats_drawing_area_finalize (GObject *self)
{
  GObjectClass *parent_class = NULL;

  g_print ("Test object dispose\n");

  parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (STATS_DRAWING_AREA_GET_CLASS (self)));
  parent_class->finalize (G_OBJECT (self));
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

  g_signal_connect (G_OBJECT (self), "expose_event",
		    G_CALLBACK (stats_drawing_area_exposed_cb), 
		    NULL);

}


static void
stats_drawing_area_class_init (StatsDrawingAreaClass *klass)
{
  GObjectClass *object_klass = NULL;

  object_klass = G_OBJECT_CLASS (klass);
  object_klass->dispose = stats_drawing_area_dispose;
  object_klass->finalize = stats_drawing_area_finalize;
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
  for (i = 0; i < 100; i++) {
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
  
  GdkSegment s [50];

  gchar *pango_text = NULL;
  
  int x = 0;
  int y = 0;
  int cpt = 0;
  int pos = 0;
  GdkPoint points [50];
  int width_step = 0;
  int allocation_height = 0;
  float height_step = 0;

  float max_transmitted_video = 1;
  float max_transmitted_audio = 1;
  float max_received_video = 1;
  float max_received_audio = 1;
  gboolean success [256];

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
  height_step = allocation_height;

  x = width_step;
  
  gdk_gc_set_foreground (self->gc, &self->colors [0]);
  gdk_draw_rectangle (widget->window,
		      self->gc,
		      TRUE, 0, 0, 
		      GTK_WIDGET (widget)->allocation.width,
		      allocation_height);

  gdk_gc_set_foreground (self->gc, &self->colors [1]);
  gdk_gc_set_line_attributes (self->gc, 1, GDK_LINE_SOLID, 
			      GDK_CAP_ROUND, GDK_JOIN_BEVEL);
  
  while ( y < allocation_height
	  && cpt < 50) {

    s [cpt].x1 = 0;
    s [cpt].x2 = widget->allocation.width;
    s [cpt].y1 = y;
    s [cpt].y2 = y;
      
    y = y + 21;
    cpt++;
  }
 
  gdk_draw_segments (GDK_DRAWABLE (widget->window), self->gc, s, cpt);

  cpt = 0;
  while (x < widget->allocation.width && cpt < 50) {

    s [cpt].x1 = x;
    s [cpt].x2 = x;
    s [cpt].y1 = 0;
    s [cpt].y2 = widget->allocation.height;
      
    x = x + 21;
    cpt++;
  }
 
  gdk_draw_segments (GDK_DRAWABLE (widget->window), self->gc, s, cpt);
  gdk_window_set_background (widget->window, &self->colors [0]);


  /* Compute the height_step */
  for (cpt = 0 ; cpt < 50 ; cpt++) {
    
    if (self->transmitted_audio_speeds [cpt] > max_transmitted_audio)
      max_transmitted_audio = self->transmitted_audio_speeds [cpt];
    if (self->received_audio_speeds [cpt] > max_received_audio)
      max_received_audio = self->received_audio_speeds [cpt];
    if (self->transmitted_video_speeds [cpt] > max_transmitted_video)
      max_transmitted_video = self->transmitted_video_speeds [cpt];
    if (self->received_video_speeds [cpt] > max_received_video)
      max_received_video = self->received_video_speeds [cpt];    
  }
  if (max_received_video > allocation_height / height_step)
    height_step = allocation_height / max_received_video;
  if (max_received_audio > allocation_height / height_step)
    height_step = allocation_height / max_received_audio;
  if (max_transmitted_video > allocation_height / height_step)
    height_step = allocation_height /  max_transmitted_video;
  if (max_transmitted_audio > allocation_height / height_step)
    height_step = allocation_height / max_transmitted_audio;
  
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
    g_strdup_printf ("Total: %.2f MB", /* FIXME: gettextize */
		     (float) (self->last_video_octets_transmitted
			      + self->last_audio_octets_transmitted
			      + self->last_video_octets_received
			      + self->last_audio_octets_received)
		     / (1024*1024));
  pango_layout_set_text (self->pango_layout, pango_text, strlen (pango_text));
  gdk_draw_layout_with_colors (GDK_DRAWABLE (widget->window),
			       self->gc, 5, 2,
			       self->pango_layout,
			       &self->colors [5], &self->colors [0]);
  g_free (pango_text);

  return TRUE;
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

  /* sets things right at startup */
  if (self->last_audio_octets_transmitted == 0 && self->position == 0)
    self->last_audio_octets_transmitted = new_audio_octets_transmitted;
  if (self->last_audio_octets_received == 0 && self->position == 0)
    self->last_audio_octets_received = new_audio_octets_received;
  if (self->last_video_octets_transmitted == 0 && self->position == 0)
    self->last_video_octets_transmitted = new_video_octets_transmitted;
  if (self->last_video_octets_received == 0 && self->position == 0)
    self->last_video_octets_received = new_video_octets_received;

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

  gtk_widget_queue_draw (GTK_WIDGET (self));
}
