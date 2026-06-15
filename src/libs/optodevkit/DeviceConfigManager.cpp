
#include "DeviceConfigManager.h"

#include <QTreeWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>

#include <QSettings>

#include "MMCore/MMCore.h"
#include "MMCore/CoreUtils.h"

DeviceConfigManager::DeviceConfigManager(CMMCore* core, QWidget* parent)
    : QWidget(parent), mmCore_(core)
{
    setupUI();
    refreshDeviceLists();
}

void DeviceConfigManager::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 顶部按钮栏
    QHBoxLayout* topButtonLayout = new QHBoxLayout();
    btnLoadDefault_ = new QPushButton("加载默认配置", this);
    btnSaveConfig_ = new QPushButton("保存配置", this);
    btnLoadConfig_ = new QPushButton("加载配置", this);
    btnRefresh_ = new QPushButton("刷新", this);

    topButtonLayout->addWidget(btnLoadDefault_);
    topButtonLayout->addWidget(btnSaveConfig_);
    topButtonLayout->addWidget(btnLoadConfig_);
    topButtonLayout->addWidget(btnRefresh_);
    topButtonLayout->addStretch();

    // 主内容区域
    QHBoxLayout* contentLayout = new QHBoxLayout();

    // 左侧：可用设备面板
    QGroupBox* availableGroup = new QGroupBox("可用设备", this);
    QVBoxLayout* availableLayout = new QVBoxLayout(availableGroup);

    // 适配器库选择
    QHBoxLayout* libraryLayout = new QHBoxLayout();
    libraryLayout->addWidget(new QLabel("适配器库:", this));
    adapterLibraryCombo_ = new QComboBox(this);
    libraryLayout->addWidget(adapterLibraryCombo_);
    availableLayout->addLayout(libraryLayout);

    // 可用设备树
    availableDevicesTree_ = new QTreeWidget(this);
    availableDevicesTree_->setHeaderLabels(QStringList() << "设备" << "类型");
    availableDevicesTree_->setSelectionMode(QTreeWidget::SingleSelection);
    availableLayout->addWidget(availableDevicesTree_);

    // 添加设备按钮
    btnAddDevice_ = new QPushButton("添加设备", this);
    availableLayout->addWidget(btnAddDevice_);

    // 右侧：已加载设备面板
    QGroupBox* loadedGroup = new QGroupBox("已加载设备", this);
    QVBoxLayout* loadedLayout = new QVBoxLayout(loadedGroup);

    loadedDevicesList_ = new QListWidget(this);
    loadedLayout->addWidget(loadedDevicesList_);

    // 移除设备按钮
    btnRemoveDevice_ = new QPushButton("移除设备", this);
    loadedLayout->addWidget(btnRemoveDevice_);

    contentLayout->addWidget(availableGroup, 1);
    contentLayout->addWidget(loadedGroup, 1);

    // 状态栏
    statusLabel_ = new QLabel("就绪", this);
    statusLabel_->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    // 组装主界面
    mainLayout->addLayout(topButtonLayout);
    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(statusLabel_);

    // 连接信号槽
    connect(btnLoadDefault_, &QPushButton::clicked, this, &DeviceConfigManager::loadDefaultConfiguration);
    connect(btnSaveConfig_, &QPushButton::clicked, this, &DeviceConfigManager::saveConfiguration);
    connect(btnLoadConfig_, &QPushButton::clicked, this, &DeviceConfigManager::loadConfiguration);
    connect(btnRefresh_, &QPushButton::clicked, this, &DeviceConfigManager::refreshDeviceLists);
    connect(btnAddDevice_, &QPushButton::clicked, this, &DeviceConfigManager::addDevice);
    connect(btnRemoveDevice_, &QPushButton::clicked, this, &DeviceConfigManager::removeDevice);

    connect(adapterLibraryCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &DeviceConfigManager::populateAvailableDevices);

    connect(availableDevicesTree_, &QTreeWidget::itemSelectionChanged,
        this, &DeviceConfigManager::onAvailableDeviceSelectionChanged);

    connect(loadedDevicesList_, &QListWidget::itemSelectionChanged,
        this, &DeviceConfigManager::onLoadedDeviceSelectionChanged);
}

