SUBDIRS = pics

INCLUDES= $(all_includes)

bin_PROGRAMS = kruler
kruler_METASOURCES = AUTO
kruler_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kruler_LDADD   = $(LIB_KDEUI)
kruler_SOURCES = klineal.cpp main.cpp 

xdg_apps_DATA = kruler.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Graphics
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Graphics/kruler.desktop

appdir = $(kde_datadir)/kruler
app_DATA = eventsrc

sounddir = $(kde_datadir)/kruler/sounds
sound_DATA = move.wav

messages:
	$(XGETTEXT) $(kruler_SOURCES) -o $(podir)/kruler.pot

