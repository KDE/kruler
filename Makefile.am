INCLUDES= $(all_includes)

bin_PROGRAMS = kruler
kruler_METASOURCES = AUTO
kruler_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kruler_LDADD   = $(LIB_KDEUI)
kruler_SOURCES = klineal.cpp main.cpp 

DISTCLEANFILES = $(kruler_METASOURCES)

KDE_ICON = kruler

pixdir = $(kde_datadir)/kruler
pix_DATA = kruler-east.png kruler-north.png kruler-south.png kruler-west.png

lnkdir = $(kde_appsdir)/Graphics
lnk_DATA = kruler.desktop

messages:
	$(XGETTEXT) -C -ki18n -x $(kde_includes)/kde.pot $(kruler_SOURCES) -o $(podir)/kruler.pot