void DeviceConfigManager::refreshDeviceLists()
{
    try {
        // 刷新适配器库列表
        adapterLibraryCombo_->clear();
        std::vector<std::string> adapters = mmCore_->getDeviceAdapterNames();
        for (const auto& adapter : adapters) {
            adapterLibraryCombo_->addItem(QString::fromStdString(adapter));
        }

        populateAvailableDevices();
        populateLoadedDevices();

        statusLabel_->setText("设备列表已刷新");

    }
    catch (const CMMError& e) {
        QMessageBox::warning(this, "错误",
            QString("刷新设备列表失败: %1").arg(e.getMsg().c_str()));
        statusLabel_->setText("刷新失败");
    }
}

void DeviceConfigManager::populateAvailableDevices()
{
    availableDevicesTree_->clear();

    QString libraryName = adapterLibraryCombo_->currentText();
    if (libraryName.isEmpty()) return;

    try {
        std::vector<std::string> devices = mmCore_->getAvailableDevices(libraryName.toStdString().c_str());
        std::vector<long> types = mmCore_->getAvailableDeviceTypes(libraryName.toStdString().c_str());

        for (size_t i = 0; i < devices.size(); ++i) {
            QTreeWidgetItem* item = new QTreeWidgetItem(availableDevicesTree_);
            item->setText(0, QString::fromStdString(devices[i]));
            item->setText(1, QString::fromStdString(ToString(static_cast<MM::DeviceType>(types[i]))));

            // 存储设备信息
            item->setData(0, Qt::UserRole, libraryName);  // 适配器库
            item->setData(1, Qt::UserRole, QString::fromStdString(devices[i]));  // 设备名
        }

    }
    catch (const CMMError& e) {
        QMessageBox::warning(this, "错误",
            QString("获取可用设备失败: %1").arg(e.getMsg().c_str()));
    }
}

void DeviceConfigManager::populateLoadedDevices()
{
    loadedDevicesList_->clear();

    try {
        std::vector<std::string> devices = mmCore_->getLoadedDevices();

        for (const auto& deviceLabel : devices) {
            MM::DeviceType type = mmCore_->getDeviceType(deviceLabel.c_str());
            std::string library = mmCore_->getDeviceLibrary(deviceLabel.c_str());

            QString itemText = QString::fromStdString(deviceLabel) +
                " [" + QString::fromStdString(library) + " - " +
                QString::fromStdString(ToString(type)) + "]";

            QListWidgetItem* item = new QListWidgetItem(itemText, loadedDevicesList_);
            item->setData(Qt::UserRole, QString::fromStdString(deviceLabel));

            // 根据初始化状态设置颜色
            DeviceInitializationState state = mmCore_->getDeviceInitializationState(deviceLabel.c_str());
            switch (state) {
            case Uninitialized:
                item->setForeground(Qt::red);
                break;
            case InitializedSuccessfully:
                item->setForeground(Qt::darkGreen);
                break;
            case InitializationFailed:
                item->setForeground(Qt::red);
                break;
            }
        }

    }
    catch (const CMMError& e) {
        QMessageBox::warning(this, "错误",
            QString("获取已加载设备失败: %1").arg(e.getMsg().c_str()));
    }
}

void DeviceConfigManager::addDevice()
{
    QList<QTreeWidgetItem*> selectedItems = availableDevicesTree_->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一个可用设备");
        return;
    }

    QTreeWidgetItem* item = selectedItems.first();
    QString libraryName = item->data(0, Qt::UserRole).toString();
    QString deviceName = item->data(1, Qt::UserRole).toString();

    // 获取设备标签
    bool ok;
    QString deviceLabel = QInputDialog::getText(this, "添加设备",
        "请输入设备标签:", QLineEdit::Normal, deviceName, &ok);

    if (!ok || deviceLabel.isEmpty()) return;

    try {
        // 检查设备是否已存在
        std::vector<std::string> loadedDevices = mmCore_->getLoadedDevices();
        for (const auto& loaded : loadedDevices) {
            if (loaded == deviceLabel.toStdString()) {
                QMessageBox::warning(this, "错误", "设备标签已存在，请使用不同的标签");
                return;
            }
        }

        loadDevice(libraryName, deviceName, deviceLabel);

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, "错误",
            QString("添加设备失败: %1").arg(e.getMsg().c_str()));
        statusLabel_->setText("添加设备失败");
    }
}

