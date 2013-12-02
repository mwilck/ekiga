 
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
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
 *                         pref_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <miguelrp@gmail.com>
 */


#include "config.h"

#include <glib/gi18n.h>

#include "ekiga-settings.h"

#include "preferences-window.h"
#include "default_devices.h"

#include "scoped-connections.h"

#include "gmwindow.h"
#include "codecsbox.h"

#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"

#include "device-lists.h"

#ifdef WIN32
#include "platform/winpaths.h"
#endif

#define GTK_GRID_LAST_ROW(g, p) while (gtk_grid_get_child_at (GTK_GRID (g), 0, p++));


typedef struct _GmPreferencesWindow
{
  _GmPreferencesWindow();
  ~_GmPreferencesWindow();

  GtkWidget *audio_codecs_list;
  GtkWidget *sound_events_list;
  GtkWidget *audio_player;
  GtkWidget *sound_events_output;
  GtkWidget *audio_recorder;
  GtkWidget *video_device;
  GtkWidget *iface;
  GtkWidget *fsbutton;
  boost::shared_ptr<Ekiga::VideoInputCore> videoinput_core;
  boost::shared_ptr<Ekiga::AudioInputCore> audioinput_core;
  boost::shared_ptr<Ekiga::AudioOutputCore> audiooutput_core;
  boost::shared_ptr<Ekiga::Settings> protocols_settings;
  boost::shared_ptr<Ekiga::Settings> sip_settings;
  boost::shared_ptr<Ekiga::Settings> h323_settings;
  boost::shared_ptr<Ekiga::Settings> nat_settings;
  boost::shared_ptr<Ekiga::Settings> call_forwarding_settings;
  boost::shared_ptr<Ekiga::Settings> call_options_settings;
  boost::shared_ptr<Ekiga::Settings> personal_data_settings;
  boost::shared_ptr<Ekiga::Settings> sound_events_settings;
  boost::shared_ptr<Ekiga::Settings> audio_devices_settings;
  boost::shared_ptr<Ekiga::Settings> audio_codecs_settings;
  boost::shared_ptr<Ekiga::Settings> video_devices_settings;
  boost::shared_ptr<Ekiga::Settings> video_codecs_settings;
  boost::shared_ptr<Ekiga::Settings> video_display_settings;
  Ekiga::scoped_connections connections;
} GmPreferencesWindow;

#define GM_PREFERENCES_WINDOW(x) (GmPreferencesWindow *) (x)

_GmPreferencesWindow::_GmPreferencesWindow()
{
  sound_events_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (SOUND_EVENTS_SCHEMA));
  audio_devices_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (AUDIO_DEVICES_SCHEMA));
  audio_codecs_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (AUDIO_CODECS_SCHEMA));
  video_devices_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_DEVICES_SCHEMA));
  video_codecs_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_CODECS_SCHEMA));
  video_display_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (VIDEO_DISPLAY_SCHEMA));
  personal_data_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (PERSONAL_DATA_SCHEMA));
  protocols_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (PROTOCOLS_SCHEMA));
  sip_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (SIP_SCHEMA));
  h323_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (H323_SCHEMA));
  nat_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (NAT_SCHEMA));
  call_forwarding_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CALL_FORWARDING_SCHEMA));
  call_options_settings =
    boost::shared_ptr<Ekiga::Settings> (new Ekiga::Settings (CALL_OPTIONS_SCHEMA));
}

_GmPreferencesWindow::~_GmPreferencesWindow()
{
}

enum {
  COLUMN_STRING_RAW = 0,
  COLUMN_STRING_TRANSLATED,
  COLUMN_SENSITIVE,
  COLUMN_GSETTINGS,
};

typedef struct _GnomePrefsWindow {

  GtkWidget *notebook;
  GtkWidget *section_label;
  GtkWidget *sections_tree_view;
  GtkTreeIter iter;
  int last_page;

} GnomePrefsWindow;

/* Declarations */

/* GUI Functions */


/* DESCRIPTION  : /
 * BEHAVIOR     : Frees a GmPreferencesWindow and its content.
 * PRE          : A non-NULL pointer to a GmPreferencesWindow structure.
 */
static void gm_pw_destroy (gpointer prefs_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a pointer to the private GmPrerencesWindow structure
 *                used by the preferences window GMObject.
 * PRE          : The given GtkWidget pointer must be a preferences window
 * 		  GMObject.
 */
static GmPreferencesWindow *gm_pw_get_pw (GtkWidget *preferences_window);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the sound events list of the preferences window.
 * PRE          :  /
 */
static void gm_prefs_window_sound_events_list_build (GtkWidget *prefs_window);


/* DESCRIPTION  : /
 * BEHAVIOR     : Adds an update button connected to the given callback to
 * 		  the given GtkBox.
 * PRE          : A valid pointer to the container widget where to attach
 *                the button, followed by a label, the callback, a
 *                tooltip and the alignment.
 */
static void gm_pw_add_update_button (GtkWidget *box,
                                     const char *label,
                                     GCallback func,
                                     const gchar *tooltip,
                                     gfloat valign,
                                     gpointer data);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the general settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_general_page (GtkWidget *prefs_window,
                                     GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the interface settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_interface_page (GtkWidget *prefs_window,
                                       GtkWidget *container);



/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the call options page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_call_options_page (GtkWidget *prefs_window,
                                          GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the sound events settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_sound_events_page (GtkWidget *prefs_window,
                                          GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the H.323 settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_h323_page (GtkWidget *prefs_window,
                                  GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the SIP settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_sip_page (GtkWidget *prefs_window,
                                 GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio devices settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_audio_devices_page (GtkWidget *prefs_window,
                                           GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video devices settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_video_devices_page (GtkWidget *prefs_window,
                                           GtkWidget *container);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the codecs settings page.
 * PRE          : A valid pointer to the preferences window GMObject, and to the
 * 		  container widget where to attach the generated page.
 */
static void gm_pw_init_audio_codecs_page (GtkWidget *prefs_window,
                                          GtkWidget *container);
static void gm_pw_init_video_codecs_page (GtkWidget *prefs_window,
                                          GtkWidget *container);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkEntry associated with a config key and returns
 *                 the result.
 *                 The first parameter is the section in which
 *                 the GtkEntry should be attached. The other parameters are
 *                 the text label, the config key, the tooltip.
 * PRE          :  /
 */
GtkWidget * gm_pw_entry_new (GtkWidget* grid,
                             const gchar *label_txt,
                             boost::shared_ptr<Ekiga::Settings> settings,
                             const std::string & key,
                             const gchar *tooltip);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkHScale associated with a config key and
 *                 returns the result.
 *                 The first parameter is the section in which
 *                 the GtkHScale should be attached. The other parameters
 *                 are the text labels, the config key, the tooltip, the
 *                 minimal and maximal values and the incrementation step.
 * PRE          :  /
 */
GtkWidget *gm_pw_scale_new (GtkWidget* grid,
                            const gchar *down_label_txt,
                            const gchar *up_label_txt,
                            boost::shared_ptr <Ekiga::Settings> settings,
                            const std::string & key,
                            const gchar *tooltip,
                            double min,
                            double max,
                            double step);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkSpinButton associated with a config key
 *                 and add it to the given GtkGrid container.
 *                 The first parameter is the section in which
 *                 the GtkSpinButton should be attached. The other parameters
 *                 are the text label(s), the config key, the tooltip, the
 *                 minimal and maximal values and the incrementation step.
 * PRE          :  /
 */
GtkWidget *gm_pw_spin_new (GtkWidget* grid,
                           const gchar *label_txt,
                           const gchar *label_txt2,
                           boost::shared_ptr <Ekiga::Settings> settings,
                           const std::string & key,
                           const gchar *tooltip,
                           double min,
                           double max,
                           double step);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkToggleButton associated with a config key
 *                 and add it to the given GtkGrid container.
 *                 The first parameter is the section in which
 *                 the GtkWidget should be attached. The other parameters are
 *                 the text label, the config key, the tooltip.
 * PRE          :  /
 */
GtkWidget * gm_pw_toggle_new (GtkWidget* grid,
                              const gchar *label_txt,
                              boost::shared_ptr <Ekiga::Settings> settings,
                              const std::string & key,
                              const gchar *tooltip);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new prefs window. The parameter is a filename
 *                 corresponding to the logo displayed by default. Returns
 *                 the created window which still has to be connected to the
 *                 signals.
 * PRE          :  /
 */
GtkWidget *gm_pw_window_new (const gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new section in the given prefs window.
 *                 The parameter are the prefs window and the prefs
 *                 window section name.
 * PRE          :  /
 */
void gm_pw_window_section_new (GtkWidget *,
                               const gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a subsection inside a section of a prefs window.
 *                 The parameters are the prefs window and the section name.
 * PRE          :  Not NULL name.
 */
void gm_pw_subsection_new (GtkWidget *container,
                           const gchar *name);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new subsection in the given prefs window and
 *                 returns it. The parameter are the prefs window and the
 *                 prefs window subsection name. General subsections can
 *                 be created in the returned gnome prefs window subsection
 *                 and widgets can be attached to them.
 * PRE          :  /
 */
GtkWidget *gm_pw_window_subsection_new (GtkWidget *,
                                        const gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkOptionMenu associated with a string config
 *                 key and returns the result.
 *                 The first parameter is the section in which the GtkEntry
 *                 should be attached. The other parameters are the text label,
 *                 the possible values for the menu, the config key, the
 *                 tooltip.
 * PRE          :  The array ends with NULL.
 */
static GtkWidget *gm_pw_string_option_menu_new (GtkWidget *,
                                                const gchar *,
                                                const gchar **,
                                                boost::shared_ptr <Ekiga::Settings>,
                                                const std::string &,
                                                const gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the content of a GtkOptionMenu associated with
 *                 a string config key. The first parameter is the menu,
 *                 the second is the array of possible values, and the
 *                 last parameters are config related.
 * PRE          :  The array ends with NULL.
 */
static void gm_pw_string_option_menu_update (GtkWidget *option_menu,
                                             const gchar **options,
                                             boost::shared_ptr <Ekiga::Settings>,
                                             const std::string & key);

static void gm_pw_string_option_menu_add (GtkWidget *option_menu,
                                          const std::string & option,
                                          boost::shared_ptr <Ekiga::Settings>,
                                          const std::string & key);

static void gm_pw_string_option_menu_remove (GtkWidget *option_menu,
                                             const std::string & option,
                                             boost::shared_ptr <Ekiga::Settings>,
                                             const std::string & key);


/* Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks in the
 *                 categories and subcategories GtkTreeView.
 * BEHAVIOR     :  Display the logo if he clicked in a category, or the
 *                 different blocks of options corresponding to a subcategory
 *                 if he clicked in a specific subcategory.
 * PRE          :  /
 */
static void tree_selection_changed_cb (GtkTreeSelection *selection,
                                       gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the refresh devices list button in the prefs.
 * BEHAVIOR     :  Redetects the devices and refreshes the menu.
 * PRE          :  /
 */
static void refresh_devices_list_cb (GtkWidget *widget,
                                     gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list.
 * BEHAVIOR     :  It udpates the GtkFileChooser's filename to the config
 *                 value for the key corresponding to the currently
 *                 selected sound event.
 * PRE          :  /
 */
static void sound_event_selected_cb (GtkTreeSelection *selection,
                                     gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on the play button in the sound events list.
 * BEHAVIOR     :  Plays the currently selected sound event using the 
 * 		   selected audio player and plugin through a GMSoundEvent.
 * PRE          :  The entry.
 */
static void sound_event_play_cb (GtkWidget *widget,
                                 gpointer data);


/* DESCRIPTION  :  This callback is called when the user clicks
 *                 on a sound event in the list and change the toggle.
 * BEHAVIOR     :  It udpates the config key associated with the currently
 *                 selected sound event so that it reflects the state of the
 *                 sound event (enabled or disabled) and also updates the list.
 * PRE          :  /
 */
static void sound_event_toggled_cb (GtkCellRendererToggle *cell,
                                    gchar *path_str,
                                    gpointer data);


/* DESCRIPTION  :  This callback is called when the user selected a file
 *                 with the file selector button for the image
 * BEHAVIOR     :  Update of the config database.
 * PRE          :  /
 */
/*
   static void image_filename_browse_cb (GtkWidget *widget,
   gpointer data);
 */

/* DESCRIPTION  :  This callback is called when the user selected a file
 *                 for a sound event
 * BEHAVIOR     :  Update of the config database.
 * PRE          :  /
 */
static void sound_event_changed_cb (GtkWidget *widget,
                                    gpointer data);


/* DESCRIPTION  :  This callback is called when something changes in the sound
 *                 event related settings.
 * BEHAVIOR     :  It updates the events list widget.
 * PRE          :  A pointer to the prefs window GMObject.
 */
static void sound_event_setting_changed (GSettings *,
                                         gchar *,
                                         gpointer data);


void 
gm_prefs_window_update_devices_list (GtkWidget *prefs_window);

/* Implementation */
static void
gm_pw_destroy (gpointer prefs_window)
{
  g_return_if_fail (prefs_window != NULL);

  delete ((GmPreferencesWindow *) prefs_window);
}


static GmPreferencesWindow *
gm_pw_get_pw (GtkWidget *preferences_window)
{
  g_return_val_if_fail (preferences_window != NULL, NULL);

  return GM_PREFERENCES_WINDOW (g_object_get_data (G_OBJECT (preferences_window), "GMObject"));
}


static void
gm_prefs_window_sound_events_list_build (GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  bool enabled = FALSE;

  pw = gm_pw_get_pw (prefs_window);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pw->sound_events_list));
  gtk_list_store_clear (GTK_LIST_STORE (model));

  /* Sound on incoming calls */
  enabled = pw->sound_events_settings->get_bool ("enable-incoming-call-sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      0, enabled,
                      1, _("Play sound on incoming calls"),
                      2, "incoming-call-sound",
                      3, "enable-incoming-call-sound",
                      4, "incoming-call-sound",
                      -1);

  enabled = pw->sound_events_settings->get_bool ("enable-ring-tone-sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      0, enabled,
                      1, _("Play ring tone"),
                      2, "ring-tone-sound",
                      3, "enable-ring-tone-sound",
                      4, "ring-tone-sound",
                      -1);

  enabled = pw->sound_events_settings->get_bool ("enable-busy-tone-sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      0, enabled,
                      1, _("Play busy tone"),
                      2, "busy-tone-sound",
                      3, "enable-busy-tone-sound",
                      4, "busy-tone-sound",
                      -1);

  enabled = pw->sound_events_settings->get_bool ("enable-new-voicemail-sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      0, enabled,
                      1, _("Play sound for new voice mails"),
                      2, "new-voicemail-sound",
                      3, "enable-new-voicemail-sound",
                      4, "new-voicemail-sound",
                      -1);

  enabled = pw->sound_events_settings->get_bool ("enable-new-message-sound");
  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      0, enabled,
                      1, _("Play sound for new instant messages"),
                      2, "new-message-sound",
                      3, "enable-new-message-sound",
                      4, "new-message-sound",
                      -1);
}


static void
gm_pw_add_update_button (GtkWidget *container,
                         const char *label,
                         GCallback func,
                         const gchar *tooltip,
                         gfloat valign,
                         gpointer data)
{
  GtkWidget* alignment = NULL;
  GtkWidget* button = NULL;

  int pos = 0;

  /* Update Button */
  button = gtk_button_new_with_mnemonic (label);
  gtk_widget_set_tooltip_text (button, tooltip);

  alignment = gtk_alignment_new (1, valign, 0, 0);
  gtk_container_add (GTK_CONTAINER (alignment), button);
  gtk_container_set_border_width (GTK_CONTAINER (button), 6);

  GTK_GRID_LAST_ROW (container, pos);
  gtk_grid_attach (GTK_GRID (container), alignment, 0, pos-1, 2, 1);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (func),
                    (gpointer) data);
}


static void
gm_pw_init_general_page (GtkWidget *prefs_window,
                         GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GmPreferencesWindow *pw = NULL;

  pw = gm_pw_get_pw (prefs_window);

  /* Personal Information */
  gm_pw_subsection_new (container, _("Personal Information"));

  entry = gm_pw_entry_new (container, _("_Full name:"),
                           pw->personal_data_settings, "full-name",
                           _("Enter your full name"));
  gtk_widget_set_size_request (GTK_WIDGET (entry), 250, -1);
  gtk_entry_set_max_length (GTK_ENTRY (entry), 65);
}


static void
gm_pw_init_interface_page (GtkWidget *prefs_window,
                           GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;

  pw = gm_pw_get_pw (prefs_window);

  /* Video Display */
  gm_pw_subsection_new (container, _("Video Display"));

  gm_pw_toggle_new (container, _("Place windows displaying video _above other windows"),
                    pw->video_display_settings, "stay-on-top",
                    _("Place windows displaying video above other windows during calls"));

  /* Network Settings */
  gm_pw_subsection_new (container, _("Network Settings"));

  gm_pw_spin_new (container, _("Type of Service (TOS):"), NULL,
                  pw->protocols_settings, "rtp-tos-field",
                  _("The Type of Service (TOS) byte on outgoing RTP IP packets. This byte is used by the network to provide some level of Quality of Service (QoS). Default value 184 (0xB8) corresponds to Expedited Forwarding (EF) PHB as defined in RFC 3246."),
                  0.0, 255.0, 1.0);

  gm_pw_toggle_new (container, _("Enable network _detection"),
                    pw->nat_settings, "enable-stun",
                    _("Enable the automatic network setup resulting from the STUN test"));
}

static void
gm_pw_init_call_options_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;

  pw = gm_pw_get_pw (prefs_window);

  gm_pw_subsection_new (container, _("Call Forwarding"));

  gm_pw_toggle_new (container, _("_Always forward calls to the given host"),
                    pw->call_forwarding_settings, "always-forward",
                    _("If enabled, all incoming calls will be forwarded to the host that is specified in the protocol settings"));

  gm_pw_toggle_new (container, _("Forward calls to the given host if _no answer"),
                    pw->call_forwarding_settings, "forward-on-no-answer",
                    _("If enabled, all incoming calls will be forwarded to the host that is specified in the protocol settings if you do not answer the call"));

  gm_pw_toggle_new (container, _("Forward calls to the given host if _busy"),
                    pw->call_forwarding_settings, "forward-on-busy",
                    _("If enabled, all incoming calls will be forwarded to the host that is specified in the protocol settings if you already are in a call or if you are in busy mode"));


  gm_pw_subsection_new (container, _("Call Options"));

  /* Translators: the full sentence is Forward calls after x seconds. */
  gm_pw_spin_new (container, _("Forward calls after"), _("seconds"),
                  pw->call_options_settings, "no-answer-timeout",
                  _("Automatically reject or forward incoming calls if no answer is given after the specified amount of time (in seconds)"), 10.0, 299.0, 1.0);
  gm_pw_toggle_new (container, _("_Automatically answer incoming calls"),
                    pw->call_options_settings, "auto-answer",
                    _("If enabled, automatically answer incoming calls"));
}


static void
gm_pw_init_sound_events_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GmPreferencesWindow *pw= NULL;

  GtkWidget *button = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *selector_hbox = NULL;
  GtkWidget *selector_playbutton = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *column = NULL;

  GtkCellRenderer *renderer = NULL;

  GtkFileFilter *filefilter = NULL;

  PStringArray devs;

  pw = gm_pw_get_pw (prefs_window);

  gm_pw_subsection_new (container, _("Ekiga Sound Events"));

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_grid_attach (GTK_GRID (container), vbox, 0, 0, 1, 1);

  /* The 3rd column will be invisible and contain the config key containing
     the file to play. The 4th one contains the key determining if the
     sound event is enabled or not. */
  list_store =
    gtk_list_store_new (5,
                        G_TYPE_BOOLEAN,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_STRING);

  pw->sound_events_list =
    gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pw->sound_events_list), TRUE);

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), pw->sound_events_list);
  gtk_container_set_border_width (GTK_CONTAINER (pw->sound_events_list), 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);


  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (_("A"),
                                                     renderer,
                                                     "active",
                                                     0,
                                                     NULL);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  g_signal_connect (renderer, "toggled",
                    G_CALLBACK (sound_event_toggled_cb), (gpointer) prefs_window);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Event"),
                                                     renderer,
                                                     "text",
                                                     1,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (pw->sound_events_list), column);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 325);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  pw->fsbutton =
    gtk_file_chooser_button_new (_("Choose a sound"),
                                 GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_box_pack_start (GTK_BOX (hbox), pw->fsbutton, TRUE, TRUE, 2);

  filefilter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filefilter, _("Wavefiles"));
#ifdef WIN32
  gtk_file_filter_add_pattern (filefilter, "*.wav");
#else
  gtk_file_filter_add_mime_type (filefilter, "audio/x-wav");
#endif
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (pw->fsbutton), filefilter);

  selector_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  selector_playbutton = gtk_button_new_with_label (_("Play"));
  gtk_box_pack_end (GTK_BOX (selector_hbox),
                    selector_playbutton, FALSE, FALSE, 0);
  gtk_widget_show (selector_playbutton);
  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (pw->fsbutton),
                                     selector_hbox);

  g_signal_connect (pw->fsbutton, "selection-changed",
                    G_CALLBACK (sound_event_changed_cb),
                    (gpointer) prefs_window);

  g_signal_connect (selection, "changed",
                    G_CALLBACK (sound_event_selected_cb),
                    (gpointer) prefs_window);

  button = gtk_button_new_with_label (_("Play"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (sound_event_play_cb),
                    (gpointer) prefs_window);

  /* Place it after the signals so that we can make sure they are run if
     required */
  gm_prefs_window_sound_events_list_build (prefs_window);
}


static void
gm_pw_init_h323_page (GtkWidget *prefs_window,
                      GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GmPreferencesWindow *pw = gm_pw_get_pw (prefs_window);

  const gchar *capabilities [] =
    {_("String"),
      _("Tone"),
      _("RFC2833"),
      _("Q.931"),
      NULL};

  const gchar *roles [] =
    { _("Disable H.239 Extended Video"),
      _("Allow H.239 per Content Role Mask"),
      _("Force H.239 Presentation Role"),
      _("Force H.239 Live Role"),
      NULL };

  /* Add Misc Settings */
  gm_pw_subsection_new (container, _("Misc Settings"));

  entry =
    gm_pw_entry_new (container, _("Forward _URI:"),
                     pw->h323_settings, "forward-host",
                     _("The host where calls should be forwarded if call forwarding is enabled"));
  if (!g_strcmp0 (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), "h323:");

  /* Packing widget */
  gm_pw_subsection_new (container, _("Advanced Settings"));

  /* The toggles */
  gm_pw_toggle_new (container, _("Enable H.245 _tunneling"),
                    pw->h323_settings, "enable-h245-tunneling",
                    _("This enables H.245 Tunneling mode. In H.245 Tunneling mode H.245 messages are encapsulated into the H.225 channel (port 1720). This saves one TCP connection during calls. H.245 Tunneling was introduced in H.323v2."));

  gm_pw_toggle_new (container, _("Enable _early H.245"),
                    pw->h323_settings, "enable-early-h245",
                    _("This enables H.245 early in the setup"));

  gm_pw_toggle_new (container, _("Enable fast _start procedure"), pw->h323_settings,
                    "enable-fast-start", _("Connection will be established in Fast Start (Fast Connect) mode. Fast Start is a new way to start calls faster that was introduced in H.323v2."));

  gm_pw_toggle_new (container, _("Enable H.239 control"), pw->h323_settings,
                    "enable-h239", _("This enables H.239 capability for additional video roles."));

  gm_pw_string_option_menu_new (container, NULL, roles,
                                pw->h323_settings, "video-role",
                                _("Select the H.239 Video Role"));

  /* Packing widget */
  gm_pw_subsection_new (container, _("DTMF Mode"));

  gm_pw_string_option_menu_new (container, _("_Send DTMF as:"), capabilities,
                                pw->h323_settings, "dtmf-mode",
                                _("Select the mode for DTMFs sending"));
}


static void
gm_pw_init_sip_page (GtkWidget *prefs_window,
                     GtkWidget *container)
{
  GtkWidget *entry = NULL;
  GmPreferencesWindow *pw = gm_pw_get_pw (prefs_window);

  const gchar *capabilities [] =
    {
      _("RFC2833"),
      _("INFO"),
      NULL
    };

  /* Add Misc Settings */
  gm_pw_subsection_new (container, _("Misc Settings"));

  gm_pw_entry_new (container, _("_Outbound proxy:"),
                   pw->sip_settings, "outbound-proxy-host",
                   _("The SIP Outbound Proxy to use for outgoing calls"));

  entry =
    gm_pw_entry_new (container, _("Forward _URI:"),
                     pw->sip_settings, "forward-host",
                     _("The host where calls should be forwarded if call forwarding is enabled"));

  if (!g_strcmp0 (gtk_entry_get_text (GTK_ENTRY (entry)), ""))
    gtk_entry_set_text (GTK_ENTRY (entry), "sip:");

  /* Packing widget */
  gm_pw_subsection_new (container, _("DTMF Mode"));

  gm_pw_string_option_menu_new (container, _("_Send DTMF as:"), capabilities,
                                pw->sip_settings, "dtmf-mode",
                                _("Select the mode for DTMFs sending"));
}


static void
gm_pw_init_audio_devices_page (GtkWidget *prefs_window,
                               GtkWidget *container)
{
  PStringArray devs;
  gchar **array = NULL;
  GmPreferencesWindow *pw = NULL;

  pw = gm_pw_get_pw (prefs_window);

  /* Add all the fields */
  gm_pw_subsection_new (container, _("Audio Devices"));

  /* Add all the fields for the audio manager */
  std::vector <std::string> device_list;

  get_audiooutput_devices (pw->audiooutput_core, device_list);
  array = vector_of_string_to_array (device_list);
  pw->sound_events_output =
    gm_pw_string_option_menu_new (container,
                                  _("Ringing device:"),
                                  (const gchar **) array,
                                  pw->sound_events_settings,
                                  "output-device",
                                  _("Select the ringing audio device to use"));
  pw->audio_player =
    gm_pw_string_option_menu_new (container,
                                  _("Output device:"),
                                  (const gchar **) array,
                                  pw->audio_devices_settings,
                                  "output-device",
                                  _("Select the audio output device to use"));
  g_free (array);

  /* The recorder */
  get_audioinput_devices (pw->audioinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  pw->audio_recorder =
    gm_pw_string_option_menu_new (container,
                                  _("Input device:"),
                                  (const gchar **) array,
                                  pw->audio_devices_settings,
                                  "input-device",
                                  _("Select the audio input device to use"));
  g_free (array);

  /* That button will refresh the device list */
  gm_pw_add_update_button (container, _("_Detect devices"),
                           G_CALLBACK (refresh_devices_list_cb),
                           _("Click here to refresh the device list"), 1,
                           prefs_window);
}


static void
gm_pw_init_video_devices_page (GtkWidget *prefs_window,
                               GtkWidget *container)
{
  GmPreferencesWindow *pw = NULL;

  PStringArray devs;

  gchar **array = NULL;
  gchar *video_size[NB_VIDEO_SIZES+1];
  const gchar *video_sizes_text [] =
    {
      _("Small"),
      _("Medium"),
      _("Large"),
      _("Extra Large"),
      _("480p HD"),
    };

  unsigned int i;

  for (i=0; i< NB_VIDEO_SIZES; i++)
    video_size[i] = g_strdup_printf ("%s (%dx%d)",
                                     video_sizes_text[i < 5 ? i : 4],
                                     Ekiga::VideoSizes[i].width,
                                     Ekiga::VideoSizes[i].height);
  video_size [NB_VIDEO_SIZES] = NULL;

  const gchar *video_format [] =
    {
      _("PAL (Europe)"),
      _("NTSC (America)"),
      _("SECAM (France)"),
      _("Auto"),
      NULL
    };

  pw = gm_pw_get_pw (prefs_window);

  std::vector <std::string> device_list;

  /* The video devices related options */
  gm_pw_subsection_new (container, _("Video Devices"));

  /* The video device */
  get_videoinput_devices (pw->videoinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  pw->video_device =
    gm_pw_string_option_menu_new (container, _("Input device:"), (const gchar **)array, pw->video_devices_settings, "input-device", _("Select the video input device to use. If an error occurs when using this device a test picture will be transmitted."));
  g_free (array);

  gm_pw_string_option_menu_new (container, _("Size:"), (const gchar**)video_size, pw->video_devices_settings, "size", _("Select the transmitted video size"));

  gm_pw_subsection_new (container, _("Advanced"));

  gm_pw_string_option_menu_new (container, _("Format:"), video_format, pw->video_devices_settings, "format", _("Select the format for video cameras (does not apply to most USB cameras)"));

  gm_pw_spin_new (container, _("Channel:"), NULL,
                  pw->video_devices_settings, "channel",
                  _("The video channel number to use (to select camera, tv or other sources)"), 0.0, 10.0, 1.0);

  /* That button will refresh the device list */
  gm_pw_add_update_button (container, _("_Detect devices"), G_CALLBACK (refresh_devices_list_cb), _("Click here to refresh the device list"), 1, prefs_window);

  for (i=0; i< NB_VIDEO_SIZES; i++)
    g_free (video_size[i]);
}


static void
gm_pw_init_audio_codecs_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GtkWidget *codecs_list = NULL;
  GmPreferencesWindow *pw = NULL;

  int pos = 0;

  pw = gm_pw_get_pw (prefs_window);

  /* Packing widgets */
  gm_pw_subsection_new (container, _("Codecs"));

  GTK_GRID_LAST_ROW (container, pos);
  codecs_list = codecs_box_new_with_type (Ekiga::Call::Audio);
  gtk_container_set_border_width (GTK_CONTAINER (codecs_list), 12);
  gtk_grid_attach (GTK_GRID (container), codecs_list, 0, pos-1, 2, 1);

  /* Here we add the audio codecs options */
  gm_pw_subsection_new (container, _("Settings"));

  /* Translators: the full sentence is Automatically adjust jitter buffer
     between X and Y ms */
  gm_pw_toggle_new (container, _("Enable silence _detection"),
                    pw->audio_codecs_settings, "enable-silence-detection",
                    _("If enabled, use silence detection with the codecs supporting it"));

  gm_pw_toggle_new (container, _("Enable echo can_celation"),
                    pw->audio_codecs_settings, "enable-echo-cancellation",
                    _("If enabled, use echo cancellation"));

  /* Translators: the full sentence is Maximum jitter buffer of x ms. */
  gm_pw_spin_new (container, _("Maximum _jitter buffer of"), _("ms"),
                  pw->audio_codecs_settings, "maximum-jitter-buffer",
                  _("The maximum jitter buffer size for audio reception (in ms)"),
                  20.0, 2000.0, 50.0);
}


static void
gm_pw_init_video_codecs_page (GtkWidget *prefs_window,
                              GtkWidget *container)
{
  GtkWidget *codecs_list = NULL;
  int pos = 0;

  GmPreferencesWindow *pw = NULL;

  pw = gm_pw_get_pw (prefs_window);

  /* Packing widgets */
  gm_pw_subsection_new (container, _("Codecs"));

  GTK_GRID_LAST_ROW (container, pos);
  codecs_list = codecs_box_new_with_type (Ekiga::Call::Video);
  gtk_container_set_border_width (GTK_CONTAINER (codecs_list), 12);
  gtk_grid_attach (GTK_GRID (container), codecs_list, 0, pos-1, 2, 1);

  /* Here we add the video codecs options */
  gm_pw_subsection_new (container, _("Settings"));

  /* Translators: the full sentence is Keep a minimum video quality of X % */
  gm_pw_scale_new (container, _("Picture quality"), _("Frame rate"),
                   pw->video_codecs_settings, "temporal-spatial-tradeoff",
                   _("Choose if you want to guarantee a minimum image quality (possibly leading to dropped frames in order not to surpass the bitrate limit) or if you prefer to keep the frame rate"),
                   0.0, 32.0, 1.0);

  /* Translators: the full sentence is Maximum video bitrate of x kbits/s. */
  gm_pw_spin_new (container, _("Maximum video _bitrate of"), _("kbits/s"),
                  pw->video_codecs_settings, "maximum-video-tx-bitrate",
                  _("The maximum video bitrate in kbits/s. The video quality and the effective frame rate will be dynamically adjusted to keep the bitrate at the given value."),
                  16.0, 10240.0, 1.0);
}


GtkWidget *
gm_pw_entry_new (GtkWidget *subsection,
                 const gchar *label_txt,
                 boost::shared_ptr<Ekiga::Settings> settings,
                 const std::string & key,
                 const gchar *tooltip)
{
  GtkWidget *entry = NULL;
  GtkWidget *label = NULL;

  int pos = 0;

  GTK_GRID_LAST_ROW (subsection, pos);

  label = gtk_label_new_with_mnemonic (label_txt);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_grid_attach (GTK_GRID (subsection), label, 0, pos-1, 1, 1);

  entry = gtk_entry_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);
  gtk_grid_attach_next_to (GTK_GRID (subsection), entry, label, GTK_POS_RIGHT, 1, 1);

  g_settings_bind (settings->get_g_settings (), key.c_str (),
                   entry, "text", G_SETTINGS_BIND_DEFAULT);

  if (tooltip)
    gtk_widget_set_tooltip_text (entry, tooltip);

  gtk_widget_show_all (subsection);

  return entry;
}

GtkWidget *
gm_pw_string_option_menu_new (GtkWidget *subsection,
                              const gchar *label_txt,
                              const gchar **options,
                              boost::shared_ptr <Ekiga::Settings> settings,
                              const std::string & key,
                              const gchar *tooltip)
{
  GtkWidget *label = NULL;
  GtkWidget *option_menu = NULL;
  GList *cells = NULL;

  int cpt = 0;
  int pos = 0;
  bool int_setting = false;

  GTK_GRID_LAST_ROW (subsection, pos);

  int_setting =
    !(g_variant_type_equal (g_variant_get_type (g_settings_get_value (settings->get_g_settings (),
                                                                      key.c_str ())),
                            G_VARIANT_TYPE_STRING));

  label = gtk_label_new_with_mnemonic (label_txt);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_grid_attach (GTK_GRID (subsection), label, 0, pos-1, 1, 1);

  option_menu = gtk_combo_box_text_new ();
  cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (option_menu));
  g_object_set (G_OBJECT (cells->data), "ellipsize", PANGO_ELLIPSIZE_END, "width-chars", 30, NULL);
  g_list_free (cells);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), option_menu);
  while (options [cpt]) {
    if (int_setting)
      gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (option_menu), options [cpt]);
    else
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (option_menu), options [cpt], options [cpt]);
    cpt++;
  }
  gtk_grid_attach_next_to (GTK_GRID (subsection), option_menu, label, GTK_POS_RIGHT, 1, 1);

  g_settings_bind (settings->get_g_settings (), key.c_str (),
                   option_menu, int_setting ? "active" : "active-id",
                   G_SETTINGS_BIND_DEFAULT);

  if (tooltip)
    gtk_widget_set_tooltip_text (option_menu, tooltip);

  gtk_widget_show_all (subsection);

  return option_menu;
}


