#ifndef _STATUSICON_H_
#define _STATUSICON_H_

#include "common.h"
#include "manager.h"

G_BEGIN_DECLS

GtkWidget *gm_statusicon_new (void);

void gm_statusicon_update_full (GtkWidget *widget,
				GMManager::CallingState state,
				IncomingCallMode mode,
				gboolean forward_on_busy);


void gm_statusicon_update_menu (GtkWidget *widget,
				GMManager::CallingState state);

void gm_statusicon_signal_message (GtkWidget *widget,
				   gboolean has_message);

void gm_statusicon_ring (GtkWidget *widget,
			 guint interval);

void gm_statusicon_stop_ringing (GtkWidget *widget);

gboolean gm_statusicon_is_embedded (GtkWidget *widget);

G_END_DECLS

#endif
