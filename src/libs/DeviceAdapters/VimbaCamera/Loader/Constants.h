
#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifdef __linux__
static std::string VIMBA_X_LIB_DIR = std::string("");         //<! Vimba X library path within installation directory
static constexpr const char *VIMBA_X_LIB_NAME = "libVmbC.so"; //<! Vimba X library name
static constexpr const char *VIMBA_X_IMAGE_TRANSFORM_NAME = "libVmbImageTransform.so"; //<! Vimba X Image Transform library name
#elif _WIN32
static constexpr const char *VIMBA_X_ENV_VAR = "VIMBA_X_HOME"; //<! Vimba X Environmental variable name
static const char *ENV_VALUE = std::getenv(VIMBA_X_ENV_VAR);   //<! Vimba environment variable value
static std::string VIMBA_X_LIB_DIR =
    std::string(ENV_VALUE != nullptr ? ENV_VALUE : "") + std::string("\\bin");       //<! Vimba X library path within installation directory
static constexpr const char *VIMBA_X_LIB_NAME = "VmbC.dll";                          //<! Vimba X library name
static constexpr const char *VIMBA_X_IMAGE_TRANSFORM_NAME = "VmbImageTransform.dll"; //<! Vimba X Image Transform library name
#else
// nothing
#endif

#endif