void DeviceConfigManager::loadDevice(const QString& library, const QString& deviceName, const QString& label)
{
    try {
        // 加载设备
        mmCore_->loadDevice(label.toStdString().c_str(),
            library.toStdString().c_str(),
            deviceName.toStdString().c_str());

        // 自动初始化设备
        initializeDevice(label);

        refreshDeviceLists();
        statusLabel_->setText(QString("设备已添加: %1").arg(label));

    }
    catch (const CMMError& e) {
        throw; // 重新抛出异常
    }
}

void DeviceConfigManager::initializeDevice(const QString& label)
{
    try {
        mmCore_->initializeDevice(label.toStdString().c_str());

        // 检查初始化状态
        DeviceInitializationState state = mmCore_->getDeviceInitializationState(label.toStdString().c_str());
        if (state != InitializedSuccessfully) {
            QMessageBox::warning(this, "警告",
                QString("设备 %1 初始化可能有问题").arg(label));
        }

    }
    catch (const CMMError& e) {
        QMessageBox::warning(this, "初始化警告",
            QString("设备 %1 初始化时出现问题: %2").arg(label).arg(e.getMsg().c_str()));
    }
}

void DeviceConfigManager::removeDevice()
{
    QList<QListWidgetItem*> selectedItems = loadedDevicesList_->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一个已加载的设备");
        return;
    }

    QListWidgetItem* item = selectedItems.first();
    QString deviceLabel = item->data(Qt::UserRole).toString();

    int ret = QMessageBox::question(this, "确认移除",
        QString("确定要移除设备 '%1' 吗？").arg(deviceLabel),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    try {
        mmCore_->unloadDevice(deviceLabel.toStdString().c_str());

        refreshDeviceLists();
        statusLabel_->setText(QString("设备已移除: %1").arg(deviceLabel));

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, "错误",
            QString("移除设备失败: %1").arg(e.getMsg().c_str()));
        statusLabel_->setText("移除设备失败");
    }
}

void DeviceConfigManager::loadDefaultConfiguration()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择默认配置文件", "", "配置文件 (*.cfg)");

    if (fileName.isEmpty()) return;

    try {
        mmCore_->loadSystemConfiguration(fileName.toStdString().c_str());

        refreshDeviceLists();
        statusLabel_->setText("默认配置已加载");

        // 保存默认配置文件路径
        QSettings settings;
        settings.setValue("LastDefaultConfig", fileName);

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, "错误",
            QString("加载默认配置失败: %1").arg(e.getMsg().c_str()));
        statusLabel_->setText("加载配置失败");
    }
}

void DeviceConfigManager::saveConfiguration()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存配置", "MMConfig.cfg", "配置文件 (*.cfg)");

    if (fileName.isEmpty()) return;

    try {
        mmCore_->saveSystemConfiguration(fileName.toStdString().c_str());

        statusLabel_->setText(QString("配置已保存: %1").arg(fileName));

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, "错误",
            QString("保存配置失败: %1").arg(e.getMsg().c_str()));
        statusLabel_->setText("保存配置失败");
    }
}

void DeviceConfigManager::loadConfiguration()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "加载配置", "", "Micro-Manager配置文件 (*.cfg)");

    if (fileName.isEmpty()) return;

    try {
        // 先卸载所有现有设备
        mmCore_->unloadAllDevices();

        // 加载新配置
        mmCore_->loadSystemConfiguration(fileName.toStdString().c_str());

        refreshDeviceLists();
        statusLabel_->setText(QString("配置已加载: %1").arg(fileName));

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, "错误",
            QString("加载配置失败: %1").arg(e.getMsg().c_str()));
        statusLabel_->setText("加载配置失败");
    }
}

void DeviceConfigManager::onAvailableDeviceSelectionChanged()
{
    QList<QTreeWidgetItem*> selectedItems = availableDevicesTree_->selectedItems();
    btnAddDevice_->setEnabled(!selectedItems.isEmpty());
}

void DeviceConfigManager::onLoadedDeviceSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = loadedDevicesList_->selectedItems();
    btnRemoveDevice_->setEnabled(!selectedItems.isEmpty());
}

DeviceConfigManager::~DeviceConfigManager()
{
    // 清理资源
}