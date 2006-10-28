
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         gmcodecsbox.c  -  description
 *                         -------------------------------
 *   begin                : Sat Sep 2 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a codecs box widget
 *
 */


#include "gmcodecsbox.h"

#include <gmconf.h>


/* Columns for the codecs page */
enum {

  COLUMN_CODEC_ACTIVE,
  COLUMN_CODEC_NAME, 
  COLUMN_CODEC_BANDWIDTH,
  COLUMN_CODEC_CLOCKRATE,
  COLUMN_CODEC_CONFIG_NAME,
  COLUMN_CODEC_SELECTABLE,
  COLUMN_CODEC_COLOR,
  COLUMN_CODEC_NUMBER
};


/* GTK+ Callbacks */
static void codec_toggled_cb (GtkCellRendererToggle *cell,
                              gchar *path_str,
                              gpointer data);

static void codec_moved_cb (GtkWidget *widget, 
                            gpointer data);

static void codecs_box_changed_nt (gpointer id, 
                                   GmConfEntry *entry,
                                   gpointer data);

static GSList *gm_codecs_box_to_gm_conf_list (GmCodecsBox *cb);


/* Static functions and declarations */
static void gm_codecs_box_class_init (GmCodecsBoxClass *);

static void gm_codecs_box_init (GmCodecsBox *);

static void gm_codecs_box_destroy (GtkObject *);


static void
codec_toggled_cb (GtkCellRendererToggle *cell,
		  gchar *path_str,
		  gpointer data)
{
  GmCodecsBox *cb = NULL;

  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  GSList *codecs_data = NULL;

  gboolean fixed = FALSE;

  g_return_if_fail (data != NULL);
  g_return_if_fail (GM_IS_CODECS_BOX (data));

  cb = GM_CODECS_BOX (data);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (cb->codecs_list));
  path = gtk_tree_path_new_from_string (path_str);

  /* Update the tree model */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_CODEC_ACTIVE, &fixed, -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      COLUMN_CODEC_ACTIVE, fixed^1, -1);
  gtk_tree_path_free (path);

  /* Update the gmconf key */
  codecs_data = gm_codecs_box_to_gm_conf_list (cb);
  gm_conf_set_string_list (cb->conf_key, codecs_data);
  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);
}


static void
codec_moved_cb (GtkWidget *widget, 
		gpointer data)
{ 	
  GmCodecsBox *cb = NULL;

  GtkTreeIter iter;
  GtkTreeIter *iter2 = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreePath *tree_path = NULL;

  GSList *codecs_data = NULL;

  gchar *path_str = NULL;

  g_return_if_fail (data != NULL);
  g_return_if_fail (GM_IS_CODECS_BOX (data));

  cb = GM_CODECS_BOX (data);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (cb->codecs_list));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (cb->codecs_list));
  gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), 
				   NULL, &iter);

  /* Update the tree view */
  iter2 = gtk_tree_iter_copy (&iter);
  path_str = gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (model), 
						  &iter);
  tree_path = gtk_tree_path_new_from_string (path_str);
  if (!strcmp ((gchar *) g_object_get_data (G_OBJECT (widget), "operation"), 
               "up"))
    gtk_tree_path_prev (tree_path);
  else
    gtk_tree_path_next (tree_path);

  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, tree_path);
  if (gtk_list_store_iter_is_valid (GTK_LIST_STORE (model), &iter)
      && gtk_list_store_iter_is_valid (GTK_LIST_STORE (model), iter2))
    gtk_list_store_swap (GTK_LIST_STORE (model), &iter, iter2);

  /* Scroll to the new position */
  gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (cb->codecs_list), 
				tree_path, NULL, FALSE, 0, 0);

  gtk_tree_path_free (tree_path);
  gtk_tree_iter_free (iter2);
  g_free (path_str);

  /* Update the gmconf key */
  codecs_data = gm_codecs_box_to_gm_conf_list (cb);
  gm_conf_set_string_list (cb->conf_key, codecs_data);
  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);
}


static void
codecs_box_changed_nt (gpointer id, 
                       GmConfEntry *entry,
                       gpointer data)
{
  GmCodecsBox *cb = NULL;
  
  PStringArray *order = NULL;

  g_return_if_fail (data != NULL);
  g_return_if_fail (GM_IS_CODECS_BOX (data));

  cb = GM_CODECS_BOX (data);

  if (gm_conf_entry_get_type (entry) != GM_CONF_LIST) 
    return;

  order = new PStringArray ();
  gm_codecs_box_get_codecs (cb, *order);

  g_signal_emit_by_name (GTK_WIDGET (data), "codecs-box-changed", order);

  delete (order);
}


