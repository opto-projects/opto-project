
#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4290) // 'C++ exception specification ignored'
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
// 'dynamic exception specifications are deprecated in C++11 [-Wdeprecated]'
#pragma GCC diagnostic ignored "-Wdeprecated"
#endif

#include <string>
#include <vector>
#include <map>
#include "Error.h"


/**
 * Property setting defined as triplet:
 * device - property - value.
 */
struct PropertySetting
{
   /**
    * Constructor for the struct specifying the entire contents.
    * @param deviceLabel
    * @param prop
    * @param value 
    */
    PropertySetting(const char* deviceLabel, const char* prop, const char* value, bool readOnly = false) :
      deviceLabel_(deviceLabel), propertyName_(prop), value_(value), readOnly_(readOnly)
      {
        key_ = generateKey(deviceLabel, prop);
      }

    PropertySetting() : readOnly_(false) {}
    ~PropertySetting() {}

   /**
    * Returns the device label.
    */
   std::string getDeviceLabel() const {return deviceLabel_;}
   /**
    * Returns the property name.
    */
   std::string getPropertyName() const {return propertyName_;}
   /**
    * Returns the read-only status.
    */
   bool getReadOnly() const {return readOnly_;}
   /**
    * Returns the property value.
    */
   std::string getPropertyValue() const {return value_;}

   std::string getKey() const {return key_;}

   static std::string generateKey(const char* device, const char* prop);

   std::string getVerbose() const;
   bool isEqualTo(const PropertySetting& ps);

private:
   std::string deviceLabel_;
   std::string propertyName_;
   std::string value_;
   std::string key_;
   bool readOnly_;
};

/**
 * Encapsulation of the configuration information. Designed to be wrapped
 * by SWIG. A collection of configuration settings.
 */
class Configuration
{
public:

   Configuration() {}
   ~Configuration() {}

   /**
    * Adds new property setting to the existing contents.
    */
   void addSetting(const PropertySetting& setting);
   void deleteSetting(const char* device, const char* prop);

   bool isPropertyIncluded(const char* device, const char* property);
   bool isSettingIncluded(const PropertySetting& ps);
   bool isConfigurationIncluded(const Configuration& cfg);

   PropertySetting getSetting(size_t index) const throw (CMMError);
   PropertySetting getSetting(const char* device, const char* prop);
   
   /**
    * Returns the number of settings.
    */
   size_t size() const {return settings_.size();}
   std::string getVerbose() const;
 
private:
   std::vector<PropertySetting> settings_;
   std::map<std::string, int> index_;
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
