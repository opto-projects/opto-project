
set(target_name CameraExtension)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Gui Widgets Multimedia MultimediaWidgets)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/overlayitems
)


file(GLOB_RECURSE PLUGIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PLUGIN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE PLUGIN_UI ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui)

file(GLOB_RECURSE PLUGIN_SOURCES_OVERLAYITEMS ${CMAKE_CURRENT_SOURCE_DIR}/src/overlayitems/*.cpp)
file(GLOB_RECURSE PLUGIN_HEADERS_OVERLAYITEMS ${CMAKE_CURRENT_SOURCE_DIR}/src/overlayitems/*.h)


add_library(${target_name} SHARED 
    ${PLUGIN_SOURCES} 
    ${PLUGIN_HEADERS} 
    ${PLUGIN_UI} 
    ${PLUGIN_SOURCES_OVERLAYITEMS} 
    ${PLUGIN_HEADERS_OVERLAYITEMS}
)

# Qt: enable all automoc, autouic and autorcc.
set_target_properties(${target_name} PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC ON)

target_link_libraries(${target_name} PRIVATE
    optodevkit
    Qt::Core
    Qt::Widgets
    Qt::Gui
    Qt::Multimedia
    Qt::MultimediaWidgets
)

#translation
set(FILES_TO_TRANSLATE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS} ${PLUGIN_UI})
