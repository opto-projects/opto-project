set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE DIR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE DIR_HEADLESS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(qcustomplot STATIC
     ${DIR_SRCS}
        ${DIR_HEADERS}
        ${DIR_HEADLESS}
)

target_link_libraries(qcustomplot PRIVATE
    Qt::Widgets
    Qt::PrintSupport
)

target_include_directories(qcustomplot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/qcp/src
)

target_include_directories(qcustomplot INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
