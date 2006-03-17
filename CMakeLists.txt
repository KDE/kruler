kde4_header()
project(kruler)

add_subdirectory( pics ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kruler_SRCS klineal.cpp main.cpp )

kde4_automoc(${kruler_SRCS})

kde4_add_executable(kruler ${kruler_SRCS})

target_link_libraries(kruler  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(/bin kruler )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kruler.desktop )
install_files( ${DATA_INSTALL_DIR}/kruler FILES eventsrc )
install_files( ${DATA_INSTALL_DIR}/kruler/sounds FILES move.wav )

kde4_footer()



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
