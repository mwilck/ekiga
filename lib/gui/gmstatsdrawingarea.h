
/*  stats_drawing_area.h
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2006 Damien Sandras
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
				  float vin, float vout, float ain, float aout);

G_END_DECLS

#endif
