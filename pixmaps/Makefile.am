EXTRA_DIST = 

pixmapdir = $(datadir)/pixmaps

pixmap_DATA = gnomemeeting-logo-icon.png gnomemeeting-logo.png gnomemeeting-splash.png Lumi.png


EMOTICONS_FILES = \
	emoticon-face1.png \
	emoticon-face2.png \
	emoticon-face3.png \
	emoticon-face4.png \
	emoticon-face5.png \
	emoticon-face6.png \
	emoticon-face7.png \
	emoticon-face8.png \
	emoticon-face9.png \
	emoticon-face10.png \
	emoticon-face11.png 

IMAGES= \
	gm_ils_directory_stock_data ils-directory.png \
	gm_text_chat_stock_data text-chat.png \
	gm_control_panel_stock_data control-panel.png \
	gm_connect_stock_data connect.png \
	gm_disconnect_stock_data disconnect.png \
	gm_volume_stock_data volume.png \
	gm_microphone_stock_data microphone.png \
	gm_video_preview_stock_data video-preview.png \
	gm_audio_mute_stock_data audio-mute.png \
	gm_speaker_phone_stock_data speaker-phone.png \
	gm_video_mute_stock_data video-mute.png \
	gm_status_available_stock_data status-available-16.png \
	gm_status_ringing_stock_data status-ringing-16.png \
	gm_status_occupied_stock_data status-occupied-16.png 

EMOTICONS= \
	gm_emoticon_face1 emoticon-face1.png \
	gm_emoticon_face2 emoticon-face2.png \
	gm_emoticon_face3 emoticon-face3.png \
	gm_emoticon_face4 emoticon-face4.png \
	gm_emoticon_face5 emoticon-face5.png \
	gm_emoticon_face6 emoticon-face6.png \
	gm_emoticon_face7 emoticon-face7.png \
	gm_emoticon_face8 emoticon-face8.png \
	gm_emoticon_face9 emoticon-face9.png \
	gm_emoticon_face10 emoticon-face10.png \
	gm_emoticon_face11 emoticon-face11.png 

noinst_DATA = inlines.h inline_emoticons.h

inlines.h: $(IMAGE_FILES) Makefile.am
	gdk-pixbuf-csource --raw --build-list $(IMAGES) > inlines.h

inline_emoticons.h: $(EMOTICONS_FILES) Makefile.am
	gdk-pixbuf-csource --raw --build-list $(EMOTICONS) > inline_emoticons.h