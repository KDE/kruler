SUBDIRS = pics

INCLUDES= $(all_includes)

bin_PROGRAMS = kruler
kruler_METASOURCES = AUTO
kruler_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kruler_LDADD   = $(LIB_KDEUI)
kruler_SOURCES = klineal.cpp main.cpp 

lnkdir = $(kde_appsdir)/Graphics
lnk_DATA = kruler.desktop

appdir = $(kde_datadir)/kruler
app_DATA = eventsrc move.wav

messages:
	$(XGETTEXT) $(kruler_SOURCES) -o $(podir)/kruler.pot

