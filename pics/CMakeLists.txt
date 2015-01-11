########### install files ###############

#install( FILES kruler-north.png kruler-south.png kruler-west.png DESTINATION ${DATA_INSTALL_DIR}/kruler/pics )

ecm_install_icons(ICONS 
hi128-apps-kruler.png         hi16-actions-kruler-north.png  hi16-actions-kruler-west.png  hi22-actions-kruler-east.png   hi22-actions-kruler-south.png  hi22-apps-kruler.png  hi48-apps-kruler.png
hi16-actions-kruler-east.png  hi16-actions-kruler-south.png  hi16-apps-kruler.png          hi22-actions-kruler-north.png  hi22-actions-kruler-west.png   hi32-apps-kruler.png  hi64-apps-kruler.png
DESTINATION ${ICON_INSTALL_DIR} )
