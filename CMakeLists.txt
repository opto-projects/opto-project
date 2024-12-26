cmake_minimum_required(VERSION 3.5)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/cmake)

include(SetupCore)

file(READ "release/version.txt" APP_VER_FULL)

project(optochecker VERSION ${APP_VER_FULL} LANGUAGES C CXX)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP APP_VER_YEAR "%Y")

string(REPLACE "." ";" APP_VER_LIST ${APP_VER_FULL})
list(GET APP_VER_LIST 0 APP_VER_MAJOR)
list(GET APP_VER_LIST 1 APP_VER_MINOR)
list(GET APP_VER_LIST 2 APP_VER_PATCH)

configure_file(
    release/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(WITH_CUDA "Add support for GPU (CUDA must be installed)" OFF)
set(WITH_CUDA OFF)

option(WITH_OPENCV "Add support for OPENCV (OPENCV must be installed)" ON)
set(WITH_OPENCV ON)


#add_compile_options(
#    -O3
#    -ffast-math
#    -funsafe-math-optimizations
#    -msse4.2
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Widgets PrintSupport)

file(GLOB_RECURSE DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/app/*.cpp)
file(GLOB_RECURSE DIR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/app/*.h)
file(GLOB_RECURSE DIR_HEADLESS ${CMAKE_CURRENT_SOURCE_DIR}/src/app/*.hpp)
file(GLOB_RECURSE DIR_QRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc)
file(GLOB_RECURSE DIR_UI ${CMAKE_CURRENT_SOURCE_DIR}/src/app/*.ui)

#QT5_WRAP_UI(WRAP_FILES ${DIR_UI})

add_subdirectory(libs/qcustomplot)
add_subdirectory(libs/optodevkit)



if(WITH_CUDA)
    add_subdirectory(libs/cuda)
endif()

SET(TS_FILES
src/zh_CN.ts
)
find_package(Qt5LinguistTools)
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -source-language en_US -no-obsolete)



if(WITH_OPENCV)
    find_package(OpenCV COMPONENTS core highgui imgproc REQUIRED)
    if(OpenCV_VERSION_MAJOR GREATER 2)
        # for opencv >= 3 we need the imgcodecs module which is not present in opencv < 3 so run again
        find_package(OpenCV COMPONENTS core highgui imgproc imgcodecs videoio REQUIRED)
    endif(OpenCV_VERSION_MAJOR GREATER 2)
    message(STATUS "--------------- OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS} ${OpenCV_VERSION}-----------------")


endif()


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${LIB_RESOURCES}
        ${QM_FILES} 
    )
else()
    add_executable(${PROJECT_NAME}
        ${DIR_SRCS}
        ${DIR_HEADERS}
        ${DIR_HEADLESS}
        ${DIR_QRC}
        ${DIR_UI}
    )
endif()


target_link_libraries(${PROJECT_NAME} PRIVATE
    optodevkit
    qcustomplot
    Qt::Widgets
    ${OpenCV_LIBS}
    opengl32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER opto-project.optochecker
    MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VER_FULL}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_INCLUDE_CURRENT_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BUILD_DATE="${BUILD_DATE}"
    APP_VER_MAJOR=${APP_VER_MAJOR}
    APP_VER_MINOR=${APP_VER_MINOR}
    APP_VER_PATCH=${APP_VER_PATCH}
    APP_VER_YEAR=${APP_VER_YEAR}
    QT_USE_QSTRINGBUILDER
)



if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()


add_subdirectory(plugins)