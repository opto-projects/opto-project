set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "\n<--- BASE LIBRARY BaslerCamera --->")
message(STATUS "-- Build directory: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "-- Source directory: ${CMAKE_CURRENT_SOURCE_DIR}")

set(PYLON_ROOT $ENV{PYLON_DEV_DIR})
if (NOT DEFINED ENV{PYLON_DEV_DIR})
    set(PYLON_ROOT "D:\Program Files\Basler\pylon 5\Development")
endif()
set(PYLON_LIB_DIR ${PYLON_ROOT}/lib/x64)
set(PYLON_INC_DIR  ${PYLON_ROOT}/include)


set(DIR_SRCS 
	BaslerPylonCamera.cpp
	BaslerPylonCamera.h 
)

if(BaslerCamera_STATIC)
    add_library(BaslerCamera STATIC ${DIR_SRCS})
else()
    add_library(BaslerCamera SHARED ${DIR_SRCS})
endif()



target_include_directories(BaslerCamera PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PYLON_INC_DIR}
)

target_include_directories(BaslerCamera INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BaslerCamera PRIVATE MMDevice 
"${PYLON_LIB_DIR}/PylonC.lib"
"${PYLON_LIB_DIR}/GCBase_MD_VC141_v3_1_Basler_pylon.lib"
"${PYLON_LIB_DIR}/GenApi_MD_VC141_v3_1_Basler_pylon.lib"
"${PYLON_LIB_DIR}/PylonBase_v5_2.lib"
"${PYLON_LIB_DIR}/PylonGUI_v5_2.lib"
"${PYLON_LIB_DIR}/PylonUtility_v5_2.lib"
)

set_target_properties(BaslerCamera PROPERTIES
    EXPORT_NAME BaslerCamera
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/devices"
)