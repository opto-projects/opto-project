set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV COMPONENTS core highgui imgproc REQUIRED)

set(DIR_SRCS 
	OpenCVgrabber.cpp 
    OpenCVgrabber.h
    DeviceEnumerator.h
    DeviceEnumerator.cpp
)

if(OpenCVgrabber_STATIC)
    add_library(OpenCVgrabber STATIC ${DIR_SRCS})
else()
    add_library(OpenCVgrabber SHARED ${DIR_SRCS})
    
endif()



target_include_directories(OpenCVgrabber PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_include_directories(OpenCVgrabber INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(OpenCVgrabber ${OpenCV_LIBS} MMDevice)

set_target_properties(OpenCVgrabber PROPERTIES
    EXPORT_NAME OpenCVgrabber
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/devices"
)