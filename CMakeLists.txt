cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "90")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kruler VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.91.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMInstallIcons)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMDeprecationSettings)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF${KF_MAJOR_VERSION} ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Notifications
    WindowSystem
    XmlGui
)
find_package(KF${KF_MAJOR_VERSION}DocTools ${KF5_MIN_VERSION})
set_package_properties(KF${KF_MAJOR_VERSION}DocTools PROPERTIES DESCRIPTION
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

ecm_set_disabled_deprecation_versions(QT 6.4
     KF 5.101.0
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
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Notifications
    KF${KF_MAJOR_VERSION}::WindowSystem
    KF${KF_MAJOR_VERSION}::XmlGui
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

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
