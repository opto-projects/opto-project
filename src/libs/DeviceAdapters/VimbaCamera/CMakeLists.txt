set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "\n<--- BASE LIBRARY VimbaCamera --->")
message(STATUS "-- Build directory: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "-- Source directory: ${CMAKE_CURRENT_SOURCE_DIR}")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vmb_cmake_prefix_paths.cmake")
    # read hardcoded package location information, if the example is still located in the original install location
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vmb_cmake_prefix_paths.cmake)
endif()

find_package(Vmb REQUIRED COMPONENTS CPP NAMES Vmb VmbC VmbCPP VmbImageTransform)

set(DIR_SRCS 
	AlliedVisionCamera.cpp
	AlliedVisionCamera.h 
	AlliedVisionDeviceBase.cpp 
	AlliedVisionDeviceBase.h 
	AlliedVisionHub.cpp 
	AlliedVisionHub.h 
	Loader/Constants.h 
	Loader/LibLoader.cpp 
	Loader/LibLoader.h
)

if(VimbaCamera_STATIC)
    add_library(VimbaCamera STATIC ${DIR_SRCS})
else()
    add_library(VimbaCamera SHARED ${DIR_SRCS})
    
endif()



target_include_directories(VimbaCamera PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_include_directories(VimbaCamera INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(VimbaCamera PRIVATE Vmb::CPP MMDevice)

set_target_properties(VimbaCamera PROPERTIES
    EXPORT_NAME VimbaCamera
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/devices"
)