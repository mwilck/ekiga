icondir = $(datadir)/icons/gnome
pixmapdir = $(datadir)/pixmaps/@PACKAGE_NAME@

more_icons = @PACKAGE_NAME@-logo.png 256x256/@PACKAGE_NAME@-full-icon.png

theme_icons = \
	gnome_actions_scalable_call-hold-symbolic.svg           \
	gnome_actions_scalable_call-transfer-symbolic.svg       \
	gnome_actions_scalable_call-transfer-symbolic-rtl.svg   \
        gnome_actions_16x16_phone-hang-up.png                   \
        gnome_actions_16x16_phone-pick-up.png                   \
        gnome_actions_24x24_phone-hang-up.png                   \
        gnome_actions_24x24_phone-pick-up.png                   \
        gnome_status_16x16-im-message.png                       \
        gnome_status_16x16_user-inacall.png                     \
        hicolor_apps_16x16_ekiga.png                            \
        hicolor_apps_22x22_ekiga.png                            \
        hicolor_apps_24x24_ekiga.png                            \
        hicolor_apps_32x32_ekiga.png                            \
        hicolor_apps_48x48_ekiga.png                            \
        hicolor_apps_64x64_ekiga.png                            \
        hicolor_apps_72x72_ekiga.png                            \
        hicolor_apps_128x128_ekiga.png

EXTRA_DIST= $(more_icons) $(theme_icons)

install-data-hook: install-icons update-icon-cache

uninstall-hook: uninstall-icons update-icon-cache

install-icons:
	for icon in $(theme_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
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
