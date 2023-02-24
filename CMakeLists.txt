cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "22")
set(RELEASE_SERVICE_VERSION_MINOR "12")
set(RELEASE_SERVICE_VERSION_MICRO "3")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kruler VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.90.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMSetupVersion)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Notifications
    WindowSystem
    XmlGui
)
find_package(KF5DocTools ${KF5_MIN_VERSION})
set_package_properties(KF5DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)
if (NOT APPLE)
    option(WITHOUT_X11 "Build without X11 integration (skips finding X11)" OFF)
    if (NOT WITHOUT_X11)
        find_package(XCB COMPONENTS XCB)
        set (KRULER_HAVE_X11 ${XCB_FOUND})
        if (XCB_FOUND)
            if (QT_MAJOR_VERSION STREQUAL "5")
                find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED X11Extras)
            else()
                # qtx11extras_p.h is in Qt6Gui, which implied by the Qt6Gui above
            endif()
        endif()
    endif()
endif()

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055800
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_definitions(-DTRANSLATION_DOMAIN="kruler")

configure_file(krulerconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/krulerconfig.h)

add_subdirectory( pics )

if (KF5DocTools_FOUND)
    add_subdirectory(doc)
endif()

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
    KF5::CoreAddons
    KF5::I18n
    KF5::Notifications
    KF5::WindowSystem
    KF5::XmlGui
)
if (KRULER_HAVE_X11)
    target_link_libraries(kruler
        ${XCB_LIBRARIES}
    )
    if (TARGET Qt5::X11Extras)
        target_link_libraries(kruler Qt5::X11Extras)
    elseif (TARGET Qt6::Gui)
        target_link_libraries(kruler Qt6::GuiPrivate) # qtx11extras_p.h
    endif()
endif()

install(TARGETS kruler ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install( PROGRAMS org.kde.kruler.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES org.kde.kruler.appdata.xml    DESTINATION  ${KDE_INSTALL_METAINFODIR} )
install( FILES kruler.notifyrc  DESTINATION  ${KDE_INSTALL_KNOTIFYRCDIR} )
install( FILES move.wav  DESTINATION  ${KDE_INSTALL_DATADIR}/kruler/sounds )

ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

