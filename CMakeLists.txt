cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "21")
set(RELEASE_SERVICE_VERSION_MINOR "12")
set(RELEASE_SERVICE_VERSION_MICRO "2")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kruler VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.85.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMSetupVersion)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
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

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055500
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_definitions(-DTRANSLATION_DOMAIN="kruler")

configure_file(krulerconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/krulerconfig.h)

add_subdirectory( pics )
add_subdirectory( doc )

ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX KRULER
    VERSION_HEADER kruler_version.h
)

add_executable(kruler)
target_sources(kruler PRIVATE
    klineal.cpp
    krulersystemtray.cpp
    klineal.h
    krulersystemtray.h
    main.cpp
)

ki18n_wrap_ui(kruler
    cfg_appearance.ui
    cfg_advanced.ui
)

kconfig_add_kcfg_files(kruler kruler.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/*-apps-kruler.png")
ecm_add_app_icon(kruler ICONS ${ICONS_SRCS})

target_link_libraries(kruler
    KF5::I18n
    KF5::Notifications
    KF5::WindowSystem
    KF5::XmlGui
)
if (X11_FOUND)
    target_link_libraries(kruler
        Qt::X11Extras
        ${XCB_LIBRARIES}
    )
endif()

install(TARGETS kruler ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install( PROGRAMS org.kde.kruler.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES org.kde.kruler.appdata.xml    DESTINATION  ${KDE_INSTALL_METAINFODIR} )
install( FILES kruler.notifyrc  DESTINATION  ${KDE_INSTALL_KNOTIFY5RCDIR} )
install( FILES move.wav  DESTINATION  ${KDE_INSTALL_DATADIR}/kruler/sounds )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

