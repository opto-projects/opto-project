

if (WIN32)
  list(APPEND DEFAULT_DEFINES UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
endif()

###############################################################################
# Python Definitions
message(STATUS "--------------- Python_ROOT_DIR ${Python_ROOT_DIR} PythonDir $ENV{PythonDir}-----------------")
if(NOT EXISTS ${Python_ROOT_DIR})
    if(EXISTS $ENV{PythonDir})
        set(Python_ROOT_DIR $ENV{PythonDir} CACHE PATH "Path to the root directory of the Python installation")
    else(EXISTS $ENV{PythonDir})
        set(Python_ROOT_DIR "Python_ROOT_DIR-NOTFOUND" CACHE PATH "Path to the root directory of the Python installation")
    endif(EXISTS $ENV{PythonDir})
endif(NOT EXISTS ${Python_ROOT_DIR})

if(WIN32)
    if(NOT EXISTS ${Python_ROOT_DIR})
        message(FATAL_ERROR "Depencencies Missing for Python Library. \
        Please make sure that the Cmake variable Python_ROOT_DIR or \
        the Environment Variable PythonDir are well defined and \
        point to the root directory of the Python installation, that \
        should be used for compiling project.")
    endif(NOT EXISTS ${Python_ROOT_DIR})
endif(WIN32)


###############################################################################
# OpenCV Definitions
message(STATUS "--------------- OPENCV_ROOT_DIR ${OPENCV_ROOT_DIR} OpenCV_Dir $ENV{OpenCV_Dir}-----------------")
if(NOT EXISTS ${OPENCV_ROOT_DIR})
    if(EXISTS $ENV{OpenCV_Dir})
        set(OPENCV_ROOT_DIR $ENV{OpenCV_Dir} CACHE PATH "Path to the OpenCV root directory")
    else(EXISTS $ENV{OpenCV_Dir})
        set(OPENCV_ROOT_DIR "OpenCV_DIR-NOTFOUND" CACHE PATH "Path to the OpenCV root directory")
    endif(EXISTS $ENV{OpenCV_Dir})
endif(NOT EXISTS ${OPENCV_ROOT_DIR})


if(WIN32)
    if(NOT EXISTS ${OPENCV_ROOT_DIR})
        message(FATAL_ERROR "Depencencies Missing for OpenCV Library. \
        Please make sure that the Cmake Variable OPENCV_ROOT_DIR or the \
        environment variable OpenCV_Dir are well defined and point \
        to the root directory of OpenCV.")
    endif(NOT EXISTS ${OPENCV_ROOT_DIR})
endif(WIN32)


###############################################################################
# QT Definitions
message(STATUS "--------------- BUILD_QTVERSION ${BUILD_QTVERSION} Qt_Prefix_DIR ${Qt_Prefix_DIR} Qt5_DIR $ENV{Qt5_DIR}-----------------")
if(NOT EXISTS BUILD_QTVERSION)
    SET(BUILD_QTVERSION "Qt5" CACHE STRING "Qt Version to be used, currently only Qt6 and Qt5 is supported.Qt5 by default.")
endif(NOT EXISTS BUILD_QTVERSION)
set_property(CACHE BUILD_QTVERSION PROPERTY STRINGS Qt6 Qt5)

if(NOT EXISTS ${Qt_Prefix_DIR})
    if(EXISTS $ENV{Qt5_DIR})
        set(Qt_Prefix_DIR $ENV{Qt5_DIR} CACHE PATH "Path to the Qt Directory")
    else(EXISTS $ENV{Qt5_DIR})
        set(Qt_Prefix_DIR "Qt_Prefix_DIR-NOTFOUND" CACHE PATH "Path to the Qt Directory")
    endif(EXISTS $ENV{Qt5_DIR})
endif(NOT EXISTS ${Qt_Prefix_DIR})

message(STATUS "--------------- Qt_DIR ${Qt_DIR} Qt5_DIR $ENV{Qt5_DIR}-----------------")

if(WIN32)
    if(NOT EXISTS ${Qt_Prefix_DIR})
        message(FATAL_ERROR "Depencencies missing for Qt Library. \
        Please make sure that the Cmake Variable Qt_Prefix_DIR or the environment \
        variable Qt5_DIR are well defined and point to the root directory \
        of the Qt installation, e.g. a folder like ...Qt/6.4.0/msvc2019_64")
    endif(NOT EXISTS ${Qt_Prefix_DIR})
endif(WIN32)



