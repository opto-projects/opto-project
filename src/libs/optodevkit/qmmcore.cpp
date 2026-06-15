#include "QMMCore.h"
#include "MMCore/CoreUtils.h"

#include <opencv2/core.hpp>

#include <QFile>
#include <QTextStream>



QMMCore::QMMCore(QObject* parent) : QObject(parent) {

    
}

QMMCore::~QMMCore() {
}

void QMMCore::SetupDeviceConfigManager(QWidget* parent)
{
    if (deviceManager_ == nullptr)
    {
        deviceManager_ = new DeviceConfigManager(&core_, parent);
    }
    deviceManager_->show();
    deviceManager_->raise();
    deviceManager_->activateWindow();
}

void QMMCore::CreatePropertyBrowser()
{
    if (propertBrowser == nullptr)
    {
        propertBrowser = new MMCorePropertyBrowser(&core_);
    }
    //propertBrowser->show();
    //propertBrowser->raise();
    //propertBrowser->activateWindow();
}

bool QMMCore::LoadSystem()
{
    //SetupDeviceConfigManager(nullptr);

    std::vector<std::string> modules = core_.getDeviceAdapterNames();
    for (size_t i = 0; i < modules.size(); i++)
    {
        emit info(QString("Load Module: %1").arg(QString(modules[i].c_str())));
        std::vector<std::string> devices = core_.getAvailableDevices(modules[i].c_str());
        std::vector<long> deviceTypes =core_.getAvailableDeviceTypes(modules[i].c_str());
        for (size_t i = 0; i < devices.size(); i++)
        {
            std::string type = ToString(MM::DeviceType(deviceTypes[i]));
            emit info(QString("Load Device: %1 Type: %2").arg(QString(devices[i].c_str())).arg(QString(type.c_str())));
        }
    }

    core_.setPrimaryLogFile("app.log", true);
    core_.loadDevice("Camera", "FakeCamera", "FakeCamera");

    emit info(QString("Init Sytem: %1").arg(QString("Camera")));

    return 1;
}

bool QMMCore::OpenSystem()
{
    // initialize
    core_.initializeAllDevices();
    core_.setCameraDevice("Camera");
    core_.registerCallback(this);

    emit info(QString("Open Sytem: %1").arg(QString("AllDevices")));

    return 1;
}

void QMMCore::SystemConfigurationLoaded()
{
    // list devices
    std::vector<std::string> devices = core_.getLoadedDevices();
    emit info(QString("Device status:"));

    for (int i = 0; i < devices.size(); i++) {

        emit info(QString(" %1").arg(i));

        std::vector<std::string> properties = core_.getDevicePropertyNames(devices[i].c_str());
        for (int j = 0; j < properties.size(); j++) {
            std::string prop = core_.getProperty(devices[i].c_str(), properties[j].c_str());
            std::vector<std::string>propV = core_.getAllowedPropertyValues(devices[i].c_str(), properties[j].c_str());
            emit info(QString(" %1 = %2").arg(properties[j].c_str()).arg(prop.c_str()));

            for (int k = 0; k < propV.size(); k++) {
                emit info(QString(" %1").arg(propV[k].c_str()));
            }
        }
    }

    emit info(QString("Configuration status:"));

    std::vector<std::string> configsGroup = core_.getAvailableConfigGroups();
    for (int i = 0; i< configsGroup.size(); i++) {
        emit info(QString(" %1").arg(i));
        std::vector<std::string> configs = core_.getAvailableConfigs(configs[i].c_str());
        for (size_t j = 0; j < configs.size(); j++)
        {
            emit info(QString(" Group %1, Config %2").arg(configsGroup[i].c_str()).arg(configs[i].c_str()));

            Configuration cdata = core_.getConfigData(configsGroup[i].c_str(), configs[i].c_str());

            for (int k = 0; k < cdata.size(); j++) {
                PropertySetting s = cdata.getSetting(j);
                emit info(QString(" %1  %2  %3").arg(s.getDeviceLabel().c_str())
                    .arg(s.getPropertyName().c_str()).arg(s.getPropertyValue().c_str()));
            }
        }
        
    }

}

void QMMCore::logInfo(const QString msg)
{
    core_.logMessage(msg.toStdString().c_str(), 0);
}

void QMMCore::logError(const QString msg)
{
    QString errormsg = QString("Error message: %1").arg(msg);

    core_.logMessage(msg.toStdString().c_str(), 0);
}

void QMMCore::slot_enableGui(bool enable)
{
    propertBrowser->enableGui(enable);
}


//Í¼Ïñ²É¼¯
bool QMMCore::startSingleImageAcquisition() {
    try {
        core_.snapImage();
        return true;
    }
    catch (const CMMError& e) {
        emit error(QString("Error starting single image acquisition: %1").arg(e.getMsg().c_str()));
        return false;
    }
}

bool QMMCore::startContinuousSequenceAcquisition(double intervalMs) {
    try {
        core_.startContinuousSequenceAcquisition(intervalMs);
        isContinuousAcquisition_ = true;
        return true;
    }
    catch (const CMMError& e) {
        emit error(QString("Error starting continuous sequence acquisition: %1").arg(e.getMsg().c_str()));
        return false;
    }
}

bool QMMCore::stopSequenceAcquisition() {
    try {
        core_.stopSequenceAcquisition();
        isContinuousAcquisition_ = false;
        return true;
    }
    catch (const CMMError& e) {
        emit error(QString("Error stopping sequence acquisition: %1").arg(e.getMsg().c_str()));
        return false;
    }
}

void  QMMCore::snapImage() {
    try {
        core_.snapImage();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
    }
}

void* QMMCore::getImage() {
    try {
        return core_.getImage();
    }
    catch (const CMMError& e) {
        emit error(QString("Error: %1").arg(e.getMsg().c_str()));
        return NULL;
    }
}
void* QMMCore::getImage(unsigned numChannel) {
    try {
        return core_.getImage(numChannel);
    }
    catch (const CMMError& e) {
        emit error(QString("Error: %1").arg(e.getMsg().c_str()));
        return NULL;
    }
}

unsigned  QMMCore::getImageWidth() {
    try {
        return core_.getImageWidth();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}
unsigned  QMMCore::getImageHeight() {
    try {
        return core_.getImageHeight();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}
unsigned  QMMCore::getBytesPerPixel() {
    try {
        return core_.getBytesPerPixel();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}
unsigned  QMMCore::getImageBitDepth() {
    try {
        return core_.getImageBitDepth();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}
unsigned  QMMCore::getNumberOfComponents() {
    try {
        return core_.getNumberOfComponents();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}
unsigned  QMMCore::getNumberOfCameraChannels() {
    try {
        return core_.getNumberOfCameraChannels();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}
std::string  QMMCore::getCameraChannelName(unsigned int channelNr) {
    try {
        return core_.getCameraChannelName(channelNr);
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}
long  QMMCore::getImageBufferSize() {
    try {
        return core_.getImageBufferSize();
    }
    catch (const CMMError& e) {
        emit error(QString("Error : %1").arg(e.getMsg().c_str()));
        return 0;
    }
}