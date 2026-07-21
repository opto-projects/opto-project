add_library(app_version INTERFACE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Gui Core Widgets LinguistTools Quick QuickWidgets PrintSupport)

option(WITH_CUDA "Add support for GPU (CUDA must be installed)" OFF)
set(WITH_CUDA OFF)
option(WITH_OPENCV "Add support for OPENCV (OPENCV must be installed)" ON)
set(WITH_OPENCV ON)


if(WITH_CUDA)
    add_subdirectory(libs/cuda)
endif()

if(WITH_OPENCV)

    find_package(OpenCV COMPONENTS core highgui imgproc REQUIRED)
    #if(OpenCV_VERSION_MAJOR GREATER 2)
        # for opencv >= 3 we need the imgcodecs module which is not present in opencv < 3 so run again
        #  imgcodecs videoio
    #    find_package(OpenCV COMPONENTS core highgui imgproc REQUIRED)
    #endif(OpenCV_VERSION_MAJOR GREATER 2)
    
    set(OpenCV_LIBRARY_DIRS "${OpenCV_INSTALL_PATH}/x64/vc15/bin")

    message(STATUS "--------------- OpenCV_LIBRARS ${OpenCV_LIBS} OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS} -----------------")
endif()


add_subdirectory(libs)
add_subdirectory(app)
add_subdirectory(plugins)

