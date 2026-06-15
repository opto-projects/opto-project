set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(GNUInstallDirs)
file(GLOB_RECURSE DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE DIR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(library_name "qcustomplot")

message(STATUS "\n<--- BASE LIBRARY ${library_name} --->")
message(STATUS "-- Build directory: ${CMAKE_CURRENT_BINARY_DIR}")

set(QCP_BUILD_STATIC ON)

if(QCP_BUILD_STATIC)
    add_library(${library_name} STATIC
     ${DIR_SRCS}
     ${DIR_HEADERS}
)
    target_compile_definitions( ${library_name} PUBLIC QCP_STATIC)
else()
    add_library(${library_name} SHARED
     ${DIR_SRCS}
     ${DIR_HEADERS})
    target_compile_definitions( ${library_name} PRIVATE QCP_SHARED_EXPORT)
endif()

install(FILES ${DIR_HEADERS}
    DESTINATION include/${library_name}
    COMPONENT headers
)

target_link_libraries(${library_name} PRIVATE
    Qt::Widgets
    Qt::PrintSupport
)

target_include_directories(${library_name} PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
