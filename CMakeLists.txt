project(kruler)

add_subdirectory( pics )

########### next target ###############

set(kruler_SRCS qautosizelabel.cpp klineal.cpp main.cpp krulersystemtray.cpp)

kde4_add_ui_files(kruler_SRCS cfg_appearance.ui cfg_advanced.ui)

kde4_add_kcfg_files(kruler_SRCS kruler.kcfgc)

kde4_add_app_icon(kruler_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-apps-kruler.png")

kde4_add_executable(kruler ${kruler_SRCS})

target_link_libraries(kruler ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})

install(TARGETS kruler ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES kruler.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kruler.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/kruler )
install( FILES move.wav  DESTINATION  ${DATA_INSTALL_DIR}/kruler/sounds )