void
gm_pw_string_option_menu_update (GtkWidget *option_menu,
                                 const gchar **options,
                                 boost::shared_ptr<Ekiga::Settings> settings,
                                 const std::string & key)
{
  int cpt = 0;
  bool int_setting = false;

  if (!options || key.empty ())
    return;

  int_setting =
    !(g_variant_type_equal (g_variant_get_type (g_settings_get_value (settings->get_g_settings (),
                                                                      key.c_str ())),
                            G_VARIANT_TYPE_STRING));

  gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (option_menu));

  while (options [cpt]) {

    if (int_setting)
      gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (option_menu), options [cpt]);
    else
      gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (option_menu), options [cpt], options [cpt]);
    cpt++;
  }

  // We need to bind again after a remove_all operation
  g_settings_bind (settings->get_g_settings (), key.c_str (),
                   option_menu, int_setting ? "active" : "active-id",
                   G_SETTINGS_BIND_DEFAULT);

  // Force the corresponding AudioInputCore/AudioOutputCore/VideoInputCore
  // to select the most appropriate device if we removed the currently used
  // device
  if (gtk_combo_box_get_active (GTK_COMBO_BOX (option_menu)) == -1)
    settings->set_string (key, ""); // Nothing selected
}


void
gm_pw_string_option_menu_add (GtkWidget *option_menu,
                              const std::string & option,
                              boost::shared_ptr <Ekiga::Settings> settings,
                              const std::string & key)
{
  bool int_setting = false;

  if (option.empty ())
    return;

  int_setting =
    !(g_variant_type_equal (g_variant_get_type (g_settings_get_value (settings->get_g_settings (),
                                                                      key.c_str ())),
                            G_VARIANT_TYPE_STRING));

  if (int_setting)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (option_menu), option.c_str ());
  else
    gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (option_menu), option.c_str (), option.c_str ());
}


