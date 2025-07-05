cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "25")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "80")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kruler VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.1.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
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

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

if (Qt6Gui_VERSION VERSION_GREATER_EQUAL "6.10.0")
    find_package(Qt6GuiPrivate ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
endif()

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Notifications
    StatusNotifierItem
    WindowSystem
    XmlGui
    Crash
)


find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)
if (NOT APPLE)
    option(WITHOUT_X11 "Build without X11 integration (skips finding X11)" OFF)
    if (NOT WITHOUT_X11)
        find_package(XCB COMPONENTS XCB)
        set (KRULER_HAVE_X11 ${XCB_FOUND})
    endif()
endif()

ecm_set_disabled_deprecation_versions(QT 6.9
     KF 6.13.0
    )
add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
add_definitions(-DTRANSLATION_DOMAIN="kruler")

configure_file(krulerconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/krulerconfig.h)

add_subdirectory( pics )

if (KF6DocTools_FOUND)
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
    KF6::CoreAddons
    KF6::I18n
    KF6::Notifications
    KF6::WindowSystem
    KF6::XmlGui
    KF6::StatusNotifierItem
    KF6::Crash
)
if(KRULER_HAVE_X11)
    target_link_libraries(kruler
        ${XCB_LIBRARIES}
        Qt6::GuiPrivate
    )
endif()

install(TARGETS kruler ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install( PROGRAMS org.kde.kruler.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES org.kde.kruler.appdata.xml    DESTINATION  ${KDE_INSTALL_METAINFODIR} )
install( FILES kruler.notifyrc  DESTINATION  ${KDE_INSTALL_KNOTIFYRCDIR} )
install( FILES move.wav  DESTINATION  ${KDE_INSTALL_DATADIR}/kruler/sounds )

ki18n_install(po)
if (KF6DocTools_FOUND)
    kdoctools_install(po)
endif()

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
