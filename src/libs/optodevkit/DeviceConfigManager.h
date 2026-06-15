#ifndef DEVICECONFIGMANAGER_H
#define DEVICECONFIGMANAGER_H

#include <QWidget>


class QTreeWidget;
class QComboBox;
class QListWidget;
class QPushButton;
class QLabel;
class CMMCore;

class DeviceConfigManager  : public QWidget
{
	Q_OBJECT

public:
	explicit DeviceConfigManager(CMMCore* core, QWidget* parent = nullptr);
	~DeviceConfigManager();
public slots:
    void loadDefaultConfiguration();
    void saveConfiguration();
    void loadConfiguration();
    void refreshDeviceLists();

private slots:
    void addDevice();
    void removeDevice();
    void onAvailableDeviceSelectionChanged();
    void onLoadedDeviceSelectionChanged();

private:
    void setupUI();
    void populateAvailableDevices();
    void populateLoadedDevices();
    void loadDevice(const QString& library, const QString& deviceName, const QString& label);
    void initializeDevice(const QString& label);

    CMMCore* mmCore_;

    // 可用设备列表
    QTreeWidget* availableDevicesTree_;
    QComboBox* adapterLibraryCombo_;

    // 已加载设备列表
    QListWidget* loadedDevicesList_;

    // 按钮
    QPushButton* btnAddDevice_;
    QPushButton* btnRemoveDevice_;
    QPushButton* btnLoadDefault_;
    QPushButton* btnSaveConfig_;
    QPushButton* btnLoadConfig_;
    QPushButton* btnRefresh_;

    // 状态标签
    QLabel* statusLabel_;
};

#endif
