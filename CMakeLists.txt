cmake_minimum_required(VERSION 3.22)
project(win7_stickynotes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(QT_MAJOR_VERSION 6)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick DBus)
find_package(KF6 REQUIRED COMPONENTS Package)

include(KDEInstallDirs6)
include(KDECMakeSettings)
include(ECMQmlModule)

kpackage_install_package(package org.kde.plasma.win7stickynotes plasmoids plasma)

ecm_add_qml_module(stickynotesplugin
    URI "org.kde.plasma.win7stickynotes.backend"
    VERSION 1.1
    GENERATE_PLUGIN_SOURCE
)

target_sources(stickynotesplugin PRIVATE
    src/notebridge.h
    src/notebridge.cpp
)

target_include_directories(stickynotesplugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(stickynotesplugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::DBus
)

ecm_finalize_qml_module(stickynotesplugin DESTINATION ${KDE_INSTALL_QMLDIR})
