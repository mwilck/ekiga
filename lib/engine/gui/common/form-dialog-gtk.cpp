
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         form-dialog.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a gtk+ representation of forms
 *
 */

#include <iostream>

#include "form-dialog-gtk.h"


/* now, one class for each field type -- inline code for readability */

class TitleSubmitter: public Submitter
{
public:

  TitleSubmitter (const std::string _title):
    title (_title)
  { }

  ~TitleSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.title (title);
  }

private:

  const std::string title;
};

class InstructionsSubmitter: public Submitter
{
public:

  InstructionsSubmitter (const std::string _instructions): instructions (_instructions)
  { }

  ~InstructionsSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.instructions (instructions);
  }

private:

  const std::string instructions;
};

class HiddenSubmitter: public Submitter
{
public:

  HiddenSubmitter (const std::string _name,
		   const std::string _value): name(_name), value(_value)
  {}

  ~HiddenSubmitter ()
  {}

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.hidden (name, value);
  }

private:

  const std::string name;
  const std::string value;
};

class BooleanSubmitter: public Submitter
{
public:

  BooleanSubmitter (const std::string _name,
		    const std::string _description,
		    GtkWidget *_widget): name(_name),
					 description(_description),
					 widget(_widget)
  { }

  ~BooleanSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.boolean (name, description,
		     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
  }

private:

  const std::string name;
  const std::string description;
  GtkWidget *widget;
};

class TextSubmitter: public Submitter
{
public:

  TextSubmitter (const std::string _name,
		 const std::string _description,
		 GtkWidget *_widget): name(_name),
				      description(_description),
				      widget(_widget)
  { }

  ~TextSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.text (name, description,
		  gtk_entry_get_text (GTK_ENTRY (widget)));
  }

private:

  const std::string name;
  const std::string description;
  GtkWidget *widget;
};

class PrivateTextSubmitter: public Submitter
{
public:

  PrivateTextSubmitter (const std::string _name,
			const std::string _description,
			GtkWidget *_widget): name(_name),
					     description(_description),
					     widget(_widget)
  { }

  ~PrivateTextSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.private_text (name, description,
			  gtk_entry_get_text (GTK_ENTRY (widget)));
  }

private:

  const std::string name;
  const std::string description;
  GtkWidget *widget;
};

class MultiTextSubmitter: public Submitter
{
public:

  MultiTextSubmitter (const std::string _name,
		      const std::string _description,
		      GtkTextBuffer *_buffer): name(_name),
					       description(_description),
					       buffer(_buffer)
  { }

  ~MultiTextSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    GtkTextIter start;
    GtkTextIter end;

    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);
    builder.multi_text (name, description,
			gtk_text_buffer_get_text (buffer,
						  &start, &end, FALSE));
  }

private:

  const std::string name;
  const std::string description;
  GtkTextBuffer *buffer;
};

enum {
  SINGLE_LIST_COLUMN_VALUE,
  SINGLE_LIST_COLUMN_NAME,
  SINGLE_LIST_COLUMN_NUMBER
};

class SingleListSubmitter: public Submitter
{
public:

  SingleListSubmitter (const std::string _name,
		       const std::string _description,
		       const std::map<std::string, std::string> _choices,
		       GtkWidget *_combo): name(_name),
					   description(_description),
					   choices(_choices),
					   combo(_combo)
  { }

  ~SingleListSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    gchar *cvalue = NULL;
    GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
    GtkTreeIter iter;

    gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter);

    gtk_tree_model_get (model, &iter, SINGLE_LIST_COLUMN_VALUE, &cvalue, -1);

    builder.single_list (name, description,
			 std::string (cvalue), choices);
  }

private:

  const std::string name;
  const std::string description;
  const std::map<std::string, std::string> choices;
  GtkWidget *combo;
};

enum {
  MULTIPLE_LIST_COLUMN_VALUE,
  MULTIPLE_LIST_COLUMN_NAME,
  MULTIPLE_LIST_COLUMN_ACTIVE,
  MULTIPLE_LIST_COLUMN_NUMBER
};

class MultipleListSubmitter: public Submitter
{
public:

  MultipleListSubmitter (const std::string _name,
			 const std::string _description,
			 const std::map<std::string, std::string> _choices,
			 const std::map<std::string, GtkWidget *> _widgets):
    name(_name), description(_description),
    choices(_choices), widgets(_widgets)
  { }

  ~MultipleListSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    std::list<std::string> values;

    for (std::map<std::string, GtkWidget *>::const_iterator iter = widgets.begin ();
	 iter != widgets.end ();
	 iter++)
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (iter->second)))
	values.push_back (iter->first);

    builder.multiple_list (name, description, values, choices);
  }

private:

  const std::string name;
  const std::string description;
  const std::map<std::string, std::string> choices;
  const std::map<std::string, GtkWidget *> widgets;
};


static gboolean
on_delete_event (GtkWidget *,
		 GdkEvent *,
		 gpointer data)
{
  FormDialog *dialog = (FormDialog *)data;

  dialog->cancel ();

  return TRUE;
}

