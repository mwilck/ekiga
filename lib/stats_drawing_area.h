#ifndef __STATS_DRAWING_AREA_H_
#define __STATS_DRAWING_AREA_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define STATS_DRAWING_AREA_TYPE stats_drawing_area_get_type()


#define STATS_DRAWING_AREA(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), STATS_DRAWING_AREA_TYPE, StatsDrawingArea))


#define STATS_DRAWING_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), STATS_DRAWING_AREA_TYPE, StatsDrawingAreaClass))


#define IS_STATS_DRAWING_AREA(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), STATS_DRAWING_AREA_TYPE))


#define STATS_DRAWING_AREA_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), STATS_DRAWING_AREA_TYPE, StatsDrawingAreaClass))


typedef struct _StatsDrawingArea StatsDrawingArea;


typedef struct _StatsDrawingAreaClass StatsDrawingAreaClass;

GType stats_drawing_area_get_type ();

GtkWidget *stats_drawing_area_new ();

void stats_drawing_area_clear (GtkWidget *);

void stats_drawing_area_new_data (GtkWidget *w,
				  int vin, int vout, int ain, int aout);

G_END_DECLS

#endif
