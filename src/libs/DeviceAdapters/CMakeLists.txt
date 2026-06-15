#project(plugins )
add_library(DeviceAdapters INTERFACE)


add_subdirectory(OpenCVgrabber)
add_subdirectory(DemoCamera)
add_subdirectory(VimbaCamera)
add_subdirectory(Basler)
add_subdirectory(FakeCamera)
add_subdirectory(OptoScope)