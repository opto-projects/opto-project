set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DIR_SRCS 
	DemoCamera.cpp 
    DemoCamera.h
    writeCompactTiffRGB.h
)

if(DemoCamera_STATIC)
    add_library(DemoCamera STATIC ${DIR_SRCS})
else()
    add_library(DemoCamera SHARED ${DIR_SRCS})
    
endif()



target_include_directories(DemoCamera PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_include_directories(DemoCamera INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DemoCamera ${OpenCV_LIBS} MMDevice)

set_target_properties(DemoCamera PROPERTIES
    EXPORT_NAME DemoCamera
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/devices"
)