static void
on_cancel_clicked (GtkWidget *,
		   gpointer data)
{
  FormDialog *dialog = (FormDialog *)data;

  dialog->cancel ();
}

static void
on_submit_clicked (GtkWidget *,
		   gpointer data)
{
  FormDialog *dialog = (FormDialog *)data;

  dialog->submit ();
}

FormDialog::FormDialog (Ekiga::FormRequest &_request): request(_request)
{
  GtkWidget *vbox = NULL;
  GtkWidget *buttons = NULL;
  GtkWidget *button = NULL;

  rows = 0;
  loop = g_main_loop_new (NULL, FALSE);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete-event",
		    G_CALLBACK (on_delete_event), this);

  vbox = gtk_vbox_new (FALSE, 5);

  gtk_container_add (GTK_CONTAINER (window), vbox);

  preamble = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (vbox), preamble);

  fields = gtk_table_new (0, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (vbox), fields);

  buttons = gtk_hbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (vbox), buttons);

  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_end (GTK_BOX (buttons), button, FALSE, FALSE, 5);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_cancel_clicked), this);

  button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_box_pack_end (GTK_BOX (buttons), button, FALSE, FALSE, 5);
  g_signal_connect (G_OBJECT (button), "clicked",
		    G_CALLBACK (on_submit_clicked), this);

  request.visit (*this);
}

FormDialog::~FormDialog ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

  if (g_main_loop_is_running (loop))
    g_main_loop_quit (loop);
  gtk_widget_destroy (GTK_WIDGET (window));
  g_main_loop_unref (loop);
  for (std::list<Submitter *>::iterator iter = submitters.begin ();
       iter != submitters.end ();
       iter++)
    delete (*iter);
  submitters.clear ();
}

void
FormDialog::run ()
{
  gtk_widget_show_all (window);
  g_main_loop_run (loop);
}

void
FormDialog::title (const std::string title)
{
  TitleSubmitter *submitter = NULL;

  gtk_window_set_title (GTK_WINDOW (window), title.c_str ());
  submitter = new TitleSubmitter (title);
  submitters.push_back (submitter);
}

void
FormDialog::instructions (const std::string instructions)
{
  GtkWidget *widget = NULL;
  InstructionsSubmitter *submitter = NULL;

  widget = gtk_label_new (instructions.c_str ());
  gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
  gtk_label_set_line_wrap_mode (GTK_LABEL (widget), PANGO_WRAP_WORD);
#endif
  gtk_container_add (GTK_CONTAINER (preamble), widget);
  submitter = new InstructionsSubmitter (instructions);
  submitters.push_back (submitter);
}

void
FormDialog::error (const std::string error)
{
  GtkWidget *widget = NULL;

  if (!error.empty ()) {

    widget = gtk_label_new (NULL);
    gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
    gtk_label_set_line_wrap_mode (GTK_LABEL (widget), PANGO_WRAP_WORD);
#endif
    gtk_label_set_markup (GTK_LABEL (widget),
			  ("<span foreground=\"red\">" + error + "</span>").c_str ());
    gtk_container_add (GTK_CONTAINER (preamble), widget);
  }
}

void
FormDialog::hidden (const std::string name,
		    const std::string value)
{
  HiddenSubmitter *submitter = NULL;

  submitter = new HiddenSubmitter (name, value);
  submitters.push_back (submitter);
}

void
FormDialog::boolean (const std::string name,
		     const std::string description,
		     bool value)
{
  GtkWidget *widget = NULL;
  BooleanSubmitter *submitter = NULL;

  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);

  widget = gtk_check_button_new_with_label (description.c_str ());
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), value);
  gtk_table_attach_defaults (GTK_TABLE (fields), widget,
			     0, 2, rows -1, rows);
  submitter = new BooleanSubmitter (name, description, widget);
  submitters.push_back (submitter);
}

void
FormDialog::text (const std::string name,
		  const std::string description,
		  const std::string value)
{
  GtkWidget *label = NULL;
  GtkWidget *widget = NULL;
  TextSubmitter *submitter = NULL;

  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);

  label = gtk_label_new (description.c_str ());
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
  gtk_label_set_line_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD);
#endif
  widget = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (widget), value.c_str ());
  gtk_table_attach_defaults (GTK_TABLE (fields), label,
			     0, 1, rows -1, rows);
  gtk_table_attach_defaults (GTK_TABLE (fields), widget,
		    1, 2, rows -1, rows);
  submitter = new TextSubmitter (name, description, widget);
  submitters.push_back (submitter);
}

void
FormDialog::private_text (const std::string name,
			  const std::string description,
			  const std::string value)
{
  GtkWidget *label = NULL;
  GtkWidget *widget = NULL;
  PrivateTextSubmitter *submitter = NULL;

  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);

  label = gtk_label_new (description.c_str ());
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
  gtk_label_set_line_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD);
