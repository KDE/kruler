kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kruler/pics FILES kruler-east.png kruler-north.png kruler-south.png kruler-west.png )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#
#pixdir = $(kde_datadir)/kruler/pics
#pix_DATA = kruler-east.png kruler-north.png kruler-south.png kruler-west.png
#
#KDE_ICON = kruler