void
gm_pw_string_option_menu_remove (GtkWidget *option_menu,
                                 const std::string & option,
                                 boost::shared_ptr <Ekiga::Settings> settings,
                                 const std::string & key)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (option_menu));
  GtkTreeIter iter;
  int pos = 0;
  gchar *s = NULL;

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {
    do {
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &s, -1);
      if (s && !strcmp (s, option.c_str ())) {
        g_free (s);
        break;
      }
      g_free (s);
      pos++;
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }
  gtk_combo_box_text_remove (GTK_COMBO_BOX_TEXT (option_menu), pos);

  // Force the corresponding AudioInputCore/AudioOutputCore/VideoInputCore
  // to select the most appropriate device if we removed the currently used
  // device
  if (gtk_combo_box_get_active (GTK_COMBO_BOX (option_menu)) == -1)
    settings->set_string (key, ""); // Nothing selected
}


GtkWidget *
gm_pw_scale_new (GtkWidget* subsection,
                 const gchar *down_label_txt,
                 const gchar *up_label_txt,
                 boost::shared_ptr <Ekiga::Settings> settings,
                 const std::string & key,
                 const gchar *tooltip,
                 double min,
                 double max,
                 double step)
{
  GtkWidget *hbox = NULL;
  GtkAdjustment *adj = NULL;
  GtkWidget *label = NULL;
  GtkWidget *hscale = NULL;

  int pos = 0;

  GTK_GRID_LAST_ROW (subsection, pos);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  label = gtk_label_new_with_mnemonic (down_label_txt);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);

  adj = (GtkAdjustment *)
    gtk_adjustment_new (settings->get_int (key),
                        min, max, step,
                        2.0, 1.0);
  g_settings_bind (settings->get_g_settings (), key.c_str (),
                   adj, "value", G_SETTINGS_BIND_DEFAULT);

  hscale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adj);
  gtk_scale_set_draw_value (GTK_SCALE (hscale), FALSE);
  gtk_widget_set_size_request (GTK_WIDGET (hscale), 150, -1);
  gtk_box_pack_start (GTK_BOX (hbox), hscale, FALSE, FALSE, 2);

  label = gtk_label_new_with_mnemonic (up_label_txt);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);

  gtk_grid_attach (GTK_GRID (subsection), hbox, 0, pos-1, 2, 1);

  if (tooltip)
    gtk_widget_set_tooltip_text (hscale, tooltip);

  gtk_widget_show_all (subsection);

  return hscale;
}


