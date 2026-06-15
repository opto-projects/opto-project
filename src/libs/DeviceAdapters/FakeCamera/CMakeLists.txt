set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV COMPONENTS core highgui imgproc REQUIRED)

set(DIR_SRCS 
	FakeCamera.cpp
	FakeCamera.h
  	error_code.cpp
  	error_code.h
	module.cpp
)

if(FakeCamera_STATIC)
    add_library(FakeCamera STATIC ${DIR_SRCS})
else()
    add_library(FakeCamera SHARED ${DIR_SRCS})
    
endif()



target_include_directories(FakeCamera PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_include_directories(FakeCamera INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FakeCamera ${OpenCV_LIBS} MMDevice)

set_target_properties(FakeCamera PROPERTIES
    EXPORT_NAME FakeCamera
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/devices"
)