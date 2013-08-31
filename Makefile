PREFIX=/usr

CONF_PREFIX=$(PREFIX)

DATA_PATH=$(PREFIX)/share/qipmsg
ICON_PATH=$(PREFIX)/share/qipmsg/icons
DOC_PATH=$(PREFIX)/share/doc/qipmsg
TRANSLATION_PATH=$(PREFIX)/share/qipmsg/translations
SOUND_PATH=$(PREFIX)/share/qipmsg/sounds

APP_PIXMAPS=$(PREFIX)/share/pixmaps
APP_ICONS=$(PREFIX)/share/icons/hicolor
APP_LINK=$(PREFIX)/share/applications

QMAKE=qmake-qt4
LRELEASE=lrelease-qt4

DEFS=DATA_PATH=\\\"$(DATA_PATH)\\\" \
	 TRANSLATION_PATH=\\\"$(TRANSLATION_PATH)\\\" \
	 DOC_PATH=\\\"$(DOC_PATH)\\\" \
	 SOUND_PATH=\\\"$(SOUND_PATH)\\\" \
	 ICON_PATH=\\\"$(ICON_PATH)\\\"

src/qipmsg:
	cd src && $(QMAKE) && $(DEFS) make
	cd src && $(LRELEASE) qipmsg.pro

clean:
	cd src && make clean
	-rm src/qipmsg
	-rm src/translations/qipmsg_*.qm
	-rm src/Makefile

install: src/qipmsg
	-install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 src/qipmsg $(DESTDIR)$(PREFIX)/bin/
	-install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 src/qipmsg-xdg-open $(DESTDIR)$(PREFIX)/bin/
	-install -d $(DESTDIR)$(TRANSLATION_PATH)
	install -m 644 src/translations/*.qm $(DESTDIR)$(TRANSLATION_PATH)
	-install -d $(DESTDIR)$(SOUND_PATH)
	install -m 644 src/sounds/*.wav $(DESTDIR)$(SOUND_PATH)
	-install -d $(DESTDIR)$(ICON_PATH)
	install -m 644 src/icons/*.xpm $(DESTDIR)$(ICON_PATH)
	-install -d $(DESTDIR)$(DOC_PATH)
	install -m 644 ChangeLog *.txt $(DESTDIR)$(DOC_PATH)
	-install -d $(DESTDIR)$(APP_PIXMAPS)/
	-install -d $(DESTDIR)$(APP_ICONS)/64x64/apps/
	-install -d $(DESTDIR)$(APP_ICONS)/32x32/apps/
	-install -d $(DESTDIR)$(APP_ICONS)/22x22/apps/
	-install -d $(DESTDIR)$(APP_ICONS)/16x16/apps/
	install -m 644 icons/qipmsg.png $(DESTDIR)$(APP_PIXMAPS)/qipmsg.png
	install -m 644 icons/qipmsg64.png $(DESTDIR)$(APP_ICONS)/64x64/apps/qipmsg.png
	install -m 644 icons/qipmsg32.png $(DESTDIR)$(APP_ICONS)/32x32/apps/qipmsg.png
	install -m 644 icons/qipmsg22.png $(DESTDIR)$(APP_ICONS)/22x22/apps/qipmsg.png
	install -m 644 icons/qipmsg16.png $(DESTDIR)$(APP_ICONS)/16x16/apps/qipmsg.png
	-install -d $(DESTDIR)$(APP_LINK)
	install -m 644 qipmsg.desktop $(DESTDIR)$(APP_LINK)

uninstall:
	-rm $(PREFIX)/bin/qipmsg
	-rm $(PREFIX)/bin/qipmsg-xdg-open
	-rm $(TRANSLATION_PATH)/*.qm
	-rm $(SOUND_PATH)/*.wav
	-rm $(ICON_PATH)/*.xpm
	-rm $(DOC_PATH)/ChangeLog
	-rm $(DOC_PATH)/*.txt
	-rm $(APP_PIXMAPS)/qipmsg.png
	-rm $(APP_ICONS)/64x64/apps/qipmsg.png
	-rm $(APP_ICONS)/32x32/apps/qipmsg.png
	-rm $(APP_ICONS)/22x22/apps/qipmsg.png
	-rm $(APP_ICONS)/16x16/apps/qipmsg.png
	-rm $(APP_LINK)/qipmsg.desktop
	-rmdir $(SOUND_PATH)/
	-rmdir $(ICON_PATH)/
	-rmdir $(TRANSLATION_PATH)/
	-rmdir $(DATA_PATH)/
	-rmdir $(DOC_PATH)/