#endif
  widget = gtk_entry_new ();
  gtk_entry_set_visibility (GTK_ENTRY (widget), FALSE);
  gtk_entry_set_text (GTK_ENTRY (widget), value.c_str ());
  gtk_table_attach_defaults (GTK_TABLE (fields), label,
			     0, 1, rows -1, rows);
  gtk_table_attach_defaults (GTK_TABLE (fields), widget,
			     1, 2, rows -1, rows);
  submitter = new PrivateTextSubmitter (name, description, widget);
  submitters.push_back (submitter);
}

void
FormDialog::multi_text (const std::string name,
			const std::string description,
			const std::string value)
{
  GtkWidget *label = NULL;
  GtkWidget *scroller = NULL;
  GtkWidget *widget = NULL;
  GtkTextBuffer *buffer = NULL;
  MultiTextSubmitter *submitter = NULL;

  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);

  label = gtk_label_new (description.c_str ());
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
  gtk_label_set_line_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD);
#endif
  gtk_table_attach_defaults (GTK_TABLE (fields), label,
			     0, 2, rows -1, rows);
  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);

  widget = gtk_text_view_new ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (widget), GTK_WRAP_WORD);
  gtk_text_buffer_set_text (buffer, value.c_str (), -1);
  scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scroller), widget);
  gtk_table_attach_defaults (GTK_TABLE (fields), scroller,
			     0, 2, rows -1, rows);
  submitter = new MultiTextSubmitter (name, description, buffer);
  submitters.push_back (submitter);
}

void
FormDialog::single_list (const std::string name,
			 const std::string description,
			 const std::string value,
			 const std::map<std::string, std::string> choices)
{
  GtkWidget *label = NULL;
  GtkListStore *model = NULL;
  GtkWidget *widget = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeIter iter;
  SingleListSubmitter *submitter = NULL;

  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);

  label = gtk_label_new (description.c_str ());
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
  gtk_label_set_line_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD);
#endif

  model = gtk_list_store_new (SINGLE_LIST_COLUMN_NUMBER,
			      G_TYPE_STRING, G_TYPE_STRING);
  widget = gtk_combo_box_new_with_model (GTK_TREE_MODEL (model));
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (widget), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (widget), renderer,
				  "text", SINGLE_LIST_COLUMN_NAME, NULL);
  for (std::map<std::string, std::string>::const_iterator map_iter
	 = choices.begin ();
       map_iter != choices.end ();
       map_iter++) {

    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter,
			SINGLE_LIST_COLUMN_VALUE, map_iter->first.c_str (),
			SINGLE_LIST_COLUMN_NAME, map_iter->second.c_str (),
			-1);
    if (map_iter->first == value)
      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (widget), &iter);
  }

  gtk_table_attach_defaults (GTK_TABLE (fields), label,
			     0, 1, rows -1, rows);
  gtk_table_attach_defaults (GTK_TABLE (fields), widget,
			     1, 2, rows -1, rows);

  submitter = new SingleListSubmitter (name, description, choices, widget);
  submitters.push_back (submitter);
}

void
FormDialog::multiple_list (const std::string name,
			   const std::string description,
			   const std::list<std::string> values,
			   const std::map<std::string, std::string> choices)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *scroll = NULL;
  std::map<std::string, GtkWidget *> widgets;
  GtkWidget *button = NULL;
  MultipleListSubmitter *submitter = NULL;

  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);

  label = gtk_label_new (description.c_str ());
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
#if GTK_CHECK_VERSION(2,10,0)
  gtk_label_set_line_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD);
#endif
  gtk_table_attach_defaults (GTK_TABLE (fields), label,
			     0, 2, rows -1, rows);


  vbox = gtk_vbox_new (FALSE, 5);
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), vbox);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  for (std::map<std::string, std::string>::const_iterator map_iter
	 = choices.begin ();
       map_iter != choices.end ();
       map_iter++) {

    bool active = (std::find (values.begin (), values.end (), map_iter->first)
		   != values.end ());

    button = gtk_check_button_new_with_label (map_iter->second.c_str ());
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), active);
    gtk_container_add (GTK_CONTAINER (vbox), button);
    widgets[map_iter->first] = button;
  }

  rows++;
  gtk_table_resize (GTK_TABLE (fields), rows, 2);
  gtk_table_attach_defaults (GTK_TABLE (fields), scroll,
			     0, 2, rows -1, rows);

  submitter = new MultipleListSubmitter (name, description, choices, widgets);
  submitters.push_back (submitter);
}

void
FormDialog::submit ()
{
  Ekiga::FormBuilder builder;

  gtk_widget_hide_all (GTK_WIDGET (window));

  for (std::list<Submitter *>::iterator iter = submitters.begin ();
       iter != submitters.end ();
       iter++)
    (*iter)->submit (builder);

  request.submit (builder);

  if (g_main_loop_is_running (loop))
    g_main_loop_quit (loop);
}

void
FormDialog::cancel ()
{
  gtk_widget_hide_all (GTK_WIDGET (window));
  request.cancel ();
  if (g_main_loop_is_running (loop))
    g_main_loop_quit (loop);
}
