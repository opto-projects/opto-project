
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
message(STATUS "APP Version is ${APP_VER_FULL}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ads_demo_SRCS
    main.cpp
    MainWindow.cpp
    mainwindow.ui
    StatusDialog.cpp
    StatusDialog.ui
    ImageViewer.cpp
    RenderWidget.cpp
	demo.qrc
)
add_executable(${PROJECT_NAME} WIN32 ${ads_demo_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")
target_link_libraries(${PROJECT_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Core 
                                                       Qt${QT_VERSION_MAJOR}::Gui 
                                                       Qt${QT_VERSION_MAJOR}::Widgets
                                                       Qt${QT_VERSION_MAJOR}::Quick
                                                       Qt${QT_VERSION_MAJOR}::QuickWidgets)

target_link_libraries(${PROJECT_NAME} PRIVATE qt${QT_VERSION_MAJOR}advanceddocking)
set_target_properties(${PROJECT_NAME} PROPERTIES 
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VERSION ${VERSION_SHORT}
    EXPORT_NAME "Qt Advanced Docking System Demo"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/bin"
)
#if(BUILD_STATIC)
#    target_compile_definitions(AdvancedDockingSystemDemo PRIVATE ADS_STATIC)
#endif()

