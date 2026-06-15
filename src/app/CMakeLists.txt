

file(READ "release/version.txt" APP_VER_FULL)

project(optochecker VERSION ${APP_VER_FULL} LANGUAGES C CXX)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP APP_VER_YEAR "%Y")

string(REPLACE "." ";" APP_VER_LIST ${APP_VER_FULL})
list(GET APP_VER_LIST 0 APP_VER_MAJOR)
list(GET APP_VER_LIST 1 APP_VER_MINOR)
list(GET APP_VER_LIST 2 APP_VER_PATCH)

configure_file(
    release/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)
message(STATUS "APP Version is ${APP_VER_FULL}")

message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "PROJECT_SOURCE_DIR dir: ${PROJECT_SOURCE_DIR}")
#add_compile_options(
#    -O3
#    -ffast-math
#    -funsafe-math-optimizations
#    -msse4.2

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE DIR_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE DIR_QRC ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)
file(GLOB_RECURSE DIR_UI ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)


#QT5_WRAP_UI(WRAP_FILES ${DIR_UI})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED LinguistTools)

set(TRANSLATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/translations")
file(GLOB TS_FILES "${TRANSLATIONS_DIR}/*.ts")

find_program(LUPDATE_EXE lupdate)
if (LUPDATE_EXE)
    message(STATUS "lupdate found: ${LUPDATE_EXE}")
    message(STATUS "TS file: ${TS_FILES}")
    message(STATUS "DIR_SRCS file: ${DIR_SRCS}")
    

    #add_custom_command( OUTPUT ${TS_FILES} COMMAND ${LUPDATE_EXE} ${PROJECT_SOURCE_DIR} -ts ${TS_FILES} DEPENDS ${DIR_SRCS}   COMMENT "Updating translation files")

    #add_custom_target(LUPDATE_EXE ALL DEPENDS ${TS_FILES})

    add_custom_target(update_translations ALL
        COMMAND ${LUPDATE_EXE} ${PROJECT_SOURCE_DIR} -ts ${TS_FILES}
        COMMENT "Updating translation files"
    )
endif ()

QT_CREATE_TRANSLATION(QM_FILES ${DIR_SRCS} ${TS_FILES} )


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${LIB_RESOURCES}
        ${QM_FILES} 
    )
else()
    add_executable(${PROJECT_NAME}
        ${DIR_SRCS}
        ${DIR_HEADERS}
        ${DIR_QRC}
        ${DIR_UI}
        ${QM_FILES} 
    )
endif()


target_link_libraries(${PROJECT_NAME} PRIVATE
    qtadvanceddocking
    optodevkit
    qcustomplot
    qwt
    imageview
    Qt::Gui
    Qt::Core
    Qt::Widgets
    Qt::Quick
    ${OpenCV_LIBS}
    opengl32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER opto-project.optochecker
    MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VER_FULL}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_INCLUDE_CURRENT_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BUILD_DATE="${BUILD_DATE}"
    APP_VER_MAJOR=${APP_VER_MAJOR}
    APP_VER_MINOR=${APP_VER_MINOR}
    APP_VER_PATCH=${APP_VER_PATCH}
    APP_VER_YEAR=${APP_VER_YEAR}
    QT_USE_QSTRINGBUILDER
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _USE_MATH_DEFINES
    )
endif()

# 将 .qm 文件复制到构建目录
#file(COPY ${QM_FILES} DESTINATION ${TRANSLATIONS_DIR})