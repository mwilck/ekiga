## Process this file with automake to produce Makefile.in.

if HAVE_GNOME_DOC_UTILS
include $(top_srcdir)/gnome-doc-utils.make
endif

dist-hook: doc-dist-hook

# Keeping PACKAGE_NAME to be able to install stable and snapshot in parallel
DOC_MODULE = @PACKAGE_NAME@
DOC_ENTITIES =
DOC_INCLUDES =
DOC_FIGURES = figures/lumi.png \
              figures/call_window.png \
              figures/config_d1.png \
              figures/config_d2.png \
              figures/config_d3.png \
              figures/config_d4.png \
              figures/config_d5.png \
              figures/config_d6.png \
              figures/config_d7.png \
              figures/config_d8.png \
              figures/config_d9.png \
              figures/call_d1.png \
              figures/roster.png \
              figures/addressbook_d1.png \
              figures/addressbook_d2.png \
              figures/addressbook_d3.png \
              figures/chat_d1.png \
              figures/status.png \
              figures/call_history.png \
              figures/accounts_d1.png \
              figures/accounts_ekiga_net.png \
              figures/accounts_ekiga_call_out.png \
              figures/accounts_sip.png \
              figures/accounts_h323.png \
              figures/monitoring_lines.png \
              figures/audio_codecs.png \
              figures/video_codecs.png

DOC_LINGUAS = bg ca cs de el en_GB es eu fi fr oc pt_BR ru sl sv uk zh_CN
