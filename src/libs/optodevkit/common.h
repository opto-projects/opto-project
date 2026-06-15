
#ifndef SHAREDCOMMON_H
#define SHAREDCOMMON_H

#define CREATEVERSION(major,minor,patch)    (major << 16) + ((minor & 0xff) << 8) + (patch & 0xff)
#define MAJORVERSION(version)               (version >> 16)
#define MINORVERSION(version)               ((version & 0x00ff00) >> 8)
#define PATCHVERSION(version)               (version & 0x0000ff)
#define MAXVERSION                          CREATEVERSION(255,0,0)    //maximum possible version (that means no maximum version is indicated); ck 17.01.2017 changed maxversion major to 255, avoiding warning about too many bits in bitshift CREATEVERSION macro / Linux
#define MINVERSION                          CREATEVERSION(0,0,0)         //minimum possible version

#define DELETE_AND_SET_NULL(pointer) if(pointer != nullptr) { delete pointer; pointer = nullptr;};
#define DELETE_AND_SET_NULL_ARRAY(pointer) if(pointer != nullptr) { delete[] pointer; pointer = nullptr;};

#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 7
#define PLUGIN_VERSION_PATCH 0
#define PLUGIN_VERSION_REVISION 0
#define PLUGIN_VERSION        CREATE_VERSION(PLUGIN_VERSION_MAJOR,PLUGIN_VERSION_MINOR,PLUGIN_VERSION_PATCH)
#define PLUGIN_VERSION_STRING CREATE_VERSION_STRING(PLUGIN_VERSION_MAJOR,PLUGIN_VERSION_MINOR,PLUGIN_VERSION_PATCH)
#define PLUGIN_COMPANY        "OptoChecker"
#define PLUGIN_COPYRIGHT      "(C) 2025, OptoChecker"
#define PLUGIN_NAME           "DummySystem"


#endif