static GSList *
gm_codecs_box_to_gm_conf_list (GmCodecsBox *cb)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gboolean fixed = FALSE;
  gchar *codec_data = NULL;
  gchar *codec = NULL;

  GSList *codecs_data = NULL;

  g_return_val_if_fail (cb != NULL, NULL);
  g_return_val_if_fail (GM_IS_CODECS_BOX (cb), NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (cb->codecs_list));
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      gtk_tree_model_get (model, &iter, 
                          COLUMN_CODEC_ACTIVE, &fixed,
                          COLUMN_CODEC_CONFIG_NAME, &codec, -1);

      if (!cb->activatable_codecs)
        fixed = true;

      codec_data = 
        g_strdup_printf ("%s=%d", codec, fixed); 

      codecs_data = g_slist_append (codecs_data, codec_data);

      g_free (codec);

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

  return codecs_data;
}


static void
gm_codecs_box_class_init (GmCodecsBoxClass *klass)
{
  static gboolean initialized = FALSE;

  GtkObjectClass *gtkobject_class = NULL;

  gtkobject_class = GTK_OBJECT_CLASS (klass);
  
  gtkobject_class->destroy = gm_codecs_box_destroy;

  if (!initialized) {

    g_signal_new ("codecs-box-changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER, NULL);

    initialized = TRUE;
  }
}


static void
gm_codecs_box_init (GmCodecsBox *cb)
{
  g_return_if_fail (cb != NULL);
  g_return_if_fail (GM_IS_CODECS_BOX (cb));

  cb->media_formats = NULL;
  cb->codecs_list = NULL;
  cb->conf_key = NULL;
}


static void
gm_codecs_box_destroy (GtkObject *object)
{
  GmCodecsBox *cb = NULL;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GM_IS_CODECS_BOX (object));

  cb = GM_CODECS_BOX (object);
  
  if (cb->conf_key) {

    g_free (cb->conf_key);
    cb->conf_key = NULL;
  }

  if (cb->media_formats) {

    delete (cb->media_formats);
    cb->media_formats = NULL;
  }
}


/* Global functions */
GType
gm_codecs_box_get_type (void)
{
  static GType gm_codecs_box_type = 0;
  
  if (gm_codecs_box_type == 0)
  {
    static const GTypeInfo codecs_box_info =
    {
      sizeof (GmCodecsBoxClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_codecs_box_class_init,
      NULL,
      NULL,
      sizeof (GmCodecsBox),
      0,
      (GInstanceInitFunc) gm_codecs_box_init
    };
    
    gm_codecs_box_type =
      g_type_register_static (GTK_TYPE_HBOX,
			      "GmCodecsBox",
			      &codecs_box_info,
			      (GTypeFlags) 0);
  }
  
  return gm_codecs_box_type;
}


GtkWidget *
gm_codecs_box_new (gboolean activatable_codecs,
                   const char *conf_key)
{
  GmCodecsBox *cb = NULL;
  
  GtkWidget *scroll_window = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *button = NULL;

  GtkWidget *buttons_vbox = NULL;
  GtkWidget *alignment = NULL;

  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;

  cb = GM_CODECS_BOX (g_object_new (GM_CODECS_BOX_TYPE, NULL));

  cb->activatable_codecs = activatable_codecs;
  cb->conf_key = g_strdup (conf_key);
  gm_conf_notifier_add (cb->conf_key, 
			codecs_box_changed_nt, 
                        cb);

  cb->codecs_list = gtk_tree_view_new ();
  list_store = gtk_list_store_new (COLUMN_CODEC_NUMBER,
                                   G_TYPE_BOOLEAN,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING,
                                   G_TYPE_BOOLEAN,
                                   G_TYPE_STRING);

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (cb->codecs_list), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (cb->codecs_list), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (cb->codecs_list),0);
  gtk_tree_view_set_model (GTK_TREE_VIEW (cb->codecs_list), 
                           GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (cb->codecs_list), FALSE);

  /* Set all Colums */
  if (activatable_codecs) {

    renderer = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes (NULL,
                                                       renderer,
                                                       "active", 
                                                       COLUMN_CODEC_ACTIVE,
                                                       NULL);
    gtk_tree_view_column_add_attribute (column, renderer, 
                                        "activatable", COLUMN_CODEC_SELECTABLE);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
    gtk_tree_view_append_column (GTK_TREE_VIEW (cb->codecs_list), column);
    g_signal_connect (G_OBJECT (renderer), "toggled",
                      G_CALLBACK (codec_toggled_cb),
                      (gpointer) cb);
  }
  
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", 
                                                     COLUMN_CODEC_NAME,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (cb->codecs_list), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
                                      COLUMN_CODEC_COLOR);
  g_object_set (G_OBJECT (renderer), "weight", "bold", NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", 
                                                     COLUMN_CODEC_BANDWIDTH,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (cb->codecs_list), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
                                      COLUMN_CODEC_COLOR);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", 
                                                     COLUMN_CODEC_CLOCKRATE,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (cb->codecs_list), column);
  gtk_tree_view_column_add_attribute (column, renderer, "foreground", 
                                      COLUMN_CODEC_COLOR);

  
  scroll_window = gtk_scrolled_window_new (FALSE, FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), 
                                  GTK_POLICY_NEVER, 
                                  GTK_POLICY_AUTOMATIC);

  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (frame), -1, 150);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), scroll_window);
  gtk_container_add (GTK_CONTAINER (scroll_window), 
                     GTK_WIDGET (cb->codecs_list));
  gtk_container_set_border_width (GTK_CONTAINER (cb->codecs_list), 0);
  gtk_box_pack_start (GTK_BOX (cb), frame, FALSE, FALSE, 0);


  /* The buttons */
  alignment = gtk_alignment_new (1, 0.5, 0, 0);
  buttons_vbox = gtk_vbutton_box_new ();

  gtk_box_set_spacing (GTK_BOX (buttons_vbox), 4);

  gtk_container_add (GTK_CONTAINER (alignment), buttons_vbox);
  gtk_box_pack_start (GTK_BOX (cb), alignment, 
                      TRUE, TRUE, 5);

  button = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "up");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (codec_moved_cb), 
                    (gpointer) cb);

  button = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, TRUE, TRUE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "down");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (codec_moved_cb), 
                    (gpointer) cb);

  gtk_widget_show_all (GTK_WIDGET (cb));

  return GTK_WIDGET (cb);
}


