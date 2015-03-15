########### install files ###############

#install( FILES kruler-north.png kruler-south.png kruler-west.png DESTINATION ${KDE_INSTALL_DATADIR}/kruler/pics )

ecm_install_icons(ICONS 
128-apps-kruler.png         16-actions-kruler-north.png  16-actions-kruler-west.png  22-actions-kruler-east.png   22-actions-kruler-south.png  22-apps-kruler.png  48-apps-kruler.png
16-actions-kruler-east.png  16-actions-kruler-south.png  16-apps-kruler.png          22-actions-kruler-north.png  22-actions-kruler-west.png   32-apps-kruler.png  64-apps-kruler.png
DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor )
