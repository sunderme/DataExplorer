cmake_minimum_required(VERSION 3.5)

project(DataExplorer VERSION 1.16 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(GitInfo)
git_wc_info(${PROJECT_SOURCE_DIR} VARPREFIX)

message(STATUS "The current git hash is ${VARPREFIX_WC_REVISION_HASH}")

add_definitions(-DDE_VERSION=\"${VARPREFIX_WC_LATEST_TAG_LONG}\")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/win.rc")

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/zoomablechart.cpp
        src/zoomablechart.h
        src/zoomablechartview.cpp
        src/zoomablechartview.h
        src/rangelimitedvalueaxis.cpp
        src/rangelimitedvalueaxis.h
        src/verticalmarker.h src/verticalmarker.cpp
        src/horizontalmarker.h src/horizontalmarker.cpp
        src/abmarker.h src/abmarker.cpp
        src/callout.h src/callout.cpp
        resources/icons.qrc
        ${APP_ICON_RESOURCE_WINDOWS}
        resources/DataExplorer.icns
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(DataExplorer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET DataExplorer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(DataExplorer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(DataExplorer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(DataExplorer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts)

set_source_files_properties(resources/DataExplorer.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_target_properties(DataExplorer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER DataExplorer
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE DataExplorer.icns
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DataExplorer)
endif()