void 
gm_codecs_box_set_codecs (GmCodecsBox *cb,
                          const OpalMediaFormatList & l)
{
  PStringArray *order = NULL;

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  OpalMediaFormatList added_formats;
  OpalMediaFormatList k;
  PStringList m;

  gchar *bandwidth = NULL;
  gchar *name = NULL;
  gchar *clockrate = NULL;
  gchar *config_name = NULL;
  gchar *selected_codec = NULL;
  gchar **couple = NULL;

  GSList *codecs_data = NULL;
  GSList *codecs_data_iter = NULL;

  int i = 0;

  g_return_if_fail (cb != NULL);


  /* Get the data and the selected codec */
  k = l;
  if (k.GetSize () <= 0)
    return;

  cb->media_formats = new OpalMediaFormatList ();
  for (int i = 0 ; i < l.GetSize () ; i++)
    *cb->media_formats += l [i];
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (cb->codecs_list));
  codecs_data = gm_conf_get_string_list (cb->conf_key); 
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (cb->codecs_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) 
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			COLUMN_CODEC_CONFIG_NAME, &selected_codec, -1);
  gtk_list_store_clear (GTK_LIST_STORE (model));


  /* First we add all codecs in the preferences if they are in the list
   * of possible codecs */
  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {

    couple = g_strsplit ((gchar *) codecs_data_iter->data, "=", 0);

    if (couple [0] && couple [1]) {

      for (int j = 0 ; j < k.GetSize () ; j++) {

        if (k [j].GetEncodingName () != NULL
            && 
            added_formats.FindFormat (k [j].GetPayloadType (), 
                                      k [j].GetClockRate (),
                                      k [j].GetEncodingName ()) == P_MAX_INDEX) {
          name = g_strdup (k [j].GetEncodingName ());
          clockrate = g_strdup_printf ("%d kHz", k [j].GetClockRate ()/1000);
          bandwidth = g_strdup_printf ("%.1f kbps", k [j].GetBandwidth ()/1000.0);
          config_name = g_strdup_printf ("%d|%d|%d", 
                                         k [j].GetPayloadType (),
                                         k [j].GetClockRate (),
                                         k [j].GetBandwidth ());

          if (!strcmp (config_name, couple [0])) {

            gtk_list_store_append (GTK_LIST_STORE (model), &iter);
            gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                COLUMN_CODEC_ACTIVE, !strcmp (couple [1], "1"),
                                COLUMN_CODEC_NAME, name,
                                COLUMN_CODEC_BANDWIDTH, bandwidth,
                                COLUMN_CODEC_CLOCKRATE, clockrate,
                                COLUMN_CODEC_CONFIG_NAME, config_name,
                                COLUMN_CODEC_SELECTABLE, "true",
                                COLUMN_CODEC_COLOR, "black",
                                -1);
            if (selected_codec && !strcmp (selected_codec, config_name))
              gtk_tree_selection_select_iter (selection, &iter);

            added_formats += k [j];
            k.RemoveAt (j);
          }

          g_free (name);
          g_free (config_name);
          g_free (clockrate);
          g_free (bandwidth);
        }
      }
    }

    codecs_data_iter = codecs_data_iter->next;

    g_strfreev (couple);
  }
  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);

  /* #INV: m contains the list of possible codecs from the prefs */

  /* Now we add the remaining codecs */
  for (i = 0 ; i < k.GetSize () ; i++) {

    if (k [i].GetEncodingName () != NULL
        &&
        added_formats.FindFormat (k [i].GetPayloadType (), 
                                  k [i].GetClockRate (),
                                  k [i].GetEncodingName ()) == P_MAX_INDEX) {

      name = g_strdup (k [i].GetEncodingName ());
      clockrate = g_strdup_printf ("%d kHz", k [i].GetClockRate ()/1000);
      bandwidth = g_strdup_printf ("%.1f kbps", k [i].GetBandwidth ()/1000.0);
      config_name = g_strdup_printf ("%d|%d|%d", 
                                     k [i].GetPayloadType (),
                                     k [i].GetClockRate (),
                                     k [i].GetBandwidth ());

      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          COLUMN_CODEC_ACTIVE, FALSE,
                          COLUMN_CODEC_NAME, name,
                          COLUMN_CODEC_BANDWIDTH, bandwidth,
                          COLUMN_CODEC_CLOCKRATE, clockrate,
                          COLUMN_CODEC_CONFIG_NAME, config_name,
                          COLUMN_CODEC_SELECTABLE, "true",
                          COLUMN_CODEC_COLOR, "black",
                          -1);

      if (selected_codec && !strcmp (selected_codec, config_name))
        gtk_tree_selection_select_iter (selection, &iter);

      g_free (bandwidth);
      g_free (name);
      g_free (config_name);
      g_free (clockrate);

      added_formats += k[i];
    }
  }

  /* Update the gmconf key */
  codecs_data = gm_codecs_box_to_gm_conf_list (cb);
  gm_conf_set_string_list (cb->conf_key, codecs_data);
  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);

  order = new PStringArray ();
  gm_codecs_box_get_codecs (cb, *order);

  g_signal_emit_by_name (GTK_WIDGET (cb), "codecs-box-changed", order);

  delete (order);
}


