project(kruler)

add_subdirectory( pics ) 




########### next target ###############

set(kruler_SRCS klineal.cpp main.cpp )

kde4_automoc(${kruler_SRCS})

kde4_add_executable(kruler ${kruler_SRCS})

target_link_libraries(kruler  ${KDE4_KDECORE_LIBS} kdeui )

install(TARGETS kruler  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kruler.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES eventsrc  DESTINATION  ${DATA_INSTALL_DIR}/kruler )
install( FILES move.wav  DESTINATION  ${DATA_INSTALL_DIR}/kruler/sounds )

#install-data-local: uninstall.desktop
#   $(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Graphics
#   $(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Graphics/kruler.desktop
#




#original Makefile.am contents follow:

#SUBDIRS = pics
#
#INCLUDES= $(all_includes)
#
#bin_PROGRAMS = kruler
#kruler_METASOURCES = AUTO
#kruler_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kruler_LDADD   = $(LIB_KDEUI)
#kruler_SOURCES = klineal.cpp main.cpp 
#
#xdg_apps_DATA = kruler.desktop
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Graphics
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Graphics/kruler.desktop
#
#appdir = $(kde_datadir)/kruler
#app_DATA = eventsrc
#
#sounddir = $(kde_datadir)/kruler/sounds
#sound_DATA = move.wav
#
#messages:
#	$(XGETTEXT) $(kruler_SOURCES) -o $(podir)/kruler.pot
#
