set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "\n<--- BASE LIBRARY OPTOSCOPE --->")


set(OPTOCAM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(OPTOCAM_LIB_DIR "${OPTOCAM_ROOT}/win/x64")
set(OPTOCAM_INC_DIR  ${OPTOCAM_ROOT}/inc)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPTOCAM_INC_DIR}
)

set(DIR_SRCS 
	AmScope.h
	AmScope.cpp 
    SequenceThread.cpp
    ${OPTOCAM_INC_DIR}/toupcam.h
)

if(OptoScope_STATIC)
    add_library(OptoScope STATIC ${DIR_SRCS})
else()
    add_library(OptoScope SHARED ${DIR_SRCS})
endif()


target_include_directories(OptoScope PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(OptoScope INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(OptoScope PRIVATE MMDevice 
"${OPTOCAM_LIB_DIR}/toupcam.lib"
)

set_target_properties(OptoScope PROPERTIES
    EXPORT_NAME OptoScope
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/devices"
)