void 
gm_codecs_box_get_codecs (GmCodecsBox *cb,
                          PStringArray & order)
{
  GSList *codecs_data_iter = NULL;
  GSList *codecs_data = NULL;
  gchar **couple = NULL;
  gchar **codec_info = NULL;

  OpalMediaFormat format;

  g_return_if_fail (cb != NULL);
  g_return_if_fail (GM_IS_CODECS_BOX (cb));

  /* Read the codecs in the config to add them with the correct order */ 
  codecs_data = gm_conf_get_string_list (cb->conf_key);
  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {

    couple = g_strsplit ((gchar *) codecs_data_iter->data, "=", 0);

    if (couple && couple [0] && couple [1] 
        && (!strcmp (couple [1], "1") || !cb->activatable_codecs)) { 

      codec_info = g_strsplit ((gchar *) couple [0], "|", 0);
      if (codec_info && codec_info [0] && codec_info [1] && codec_info [2]) {

        for (int i = 0 ; i < cb->media_formats->GetSize () ; i++) {

          format = ((OpalMediaFormatList) (*cb->media_formats)) [i];

          if (format.GetPayloadType () == (atoi (codec_info [0]))
              && format.GetBandwidth () == (unsigned) (atoi (codec_info [2]))
              && format.GetClockRate () == (unsigned) (atoi (codec_info [1]))) 
            order += format;
        }
      }

      g_strfreev (codec_info);
    }

    g_strfreev (couple);
    codecs_data_iter = g_slist_next (codecs_data_iter);
  }

  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);
}