GtkWidget *
gm_pw_spin_new (GtkWidget* subsection,
                const gchar *label_txt,
                const gchar *label_txt2,
                boost::shared_ptr <Ekiga::Settings> settings,
                const std::string & key,
                const gchar *tooltip,
                double min,
                double max,
                double step)
{
  GtkAdjustment *adj = NULL;
  GtkWidget *label = NULL;
  GtkWidget *spin_button = NULL;
  GtkWidget *hbox = NULL;

  int pos = 0;

  GTK_GRID_LAST_ROW (subsection, pos);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  label = gtk_label_new_with_mnemonic (label_txt);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);

  adj = (GtkAdjustment *)
    gtk_adjustment_new (settings->get_int (key),
                        min, max, step,
                        1.0, 1.0);
  spin_button = gtk_spin_button_new (adj, 1.0, 0);
  g_settings_bind (settings->get_g_settings (), key.c_str (),
                   adj, "value", G_SETTINGS_BIND_DEFAULT);
  gtk_box_pack_start (GTK_BOX (hbox), spin_button, FALSE, FALSE, 2);

  if (label_txt2) {
    label = gtk_label_new_with_mnemonic (label_txt2);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  }

  gtk_grid_attach (GTK_GRID (subsection), hbox, 0, pos-1, 2, 1);

  if (tooltip)
    gtk_widget_set_tooltip_text (spin_button, tooltip);

  gtk_widget_show_all (subsection);

  return spin_button;
}


