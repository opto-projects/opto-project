
project(QtPropertyBrowser)


set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtbuttonpropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qteditorfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtgroupboxpropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowserutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertymanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qttreepropertybrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtvariantproperty.cpp
)
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtbuttonpropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qteditorfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtgroupboxpropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowserutils_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertymanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qttreepropertybrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qtvariantproperty.h
)

set(QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/qtpropertybrowser.qrc)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})

set(CMAKE_AUTORCC ON)

option(BUILD_SHARED_LIBS "Build shared library." OFF)
option(BUILD_EXAMPLES "Build examples." OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_library(QtPropertyBrowser STATIC ${HEADER_FILES} ${SOURCE_FILES} ${QRC_FILES})


target_link_libraries(QtPropertyBrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

target_include_directories(QtPropertyBrowser INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set_target_properties(QtPropertyBrowser PROPERTIES AUTOMOC TRUE)

if(BUILD_EXAMPLES)
    add_subdirectory(examples/demo)
endif()
