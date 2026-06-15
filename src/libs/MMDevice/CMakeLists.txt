set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_definitions(-DMMDEVICE_CLIENT_BUILD)

set(DIR_HEADERS  
	Debayer.h 
	DeviceBase.h 
	DeviceThreads.h 
	DeviceUtils.h 
	ImageMetadata.h 
	ImgBuffer.h 
	MMDevice.h 
	MMDeviceConstants.h 
	ModuleInterface.h 
	Property.h
)

set(DIR_SRCS 
	Debayer.cpp 
	DeviceUtils.cpp 
	ImgBuffer.cpp 
	MMDevice.cpp 
	ModuleInterface.cpp 
	Property.cpp
)

add_library(MMDevice STATIC
        ${DIR_SRCS}
        ${DIR_HEADERS}
)

target_link_libraries(MMDevice )

target_include_directories(MMDevice PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(MMDevice INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
