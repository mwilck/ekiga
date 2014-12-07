icondir = $(datadir)/icons/gnome
pixmapdir = $(datadir)/pixmaps/@PACKAGE_NAME@

pixmap_DATA = @PACKAGE_NAME@-logo.png 256x256/@PACKAGE_NAME@-full-icon.png

theme_icons = \
	gnome_actions_scalable_call-hold-symbolic.svg \
	gnome_actions_scalable_call-transfer-symbolic.svg \
	gnome_actions_scalable_call-transfer-symbolic-rtl.svg \
        gnome_actions_16x16_phone-hang-up.png \
        gnome_actions_16x16_phone-pick-up.png \
        gnome_actions_24x24_phone-hang-up.png \
        gnome_actions_24x24_phone-pick-up.png

nobase_dist_icon_DATA = \
	16x16/apps/@PACKAGE_NAME@.png \
	22x22/apps/@PACKAGE_NAME@.png \
	24x24/apps/@PACKAGE_NAME@.png \
	32x32/apps/@PACKAGE_NAME@.png \
	48x48/apps/@PACKAGE_NAME@.png \
	64x64/apps/@PACKAGE_NAME@.png \
	72x72/apps/@PACKAGE_NAME@.png \
	128x128/apps/@PACKAGE_NAME@.png


DIST_IMAGES_FILES= \
	scalable/call-hold-symbolic.svg \
	scalable/call-transfer-symbolic.svg \
	scalable/call-transfer-symbolic-rtl.svg \
	16x16/phone-hang-up.png \
	16x16/phone-pick-up.png \
	24x24/phone-hang-up.png \
	24x24/phone-pick-up.png

IMAGES_FILES= \
	48x48/multimedia-headset.png \
	22x22/multimedia-headset.png \
	22x22/multimedia-headset.svg \
	128x128/multimedia-headset.png \
	128x128/multimedia-headset.svg \
	16x16/multimedia-headset.png \
	16x16/multimedia-headset.svg \
	scalable/multimedia-headset.svg \
	32x32/multimedia-headset.png \
	32x32/multimedia-headset.svg \
	256x256/multimedia-headset.png \
	256x256/ekiga-full-icon.png \
	16x16/audio-volume.png \
	16x16/brightness.png \
	16x16/color.png \
	16x16/contrast.png \
	16x16/im-message.png \
	16x16/im-message-new.png \
	16x16/user-inacall.png \
	16x16/video-settings.png \
	16x16/whiteness.png \
	ekiga-logo.png \
	scalable/call-hold-symbolic.svg \
	scalable/call-transfer-symbolic.svg \
	gm_powermeter_default_00.xpm \
	gm_powermeter_default_01.xpm \
	gm_powermeter_default_02.xpm \
	gm_powermeter_default_03.xpm \
	gm_powermeter_default_04.xpm \
	icon.h


IMAGES= \
	gm_audio_volume_16 $(srcdir)/16x16/audio-volume.png \
	gm_brightness_16 $(srcdir)/16x16/brightness.png \
	gm_color_16 $(srcdir)/16x16/color.png \
	gm_contrast_16 $(srcdir)/16x16/contrast.png \
	gm_im_message_16 $(srcdir)/16x16/im-message.png \
	gm_im_message_new_16 $(srcdir)/16x16/im-message-new.png \
	gm_video_settings_16 $(srcdir)/16x16/video-settings.png \
	gm_whiteness_16 $(srcdir)/16x16/whiteness.png \
	gm_logo_16_stock_data $(srcdir)/16x16/apps/@PACKAGE_NAME@.png \
	gm_logo_22_stock_data $(srcdir)/22x22/apps/@PACKAGE_NAME@.png \
	gm_logo_24_stock_data $(srcdir)/24x24/apps/@PACKAGE_NAME@.png \
	gm_logo_32_stock_data $(srcdir)/32x32/apps/@PACKAGE_NAME@.png \
	gm_logo_48_stock_data $(srcdir)/48x48/apps/@PACKAGE_NAME@.png \
	gm_logo_72_stock_data $(srcdir)/72x72/apps/@PACKAGE_NAME@.png \
	gm_user_inacall_16 $(srcdir)/16x16/user-inacall.png


noinst_DATA = inlines.h

DISTCLEANFILES = inlines.h

EXTRA_DIST= $(DIST_IMAGES_FILES) $(IMAGES_FILES) $(theme_icons)

inlines.h: $(IMAGE_FILES) $(nobase_dist_icon_DATA) Makefile.am
	gdk-pixbuf-csource --raw --build-list $(IMAGES) > inlines.h

install-data-hook: install-icons update-icon-cache

uninstall-hook: uninstall-icons update-icon-cache

install-icons:
	for icon in $(theme_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$SIZE/$$icon $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done;

uninstall-icons:
	for icon in $(theme_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
		rm -f $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done;

gtk_update_icon_cache = gtk-update-icon-cache-3.0 -f -t $(DESTDIR)$(datadir)/icons/gnome/

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