GtkWidget *
gm_pw_toggle_new (GtkWidget* subsection,
                  const gchar *label_txt,
                  boost::shared_ptr <Ekiga::Settings> settings,
                  const std::string & key,
                  const gchar *tooltip)
{
  GtkWidget *toggle = NULL;

  int pos = 0;

  GTK_GRID_LAST_ROW (subsection, pos);

  toggle = gtk_check_button_new_with_mnemonic (label_txt);
  g_settings_bind (settings->get_g_settings (), key.c_str (),
                   toggle, "active", G_SETTINGS_BIND_DEFAULT);
  gtk_grid_attach (GTK_GRID (subsection), toggle, 0, pos-1, 2, 1);

  if (tooltip)
    gtk_widget_set_tooltip_text (toggle, tooltip);

  gtk_widget_show_all (subsection);

  return toggle;
}

GtkWidget *
gm_pw_window_new (const gchar *logo_name)
{
  GnomePrefsWindow *gpw = NULL;

  GtkTreeSelection *selection = NULL;
  GtkCellRenderer *cell = NULL;
  GtkTreeStore *model = NULL;
  GtkTreeViewColumn *column = NULL;

  GtkWidget *window = NULL;
  GtkWidget *event_box = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *pixmap = NULL;
  GtkWidget *hsep = NULL;

  GdkRGBA cwhite;

  PangoAttrList *attrs = NULL;
  PangoAttribute *attr = NULL;

  /* Box inside the prefs window */
  GtkWidget *dialog_vbox = NULL;

  /* Build the window */
  window = gtk_dialog_new ();

  gpw = (GnomePrefsWindow *) g_malloc (sizeof (GnomePrefsWindow));
  gpw->last_page = 1;

  g_object_set_data_full (G_OBJECT (window), "gpw", (gpointer) gpw, g_free);

  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL);


  /* The sections */
  gpw->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gpw->notebook), FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (gpw->notebook), FALSE);

  pixmap =  gtk_image_new_from_file (logo_name);

  event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (event_box),
                     GTK_WIDGET (pixmap));

  cwhite.red   = 1.0;
  cwhite.green = 1.0;
  cwhite.blue  = 1.0;
  cwhite.alpha = 1.0;
  gtk_widget_override_background_color (GTK_WIDGET (event_box),
                                        GTK_STATE_FLAG_NORMAL, &cwhite);

  gtk_notebook_prepend_page (GTK_NOTEBOOK (gpw->notebook), event_box, NULL);


  /* The sections */
  dialog_vbox = gtk_dialog_get_content_area (GTK_DIALOG (window));

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (dialog_vbox), hbox);


  /* Build the TreeView on the left */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  model = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  gpw->sections_tree_view = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (gpw->sections_tree_view),
                           GTK_TREE_MODEL (model));
  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (gpw->sections_tree_view));
  gtk_container_add (GTK_CONTAINER (frame), gpw->sections_tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gpw->sections_tree_view),
                                     FALSE);
  cell = gtk_cell_renderer_text_new ();

  column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0,
                                                     NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (gpw->sections_tree_view),
                               GTK_TREE_VIEW_COLUMN (column));
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
                               GTK_SELECTION_BROWSE);


  /* Some design stuff to put the notebook pages in it */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);


  gpw->section_label = gtk_label_new (NULL);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_misc_set_alignment (GTK_MISC (gpw->section_label), 0.0, 0.5);
  gtk_container_add (GTK_CONTAINER (frame), gpw->section_label);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  attrs = pango_attr_list_new ();
  attr = pango_attr_scale_new (PANGO_SCALE_LARGE);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (attrs, attr);
  attr = pango_attr_weight_new (PANGO_WEIGHT_HEAVY);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (gpw->section_label), attrs);
  pango_attr_list_unref (attrs);
  gtk_widget_show (gpw->section_label);

  hsep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), gpw->notebook, TRUE, TRUE, 0);

  gtk_widget_show_all (GTK_WIDGET (dialog_vbox));
  gtk_widget_show_all (GTK_WIDGET (gpw->sections_tree_view));

  g_signal_connect (selection, "changed",
                    G_CALLBACK (tree_selection_changed_cb),
                    gpw);


  return window;
}


