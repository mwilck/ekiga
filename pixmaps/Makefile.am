EXTRA_DIST = 

pixmapdir = $(datadir)/pixmaps

pixmap_DATA = gnomemeeting-logo-icon.png gnomemeeting-logo.png gnomemeeting-splash.png Lumi.png

IMAGE_FILES = \
	ils-directory.png \
	text-chat.png \
	control-panel.png \
	connect.png \
	disconnect.png \
	volume.png \
	microphone.png \
	video-preview.png \
	audio-mute.png \
	speaker-phone.png \
	available.png \
	occupied.png


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
	gm_quickcam_stock_data quickcam.xpm \
	gm_occupied_stock_data occupied.png \
	gm_available_stock_data available.png

noinst_DATA = inlines.h

inlines.h: $(IMAGE_FILES) Makefile.am
	gdk-pixbuf-csource --raw --build-list $(IMAGES) > inlines.h
