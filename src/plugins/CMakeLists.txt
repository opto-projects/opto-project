#project(plugins )
add_library(plugins INTERFACE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")

#add_subdirectory(opto_demo_extension)
#add_subdirectory(stillimage_system)
#add_subdirectory(CameraExtension)
#add_subdirectory(AxialPsfAnalyzerExtension)
#add_subdirectory(ImageStatisticsExtension)
add_subdirectory(BeamSystem)