void
gm_pw_window_section_new (GtkWidget *window,
                          const gchar *section_name)
{
  GnomePrefsWindow *gpw = NULL;
  GtkTreeModel *model = NULL;

  if (!window)
    return;

  if (window)
    gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (window), "gpw");

  if (!gpw || !section_name)
    return;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpw->sections_tree_view));
  gtk_tree_store_append (GTK_TREE_STORE (model), &gpw->iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model), &gpw->iter, 0,
                      section_name, 1, 0, -1);
  gtk_tree_view_expand_all (GTK_TREE_VIEW (gpw->sections_tree_view));
}


GtkWidget *
gm_pw_window_subsection_new (GtkWidget *window,
                             const gchar *section_name)
{
  GnomePrefsWindow *gpw = NULL;
  GtkWidget *container = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter child_iter;

  if (!window)
    return NULL;

  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (window), "gpw");

  if (!gpw || !section_name)
    return NULL;

  container = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID (container), 2);
  gtk_grid_set_row_spacing (GTK_GRID (container), 2);
  gtk_grid_set_column_homogeneous (GTK_GRID (container), FALSE);
  gtk_grid_set_row_homogeneous (GTK_GRID (container), FALSE);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpw->sections_tree_view));
  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &gpw->iter);
  gtk_tree_store_set (GTK_TREE_STORE (model), &child_iter, 0, section_name,
                      1, gpw->last_page, -1);
  gtk_tree_view_expand_all (GTK_TREE_VIEW (gpw->sections_tree_view));

  gpw->last_page++;

  gtk_notebook_append_page (GTK_NOTEBOOK (gpw->notebook),
                            container, NULL);

  gtk_widget_show_all (container);

  return container;
}

void
gm_pw_subsection_new (GtkWidget *container,
                      const gchar *name)
{
  GtkWidget *label = NULL;
  gchar *label_txt = NULL;
  int pos = 0;

  GTK_GRID_LAST_ROW (container, pos);

  label = gtk_label_new (NULL);
  gtk_grid_attach (GTK_GRID (container), label, 0, pos-1, 2, 1);

  label = gtk_label_new (NULL);
  label_txt = g_strdup_printf ("<b>%s</b>", name);
  gtk_label_set_markup (GTK_LABEL (label), label_txt);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_grid_attach (GTK_GRID (container), label, 0, pos, 2, 1);
  g_free (label_txt);
}


/* Callbacks */
static void
tree_selection_changed_cb (GtkTreeSelection *selection,
                           gpointer data)
{
  int page = 0;
  gchar *name = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  GnomePrefsWindow *gpw = NULL;

  if (!data)
    return;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gpw = (GnomePrefsWindow *) data;

    gtk_tree_model_get (GTK_TREE_MODEL (model),
                        &iter, 1, &page, -1);

    gtk_tree_model_get (GTK_TREE_MODEL (model),
                        &iter, 0, &name, -1);

    gtk_label_set_text (GTK_LABEL (gpw->section_label), name);

    gtk_notebook_set_current_page (GTK_NOTEBOOK (gpw->notebook), page);
  }
}

static void
refresh_devices_list_cb (G_GNUC_UNUSED GtkWidget *widget,
                         gpointer data)
{
  g_return_if_fail (data != NULL);
  GtkWidget *prefs_window = GTK_WIDGET (data);

  gm_prefs_window_update_devices_list (prefs_window);
}


static void
sound_event_changed_cb (GtkWidget *b,
                        gpointer data)
{

  GmPreferencesWindow *pw = NULL;

  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;

  gchar *filename = NULL;
  gchar *key = NULL;
  std::string sound_event;

  g_return_if_fail (data != NULL);
  pw = gm_pw_get_pw (GTK_WIDGET (data));

  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        2, &key, -1);

    if (key) {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (b));
      if (filename) {
        sound_event = pw->sound_events_settings->get_string (key);

        if (sound_event.empty () || g_strcmp0 (filename, sound_event.c_str ()))
          pw->sound_events_settings->set_string (key, filename);

        g_free (filename);
      }

      g_free (key);
    }
  }
}


static void
sound_event_setting_changed (G_GNUC_UNUSED GSettings *settings,
                             gchar *key,
                             gpointer data)
{
  bool valid = true;

  GtkTreeIter iter;

  g_return_if_fail (data != NULL);

  GmPreferencesWindow *pw = gm_pw_get_pw (GTK_WIDGET (data));

  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (pw->sound_events_list));

  /* Get the first iter in the list, check it is valid and walk
   *  * through the list, reading each row. */
  for (valid = gtk_tree_model_get_iter_first (model, &iter);
       valid;
       valid = gtk_tree_model_iter_next (model, &iter)) {

    gchar *str = NULL;
    gtk_tree_model_get (model, &iter, 3, &str, -1);

    if (key && str && !g_strcmp0 (key, str)) {
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          0, pw->sound_events_settings->get_bool (key));
      break;
    }

    g_free (str);
  }
}


static void
sound_event_selected_cb (GtkTreeSelection *selection,
                         gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  GmPreferencesWindow *pw = NULL;
  gchar *key = NULL;
  gchar *filename = NULL;

  std::string sound_event;

  g_return_if_fail (data != NULL);
  pw = gm_pw_get_pw (GTK_WIDGET (data));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 2, &key, -1);

    if (key) {

      sound_event = pw->sound_events_settings->get_string (key);

      if (!sound_event.empty ()) {

        if (!g_path_is_absolute (sound_event.c_str ()))
          filename = g_build_filename (DATA_DIR, "sounds", PACKAGE_NAME,
                                       sound_event.c_str (), NULL);
        else
          filename = g_strdup (sound_event.c_str ());

        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (pw->fsbutton), filename);
        g_free (filename);
      }

      g_free (key);
    }
  }
}


static void
sound_event_play_cb (G_GNUC_UNUSED GtkWidget *widget,
                     gpointer data)
{
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter selected_iter;

  gchar *key = NULL;

  std::string sound_event;

  GmPreferencesWindow *pw = NULL;

  g_return_if_fail (data != NULL);
  pw = gm_pw_get_pw (GTK_WIDGET (data));

  g_return_if_fail (data != NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pw->sound_events_list));

  if (gtk_tree_selection_get_selected (selection, &model, &selected_iter)) {

    gtk_tree_model_get (GTK_TREE_MODEL (model), &selected_iter, 2, &key, -1);

    sound_event = pw->sound_events_settings->get_string (key);
    if (!sound_event.empty ()) {
      if (!g_path_is_absolute (sound_event.c_str ()))
        pw->audiooutput_core->play_event (sound_event);
      else
        pw->audiooutput_core->play_file (sound_event);
    }

    g_free (key);
  }
}


static void
sound_event_toggled_cb (G_GNUC_UNUSED GtkCellRendererToggle *cell,
                        gchar *path_str,
                        gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  GmPreferencesWindow *pw = NULL;
  gchar *key = NULL;

  bool fixed = FALSE;

  g_return_if_fail (data != NULL);
  pw = gm_pw_get_pw (GTK_WIDGET (data));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (pw->sound_events_list));
  path = gtk_tree_path_new_from_string (path_str);

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, 0, &fixed, 3, &key, -1);

  fixed ^= 1;

  pw->sound_events_settings->set_bool (key, fixed);

  g_free (key);
  gtk_tree_path_free (path);
}

void on_videoinput_device_added_cb (const Ekiga::VideoInputDevice & device, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gm_pw_string_option_menu_add (pw->video_device, device.GetString(),
                                pw->video_devices_settings, "input-device");
}

void on_videoinput_device_removed_cb (const Ekiga::VideoInputDevice & device, bool, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gm_pw_string_option_menu_remove (pw->video_device, device.GetString(),
                                   pw->video_devices_settings, "input-device");
}

