



########### install files ###############

install( FILES kruler-east.png kruler-north.png kruler-south.png kruler-west.png  DESTINATION  ${DATA_INSTALL_DIR}/kruler/pics )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#
#pixdir = $(kde_datadir)/kruler/pics
#pix_DATA = kruler-east.png kruler-north.png kruler-south.png kruler-west.png
#
#KDE_ICON = kruler
