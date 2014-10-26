project(kruler)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

add_definitions(-DTRANSLATION_DOMAIN="kruler")

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF5 REQUIRED COMPONENTS
    DocTools
    I18n
    Notifications
    WindowSystem
    XmlGui
)

#add_subdirectory( pics )
add_subdirectory( doc )

########### next target ###############

set(kruler_SRCS qautosizelabel.cpp klineal.cpp main.cpp krulersystemtray.cpp)

ki18n_wrap_ui(kruler_SRCS cfg_appearance.ui cfg_advanced.ui)

kconfig_add_kcfg_files(kruler_SRCS kruler.kcfgc)

#kde4_add_app_icon(kruler_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-apps-kruler.png")

add_executable(kruler ${kruler_SRCS})

target_link_libraries(kruler KF5::I18n KF5::Notifications KF5::WindowSystem KF5::XmlGui)

install(TARGETS kruler ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS kruler.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kruler.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/kruler )
install( FILES move.wav  DESTINATION  ${DATA_INSTALL_DIR}/kruler/sounds )
