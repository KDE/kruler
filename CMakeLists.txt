project(kruler)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.7.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

add_definitions(-DTRANSLATION_DOMAIN="kruler")

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(FeatureSummary)
include(ECMAddAppIcon)

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

if (NOT APPLE)
    find_package(X11)
    set (KRULER_HAVE_X11 ${X11_FOUND})
    if (X11_FOUND)
        find_package(XCB COMPONENTS XCB)
        find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED X11Extras)
    endif()
endif()

configure_file(krulerconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/krulerconfig.h)

add_subdirectory( pics )
add_subdirectory( doc )

########### next target ###############

set(kruler_SRCS qautosizelabel.cpp klineal.cpp main.cpp krulersystemtray.cpp)

ki18n_wrap_ui(kruler_SRCS cfg_appearance.ui cfg_advanced.ui)

kconfig_add_kcfg_files(kruler_SRCS kruler.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/*-apps-kruler.png")
ecm_add_app_icon(kruler_SRCS ICONS ${ICONS_SRCS})

add_executable(kruler ${kruler_SRCS})

target_link_libraries(kruler KF5::I18n KF5::Notifications KF5::WindowSystem KF5::XmlGui)
if (X11_FOUND)
    target_link_libraries(kruler Qt5::X11Extras ${XCB_LIBRARIES})
endif()

install(TARGETS kruler ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS org.kde.kruler.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES kruler.notifyrc  DESTINATION  ${KDE_INSTALL_KNOTIFY5RCDIR} )
install( FILES move.wav  DESTINATION  ${KDE_INSTALL_DATADIR}/kruler/sounds )
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
