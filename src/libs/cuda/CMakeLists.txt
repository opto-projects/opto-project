set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE DIR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/common/*.h)
file(GLOB_RECURSE GL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/common/inc/*.h)

add_library(optodevkit STATIC
        ${GL_HEADERS}
        ${DIR_HEADERS}
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
