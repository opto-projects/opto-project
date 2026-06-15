#ifndef _QMMCORE_H_
#define _QMMCORE_H_

#include "MMCore/MMCore.h"
#include "MMCore/MMEventCallback.h"
#include "DeviceConfigManager.h"
#include "MMCorePropertyBrowser.h"


#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QImage>

class QMMCore : public QObject , public MMEventCallback {
    Q_OBJECT

public:
    explicit QMMCore(QObject* parent = nullptr);
    virtual ~QMMCore();

    // MMCore ÊµÀý
    CMMCore core_;
    bool isContinuousAcquisition_ = false;
    DeviceConfigManager* deviceManager_ = nullptr;
    MMCorePropertyBrowser* propertBrowser = nullptr;

    void SetupDeviceConfigManager(QWidget* parent);
    void CreatePropertyBrowser();
    bool LoadSystem();
    bool OpenSystem();
    void SystemConfigurationLoaded();

    bool startSingleImageAcquisition();
    bool startContinuousSequenceAcquisition(double intervalMs);
    bool stopSequenceAcquisition();
    
    void snapImage();
    void* getImage() ;
    void* getImage(unsigned numChannel);

    unsigned getImageWidth();
    unsigned getImageHeight();
    unsigned getBytesPerPixel();
    unsigned getImageBitDepth();
    unsigned getNumberOfComponents();
    unsigned getNumberOfCameraChannels();
    std::string getCameraChannelName(unsigned int channelNr);
    long getImageBufferSize();


    void logInfo(const QString msg);
    void logError(const QString msg);
public slots:
    void slot_enableGui(bool enable);
signals:
    void newImageReady(const QString& cameraLabel, int imageNumber);

    void propertiesChanged();

    void propertyChanged(const QString& deviceLabel, const QString& propName, const QString& propValue);
    void channelGroupChanged(const QString& newChannelGroupName);
    void configGroupChanged(const QString& groupName, const QString& newConfigName);
    void systemConfigurationLoaded();
    void pixelSizeChanged(double newPixelSizeUm);
    void pixelSizeAffineChanged(double v0, double v1, double v2, double v3, double v4, double v5);
    void stagePositionChanged(const QString& name, double pos);
    void xyStagePositionChanged(const QString& name, double xPos, double yPos);
    void exposureChanged(const QString& name, double newExposure);
    void slmExposureChanged(const QString& name, double newExposure);

    void error(QString);
    void info(QString);
private:
    virtual void onPropertiesChanged() override
    {
        std::cout << "onPropertiesChanged()\n";

        QString msg = QString("onPropertiesChanged()");
        info(msg);

        emit propertiesChanged();
    }
    virtual void onPropertyChanged(const char* name, const char* propName, const char* propValue) override
    {
        std::cout << "onPropertyChanged() " << name << " " << propName << " " << propValue << '\n';

        QString msg = QString("onPropertyChanged() %1 %2 %3").arg(name).arg(propName).arg(propValue);
        info(msg);

        emit propertyChanged(name, propName, propValue);
    }
    virtual void onChannelGroupChanged(const char* newChannelGroupName)override
    {
        std::cout << "onChannelGroupChanged() " << newChannelGroupName << '\n';

        QString msg = QString("onChannelGroupChanged() %1").arg(newChannelGroupName);
        info(msg);

        emit channelGroupChanged(newChannelGroupName);
    }
    virtual void onConfigGroupChanged(const char* groupName, const char* newConfigName)override
    {
        std::cout << "onConfigGroupChanged() " << groupName << " " << newConfigName << '\n';

        QString msg = QString("onConfigGroupChanged() %1 %2").arg(groupName).arg(newConfigName);
        info(msg);

        emit configGroupChanged(groupName, newConfigName);
    }
    virtual void onSystemConfigurationLoaded()override
    {
        std::cout << "onSystemConfigurationLoaded() \n";

        QString msg = QString("onSystemConfigurationLoaded()");
        info(msg);

        emit systemConfigurationLoaded();
    }
    virtual void onPixelSizeChanged(double newPixelSizeUm)override
    {
        std::cout << "onPixelSizeChanged() " << newPixelSizeUm << '\n';

        QString msg = QString("onPixelSizeChanged() %1").arg(newPixelSizeUm);
        info(msg);

        emit pixelSizeChanged(newPixelSizeUm);
    }
    virtual void onPixelSizeAffineChanged(double v0, double v1, double v2, double v3, double v4, double v5)override
    {
        std::cout << "onPixelSizeAffineChanged() " << v0 << "-" << v1 << "-" << v2 << "-" << v3 << "-" << v4 << "-" << v5 << '\n';

        QString msg = QString("onPixelSizeAffineChanged() %1-%2-%3-%4-%5-%6").arg(v0).arg(v1).arg(v2).arg(v3).arg(v4).arg(v5);
        info(msg);

        emit pixelSizeAffineChanged(v0, v1, v2, v3, v4, v5);
    }
    virtual void onStagePositionChanged(const char* name, double pos)override
    {
        std::cout << "onStagePositionChanged()" << name << " " << pos << '\n';

        QString msg = QString("onStagePositionChanged() %1 %2").arg(name).arg(pos);
        info(msg);

        emit stagePositionChanged(name, pos);
    }
    virtual void onXYStagePositionChanged(const char* name, double xpos, double ypos)override
    {
        std::cout << "onXYStagePositionChanged()" << name << " " << xpos;
        std::cout << " " << ypos << '\n';

        QString msg = QString("onXYStagePositionChanged() %1 %2 %3").arg(name).arg(xpos).arg(ypos);
        info(msg);

        emit xyStagePositionChanged(name, xpos, ypos);
    }
    virtual void onExposureChanged(const char* name, double newExposure)override
    {
        std::cout << "onExposureChanged()" << name << " " << newExposure << '\n';

        QString msg = QString("onExposureChanged() %1 %2").arg(name).arg(newExposure);
        info(msg);

        emit exposureChanged(name, newExposure);
    }
    virtual void onSLMExposureChanged(const char* name, double newExposure)override
    {
        std::cout << "onSLMExposureChanged()" << name << " " << newExposure << '\n';
        QString msg = QString("onSLMExposureChanged() %1 %2").arg(name).arg(newExposure);
        info(msg);

        emit slmExposureChanged(name, newExposure);
    }
    virtual void onNewImageReady(const char* cameraLabel, int imageNumber)override
    {
        emit newImageReady(cameraLabel, imageNumber);
    }

};

#endif //_QMMCORE_H_