void on_audioinput_device_added_cb (const Ekiga::AudioInputDevice & device, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gm_pw_string_option_menu_add (pw->audio_recorder, device.GetString(),
                                pw->audio_devices_settings, "input-device");
}

void on_audioinput_device_removed_cb (const Ekiga::AudioInputDevice & device, bool, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gm_pw_string_option_menu_remove (pw->audio_recorder, device.GetString(),
                                   pw->audio_devices_settings, "input-device");
}

void on_audiooutput_device_added_cb (const Ekiga::AudioOutputDevice & device, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gm_pw_string_option_menu_add (pw->audio_player, device.GetString(),
                                pw->audio_devices_settings, "output-device");
  gm_pw_string_option_menu_add (pw->sound_events_output, device.GetString(),
                                pw->sound_events_settings, "output-device");
}

void on_audiooutput_device_removed_cb (const Ekiga::AudioOutputDevice & device, bool, GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);
  gm_pw_string_option_menu_remove (pw->audio_player, device.GetString(),
                                   pw->audio_devices_settings, "output-device");
  gm_pw_string_option_menu_remove (pw->sound_events_output, device.GetString(),
                                   pw->sound_events_settings, "output-device");
}


/* Public functions */
void
gm_prefs_window_update_devices_list (GtkWidget *prefs_window)
{
  GmPreferencesWindow *pw = NULL;
  gchar **array = NULL;

  g_return_if_fail (prefs_window != NULL);
  pw = gm_pw_get_pw (prefs_window);

  std::vector <std::string> device_list;

  /* The player */
  get_audiooutput_devices (pw->audiooutput_core, device_list);
  array = vector_of_string_to_array (device_list);
  gm_pw_string_option_menu_update (pw->audio_player,
                                   (const gchar **) array,
                                   pw->audio_devices_settings,
                                   "output-device");
  gm_pw_string_option_menu_update (pw->sound_events_output,
                                   (const gchar **) array,
                                   pw->sound_events_settings,
                                   "output-device");
  g_free (array);

  /* The recorder */
  get_audioinput_devices (pw->audioinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  gm_pw_string_option_menu_update (pw->audio_recorder,
                                   (const gchar **) array,
                                   pw->audio_devices_settings,
                                   "input-device");
  g_free (array);


  /* The Video player */
  get_videoinput_devices (pw->videoinput_core, device_list);
  array = vector_of_string_to_array (device_list);
  gm_pw_string_option_menu_update (pw->video_device,
                                   (const gchar **) array,
                                   pw->video_devices_settings,
                                   "input-device");
  g_free (array);
}


GtkWidget *
preferences_window_new (Ekiga::ServiceCore& core)
{
  GmPreferencesWindow *pw = NULL;

  GdkPixbuf *pixbuf = NULL;
  GtkWidget *window = NULL;
  GtkWidget *container = NULL;
  gchar     *filename = NULL;
  std::vector <std::string> device_list;

  filename = g_build_filename (DATA_DIR, "pixmaps", PACKAGE_NAME, PACKAGE_NAME "-logo.png", NULL);
  window = gm_pw_window_new (filename);
  g_free (filename);
  g_object_set_data_full (G_OBJECT (window), "window_name",
                          g_strdup ("preferences_window"), g_free);
  gtk_window_set_title (GTK_WINDOW (window), _("Ekiga Preferences"));
  pixbuf = gtk_widget_render_icon_pixbuf (GTK_WIDGET (window),
                                          GTK_STOCK_PREFERENCES,
                                          GTK_ICON_SIZE_MENU);
  gtk_window_set_icon (GTK_WINDOW (window), pixbuf);
  gtk_widget_realize (GTK_WIDGET (window));
  g_object_unref (pixbuf);


  /* The GMObject data */
  pw = new GmPreferencesWindow;

  pw->audioinput_core = core.get<Ekiga::AudioInputCore> ("audioinput-core");
  pw->audiooutput_core = core.get<Ekiga::AudioOutputCore> ("audiooutput-core");
  pw->videoinput_core = core.get<Ekiga::VideoInputCore> ("videoinput-core");

  g_object_set_data_full (G_OBJECT (window), "GMObject",
                          pw, (GDestroyNotify) gm_pw_destroy);


  gm_pw_window_section_new (window, _("General"));
  container = gm_pw_window_subsection_new (window, _("Personal Data"));
  gm_pw_init_general_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gm_pw_window_subsection_new (window,
                                           _("General Settings"));
  gm_pw_init_interface_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gm_pw_window_subsection_new (window, _("Call Options"));
  gm_pw_init_call_options_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gm_pw_window_subsection_new (window,
                                           _("Sound Events"));
  gm_pw_init_sound_events_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  gm_pw_window_section_new (window, _("Protocols"));
  container = gm_pw_window_subsection_new (window,
                                           _("SIP Settings"));
  gm_pw_init_sip_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gm_pw_window_subsection_new (window,
                                           _("H.323 Settings"));
  gm_pw_init_h323_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));


  /* The player */
  gm_pw_window_section_new (window, _("Audio"));
  container = gm_pw_window_subsection_new (window, _("Devices"));
  gm_pw_init_audio_devices_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gm_pw_window_subsection_new (window, _("Codecs"));
  gm_pw_init_audio_codecs_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));


  gm_pw_window_section_new (window, _("Video"));
  container = gm_pw_window_subsection_new (window, _("Devices"));
  gm_pw_init_video_devices_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));

  container = gm_pw_window_subsection_new (window, _("Codecs"));
  gm_pw_init_video_codecs_page (window, container);
  gtk_widget_show_all (GTK_WIDGET (container));


  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (window, "response",
                            G_CALLBACK (gtk_widget_hide),
                            (gpointer) window);

  gtk_widget_hide_on_delete (window);

  boost::signals2::connection conn;

  conn = pw->videoinput_core->device_added.connect (boost::bind (&on_videoinput_device_added_cb, _1, window));
  pw->connections.add (conn);
  conn = pw->videoinput_core->device_removed.connect (boost::bind (&on_videoinput_device_removed_cb, _1, _2, window));
  pw->connections.add (conn);

  conn = pw->audioinput_core->device_added.connect (boost::bind (&on_audioinput_device_added_cb, _1, window));
  pw->connections.add (conn);
  conn = pw->audioinput_core->device_removed.connect (boost::bind (&on_audioinput_device_removed_cb, _1, _2, window));
  pw->connections.add (conn);

  conn = pw->audiooutput_core->device_added.connect (boost::bind (&on_audiooutput_device_added_cb, _1, window));
  pw->connections.add(conn);
  conn = pw->audiooutput_core->device_removed.connect (boost::bind (&on_audiooutput_device_removed_cb, _1, _2, window));
  pw->connections.add (conn);

  /* Connect notifiers for SOUND_EVENTS_SCHEMA settings */
  g_signal_connect (pw->sound_events_settings->get_g_settings (),
                    "changed::enable-incoming-call-sound",
                    G_CALLBACK (sound_event_setting_changed), window);

  g_signal_connect (pw->sound_events_settings->get_g_settings (),
                    "changed::enable-ring-tone-sound",
                    G_CALLBACK (sound_event_setting_changed), window);

  g_signal_connect (pw->sound_events_settings->get_g_settings (),
                    "changed::enable-busy-tone-sound",
                    G_CALLBACK (sound_event_setting_changed), window);

  g_signal_connect (pw->sound_events_settings->get_g_settings (),
                    "changed::enable-new-voicemail-sound",
                    G_CALLBACK (sound_event_setting_changed), window);

  g_signal_connect (pw->sound_events_settings->get_g_settings (),
                    "changed::enable-new-message-sound",
                    G_CALLBACK (sound_event_setting_changed), window);

  return window;
}
