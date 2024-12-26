set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE DIR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE DIR_HEADLESS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(optodevkit STATIC
        ${DIR_SRCS}
        ${DIR_HEADERS}
        ${DIR_HEADLESS}
)

target_link_libraries(optodevkit PRIVATE
    Qt::Core
    Qt::Widgets
)

target_include_directories(optodevkit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(optodevkit INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


