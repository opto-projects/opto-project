// MMCorePropertyBrowser.cpp
#include "MMCorePropertyBrowser.h"
#include <QSettings>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>

// 包含 QtPropertyBrowser 头文件
#include <qtpropertybrowser/src/qtpropertybrowser.h>
#include <qtpropertybrowser/src/qttreepropertybrowser.h>
#include <qtpropertybrowser/src/qtvariantproperty.h>
#include <qtpropertybrowser/src/qtgroupboxpropertybrowser.h>

#include "MMCore/MMCore.h"
#include "MMCore/CoreUtils.h"

MMCorePropertyBrowser::MMCorePropertyBrowser(CMMCore* core, QWidget* parent)
    : HardConfigPanel(parent), mmCore_(core), variantManager_(nullptr), variantFactory_(nullptr)
{
    setupUI();
}

void MMCorePropertyBrowser::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 顶部工具栏
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    btnRefresh_ = new QPushButton(tr("Refresh"), this);
    btnSaveState_ = new QPushButton(tr("SaveState"), this);
    btnLoadState_ = new QPushButton(tr("LoadState"), this);
    btnReset_ = new QPushButton(tr("Reset"), this);
    btnApply_ = new QPushButton(tr("Apply"), this);
    btnExpand_ = new QPushButton(tr("Expand"), this);
    btnCollapse_ = new QPushButton(tr("Collapse"), this);

    toolbarLayout->addWidget(btnRefresh_);
    toolbarLayout->addWidget(btnSaveState_);
    toolbarLayout->addWidget(btnLoadState_);
    toolbarLayout->addWidget(btnReset_);
    toolbarLayout->addWidget(btnApply_);
    toolbarLayout->addWidget(btnExpand_);
    toolbarLayout->addWidget(btnCollapse_);
    toolbarLayout->addStretch();

    // 主内容区域
    QHBoxLayout* contentLayout = new QHBoxLayout();

    // 左侧：设备列表
    QGroupBox* deviceGroup = new QGroupBox(tr("devices"), this);
    QVBoxLayout* deviceLayout = new QVBoxLayout(deviceGroup);

    deviceList_ = new QListWidget(this);
    deviceList_->setSelectionMode(QListWidget::SingleSelection);
    deviceLayout->addWidget(deviceList_);

    contentLayout->addWidget(deviceGroup, 1);

    // 右侧：属性浏览器
    QGroupBox* propertyGroup = new QGroupBox(tr("propertis"), this);
    QVBoxLayout* propertyLayout = new QVBoxLayout(propertyGroup);

    // 创建 QtPropertyBrowser 组件
    variantManager_ = new QtVariantPropertyManager(this);
    variantFactory_ = new QtVariantEditorFactory(this);
    propertyBrowser_ = new QtTreePropertyBrowser(this);
    propertyBrowser_->setFactoryForManager(variantManager_, variantFactory_);
    propertyBrowser_->setSectionResizeMode(QtTreePropertyBrowser::Interactive);
    propertyBrowser_->setPropertiesWithoutValueMarked(true);
    propertyBrowser_->setRootIsDecorated(true);

    propertyLayout->addWidget(propertyBrowser_);

    contentLayout->addWidget(propertyGroup, 3);

    // 状态栏
    statusLabel_ = new QLabel(tr("Ready"), this);
    statusLabel_->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    // 组装主界面
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(statusLabel_);

    // 连接信号槽
    connect(btnRefresh_, &QPushButton::clicked, this, &MMCorePropertyBrowser::refreshDeviceList);
    connect(btnSaveState_, &QPushButton::clicked, this, &MMCorePropertyBrowser::savePropertyState);
    connect(btnLoadState_, &QPushButton::clicked, this, &MMCorePropertyBrowser::loadPropertyState);
    connect(btnReset_, &QPushButton::clicked, this, &MMCorePropertyBrowser::resetCurrentProperties);
    connect(btnApply_, &QPushButton::clicked, this, &MMCorePropertyBrowser::applyCurrentProperties);
    connect(btnExpand_, &QPushButton::clicked, this, &MMCorePropertyBrowser::expandAllProperties);
    connect(btnCollapse_, &QPushButton::clicked, this, &MMCorePropertyBrowser::collapseAllProperties);

    connect(deviceList_, &QListWidget::itemSelectionChanged,
        this, &MMCorePropertyBrowser::onDeviceSelectionChanged);

    connect(variantManager_, &QtVariantPropertyManager::valueChanged,
        this, &MMCorePropertyBrowser::onPropertyValueChanged);

    connect(propertyBrowser_, &QtTreePropertyBrowser::currentItemChanged,
        this, &MMCorePropertyBrowser::onPropertySelectionChanged);

    deviceGroup->hide();
    btnRefresh_->hide();
    btnSaveState_->hide();
    btnLoadState_->hide();
    btnReset_->hide();
    btnApply_->hide();
    btnExpand_->hide();
    btnCollapse_->hide();
}

void MMCorePropertyBrowser::refreshDeviceList()
{
    deviceList_->clear();

    try {
        std::vector<std::string> devices = mmCore_->getLoadedDevices();

        for (const auto& deviceLabel : devices) {

            //std::vector<std::string> properties = mmCore_->getDevicePropertyNames(deviceLabel.c_str());

            MM::DeviceType type = mmCore_->getDeviceType(deviceLabel.c_str());
            std::string library = mmCore_->getDeviceLibrary(deviceLabel.c_str());

            QString itemText = QString::fromStdString(deviceLabel) +
                " [" + QString::fromStdString(ToString(type)) + "]";

            QListWidgetItem* item = new QListWidgetItem(itemText, deviceList_);
            item->setData(Qt::UserRole, QString::fromStdString(deviceLabel));

            if (type == MM::DeviceType::CoreDevice)
            {
                continue;
            }

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

        if (std::find(devices.begin(), devices.end(), "Camera") != devices.end()) {
            populateProperties("Camera");
        }
        

        statusLabel_->setText(tr("Devices Refreshed"));
    }
    catch (const CMMError& e) {
        QMessageBox::warning(this, tr("Error"),
            QString(tr("get device list failed: %1")).arg(e.getMsg().c_str()));
        statusLabel_->setText(tr("Refresh failed"));
    }
}

void MMCorePropertyBrowser::onDeviceSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = deviceList_->selectedItems();
    if (selectedItems.isEmpty()) {
        clearProperties();
        currentDevice_.clear();
        return;
    }

    QListWidgetItem* item = selectedItems.first();
    QString deviceLabel = item->data(Qt::UserRole).toString();
    currentDevice_ = deviceLabel;
    populateProperties(deviceLabel);
}

void MMCorePropertyBrowser::populateProperties(const QString& deviceLabel)
{
    clearProperties();

    try {
        std::vector<std::string> properties =
            mmCore_->getDevicePropertyNames(deviceLabel.toStdString().c_str());

        // 创建设备属性组
        QtVariantProperty* deviceGroup = variantManager_->addProperty(
            QtVariantPropertyManager::groupTypeId(),
            tr("Devices :") + QString(" %1").arg(deviceLabel));

        propertyBrowser_->addProperty(deviceGroup);

        for (const auto& propName : properties) {
            createPropertyForDevice(deviceLabel, QString::fromStdString(propName));
        }

        statusLabel_->setText(QString(tr("Loaded Divice Property: %1")).arg(deviceLabel));

    }
    catch (const CMMError& e) {
        QMessageBox::warning(this, tr("Error"),
            QString(tr("get device property failed: %1")).arg(e.getMsg().c_str()));
        statusLabel_->setText(tr("load device property failed"));
    }
}

void MMCorePropertyBrowser::createPropertyForDevice(const QString& deviceLabel, const QString& propName)
{
    try {
        // 获取属性基本信息
        std::string currentValue = mmCore_->getProperty(
            deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

        MM::PropertyType propType = mmCore_->getPropertyType(
            deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

        bool readOnly = mmCore_->isPropertyReadOnly(
            deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

        std::vector<std::string> allowedValues =
            mmCore_->getAllowedPropertyValues(
                deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

        bool hasLimits = mmCore_->hasPropertyLimits(
            deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

        // 创建属性
        QtVariantProperty* property = nullptr;

        if (!allowedValues.empty()) {
            // 枚举类型 - 使用枚举编辑器
            property = variantManager_->addProperty(QtVariantPropertyManager::enumTypeId(), propName);

            QStringList enumNames;
            for (const auto& value : allowedValues) {
                enumNames << QString::fromStdString(value);
            }

            property->setAttribute("enumNames", enumNames);

            // 设置当前值
            int index = enumNames.indexOf(QString::fromStdString(currentValue));
            if (index >= 0) {
                property->setValue(index);
            }
            else {
                property->setValue(0);
            }

        }
        else {
            // 根据属性类型创建相应的属性编辑器
            switch (propType) {
            case MM::Integer: {
                property = variantManager_->addProperty(QVariant::Int, propName);

                bool ok;
                int intValue = QString::fromStdString(currentValue).toInt(&ok);
                if (ok) {
                    property->setValue(intValue);
                }
                else {
                    property->setValue(QString::fromStdString(currentValue));
                }

                // 设置范围限制
                if (hasLimits) {
                    double lower = mmCore_->getPropertyLowerLimit(
                        deviceLabel.toStdString().c_str(), propName.toStdString().c_str());
                    double upper = mmCore_->getPropertyUpperLimit(
                        deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

                    property->setAttribute("minimum", static_cast<int>(lower));
                    property->setAttribute("maximum", static_cast<int>(upper));
                }
                break;
            }

            case MM::Float: {
                property = variantManager_->addProperty(QVariant::Double, propName);

                bool ok;
                double doubleValue = QString::fromStdString(currentValue).toDouble(&ok);
                if (ok) {
                    property->setValue(doubleValue);
                }
                else {
                    property->setValue(QString::fromStdString(currentValue));
                }

                // 设置范围限制
                if (hasLimits) {
                    double lower = mmCore_->getPropertyLowerLimit(
                        deviceLabel.toStdString().c_str(), propName.toStdString().c_str());
                    double upper = mmCore_->getPropertyUpperLimit(
                        deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

                    property->setAttribute("minimum", lower);
                    property->setAttribute("maximum", upper);
                    property->setAttribute("decimals", 6);
                }
                break;
            }

            case MM::String:
            default: {
                property = variantManager_->addProperty(QVariant::String, propName);
                property->setValue(QString::fromStdString(currentValue));
                break;
            }
            }
        }

        if (property) {

            // 设置只读状态
            property->setEnabled(!readOnly);

            if (!readOnly )
            {
                if (!propName.contains("Exposure") && !propName.contains("Gain"))
                {
                    property->setEnabled(enableGui_);
                }
                
            }
            

            // 设置工具提示
            QString toolTip = QString(tr("Device: %1\nProperty: %2\nType: %3\nReadOnly: %4"))
                .arg(deviceLabel)
                .arg(propName)
                .arg(QString::fromStdString(ToString(propType)))
                .arg(readOnly ? "Yes" : "No");

            if (hasLimits) {
                double lower = mmCore_->getPropertyLowerLimit(
                    deviceLabel.toStdString().c_str(), propName.toStdString().c_str());
                double upper = mmCore_->getPropertyUpperLimit(
                    deviceLabel.toStdString().c_str(), propName.toStdString().c_str());
                toolTip += QString(tr("\nRange: %1 - %2")).arg(lower).arg(upper);
            }

            if (!allowedValues.empty()) {
                toolTip += QString(tr("\nAllowedValuesSize: %1 ")).arg(allowedValues.size());
            }

            property->setToolTip(toolTip);

            // 添加到属性浏览器
            QtVariantProperty* root = dynamic_cast<QtVariantProperty*>(propertyBrowser_->properties().first());
            if (root) {
                root->addSubProperty(property);
            }

            // 存储属性映射
            propertyMap_[property] = qMakePair(deviceLabel, propName);

            if (!deviceProperties_.contains(deviceLabel)) {
                deviceProperties_[deviceLabel] = QList<QtProperty*>();
            }
            deviceProperties_[deviceLabel].append(property);
        }

    }
    catch (const CMMError& e) {
        qWarning() << "Failed to create property" << propName << "for device" << deviceLabel
            << ":" << e.getMsg().c_str();
    }
}

void MMCorePropertyBrowser::onPropertyValueChanged(QtProperty* property, const QVariant& value)
{
    if (!propertyMap_.contains(property)) {
        return;
    }

    auto deviceProp = propertyMap_[property];
    QString deviceLabel = deviceProp.first;
    QString propName = deviceProp.second;

    try {
        // 获取属性类型和允许值
        MM::PropertyType propType = mmCore_->getPropertyType(
            deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

        std::vector<std::string> allowedValues = mmCore_->getAllowedPropertyValues(
            deviceLabel.toStdString().c_str(), propName.toStdString().c_str());

        QString newValue;

        if (!allowedValues.empty()) {
            // 枚举类型 - 使用索引获取实际值
            int index = value.toInt();
            if (index >= 0 && index < static_cast<int>(allowedValues.size())) {
                newValue = QString::fromStdString(allowedValues[index]);
            }
            else {
                QMessageBox::warning(this, "Error", "The selected enumeration value is invalid.");
                return;
            }
        }
        else {
            // 根据属性类型转换值
            switch (propType) {
            case MM::Integer:
                newValue = QString::number(value.toInt());
                break;
            case MM::Float:
                newValue = QString::number(value.toDouble());
                break;
            case MM::String:
            default:
                newValue = value.toString();
                break;
            }
        }

        if (!newValue.isEmpty()) {
            mmCore_->setProperty(deviceLabel.toStdString().c_str(),
                propName.toStdString().c_str(),
                newValue.toStdString().c_str());

            statusLabel_->setText(QString(tr("Property Refreshed: %1.%2 = %3"))
                .arg(deviceLabel).arg(propName).arg(newValue));
        }

    }
    catch (const CMMError& e) {
        QMessageBox::warning(this, "Error",
            QString(tr("Property setting failed: %1")).arg(e.getMsg().c_str()));

        // 恢复原始值
        populateProperties(deviceLabel);
        statusLabel_->setText("Property setting failed");
    }
}

void MMCorePropertyBrowser::clearProperties()
{
    // 清空属性浏览器
    QList<QtProperty*> properties = propertyBrowser_->properties();
    for (QtProperty* prop : properties) {
        propertyBrowser_->removeProperty(prop);
    }

    propertyMap_.clear();
    deviceProperties_.clear();
}

QtVariantPropertyManager* MMCorePropertyBrowser::getVariantManager()
{
    return variantManager_;
}

QtVariantEditorFactory* MMCorePropertyBrowser::getVariantFactory()
{
    return variantFactory_;
}
void MMCorePropertyBrowser::savePropertyState()
{
    if (currentDevice_.isEmpty()) {
        QMessageBox::information(this, tr("Note"), tr("Please select a device first."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        "Save propertis",
        QString("%1_properties.ini").arg(currentDevice_),
        "INI File (*.ini)");

    if (fileName.isEmpty()) return;

    try {
        QSettings settings(fileName, QSettings::IniFormat);
        settings.beginGroup(currentDevice_);

        std::vector<std::string> properties =
            mmCore_->getDevicePropertyNames(currentDevice_.toStdString().c_str());

        for (const auto& propName : properties) {
            std::string propValue = mmCore_->getProperty(
                currentDevice_.toStdString().c_str(), propName.c_str());

            settings.setValue(QString::fromStdString(propName),
                QString::fromStdString(propValue));
        }

        settings.endGroup();

        statusLabel_->setText(QString(tr("Propertis has been saved: %1")).arg(fileName));

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, tr("Error"),
            QString(tr("Save propertis failed: %1")).arg(e.getMsg().c_str()));
        statusLabel_->setText(tr("Save failed"));
    }
}

void MMCorePropertyBrowser::loadPropertyState()
{
    if (currentDevice_.isEmpty()) {
        QMessageBox::information(this, tr("Note"), tr("Please select a device first."));
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load propertis"), "", "INI File (*.ini)");

    if (fileName.isEmpty()) return;

    try {
        QSettings settings(fileName, QSettings::IniFormat);
        settings.beginGroup(currentDevice_);

        QStringList keys = settings.allKeys();
        for (const QString& key : keys) {
            QString value = settings.value(key).toString();

            mmCore_->setProperty(currentDevice_.toStdString().c_str(),
                key.toStdString().c_str(),
                value.toStdString().c_str());
        }

        settings.endGroup();

        // 刷新属性显示
        populateProperties(currentDevice_);
        statusLabel_->setText(QString(tr("Propertis loaded: %1")).arg(fileName));

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, tr("Error"),
            QString(tr("Load propertis failed: %1")).arg(e.getMsg().c_str()));
        statusLabel_->setText(tr("Load failed"));
    }
}

void MMCorePropertyBrowser::resetCurrentProperties()
{
    if (currentDevice_.isEmpty()) {
        QMessageBox::information(this, tr("Note"), tr("Please select a device first."));
        return;
    }

    int ret = QMessageBox::question(this, tr("Confirm reset"),
        QString(tr("Confirm reset : '%1' ")).arg(currentDevice_),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    try {
        // 重新加载属性
        populateProperties(currentDevice_);

        statusLabel_->setText(QString(tr("resetted: %1")).arg(currentDevice_));

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this, tr("Error"),
            QString(tr("Failed to reset the propertiy: %1")).arg(e.getMsg().c_str()));
        statusLabel_->setText(tr("reset failed"));
    }
}

void MMCorePropertyBrowser::applyCurrentProperties()
{
    if (currentDevice_.isEmpty()) {
        QMessageBox::information(this, tr("Note"), tr("Please select a device first."));
        return;
    }

    try {
        // 确保所有属性值都已应用
        // QtPropertyBrowser 会在值改变时立即应用，所以这里主要是确认

        statusLabel_->setText(QString("Property setted: %1").arg(currentDevice_));

    }
    catch (const CMMError& e) {
        QMessageBox::critical(this,tr( "Error"),
            QString("apply property failed: %1").arg(e.getMsg().c_str()));
        statusLabel_->setText(tr("apply failed"));
    }
}
void MMCorePropertyBrowser::expandAllProperties()
{
    // 广度优先遍历展开所有项
    QList<QtBrowserItem*> queue = propertyBrowser_->topLevelItems();

    while (!queue.isEmpty()) {
        QtBrowserItem* item = queue.takeFirst();
        if (item) {
            propertyBrowser_->setExpanded(item, true);
            queue.append(item->children());
        }
    }

    statusLabel_->setText(tr("expanded all properties"));
}

void MMCorePropertyBrowser::collapseAllProperties()
{
    // 广度优先遍历折叠所有项
    QList<QtBrowserItem*> queue = propertyBrowser_->topLevelItems();

    while (!queue.isEmpty()) {
        QtBrowserItem* item = queue.takeFirst();
        if (item) {
            propertyBrowser_->setExpanded(item, false);
            queue.append(item->children());
        }
    }

    statusLabel_->setText(tr("collapsed all properties "));
}

// 在实现文件中添加
void MMCorePropertyBrowser::onPropertySelectionChanged(QtBrowserItem* selected)
{

    // 获取选中的模型索引
    /*QModelIndexList selectedIndexes = selected.indexes();

    if (!selectedIndexes.isEmpty()) {
        QModelIndex selectedIndex = selectedIndexes.first();*/

        // 通过模型索引获取浏览器项
    QtBrowserItem *item = selected;/*propertyBrowser_->currentItem()*/;
        if (item) {
            QtProperty* property = item->property();
            bool isExpanded = propertyBrowser_->isExpanded(item);

            QString propertyName = property->propertyName();
            statusLabel_->setText(QString(tr("Property: '%1' (%2)"))
                .arg(propertyName)
                .arg(isExpanded ? tr("Expanded") : tr("Collapsed")));
        }
    //}
}

MMCorePropertyBrowser::~MMCorePropertyBrowser()
{
    // 清理资源
    clearProperties();
}

void MMCorePropertyBrowser::enableGui(bool enable)
{
    enableGui_ = enable;
    refreshDeviceList();
}

// 在 createPropertyForDevice 方法中添加对特定类型的特殊处理
//void MMCorePropertyBrowser::createPropertyForDevice(const QString& deviceLabel, const QString& propName)
//{
//    try {
//        // ... 前面的代码保持不变 ...
//
//        // 特殊处理：检测布尔类型（通过属性名或值模式）
//        if (currentValue == "0" || currentValue == "1" ||
//            propName.contains("Enable", Qt::CaseInsensitive) ||
//            propName.contains("Disable", Qt::CaseInsensitive) ||
//            propName.contains("On", Qt::CaseInsensitive) ||
//            propName.contains("Off", Qt::CaseInsensitive)) {
//
//            // 如果有明确的允许值，使用枚举
//            if (!allowedValues.empty()) {
//                // 使用枚举处理
//            }
//            else {
//                // 如果没有允许值，但看起来像布尔值，创建布尔属性
//                property = variantManager_->addProperty(QVariant::Bool, propName);
//                bool boolValue = (currentValue == "1" ||
//                    currentValue == "On" ||
//                    currentValue == "Enable" ||
//                    currentValue == "True");
//                property->setValue(boolValue);
//            }
//        }
//
//        // 特殊处理：检测命令属性（通常没有值，只有允许值）
//        if (allowedValues.empty() && propType == MM::String &&
//            (propName.contains("Command", Qt::CaseInsensitive) ||
//                propName.contains("Action", Qt::CaseInsensitive))) {
//
//            // 创建字符串属性，但标记为命令
//            property = variantManager_->addProperty(QVariant::String, propName);
//            property->setValue(QString::fromStdString(currentValue));
//        }
//
//        // ... 其余代码保持不变 ...
//
//    }
//    catch (const CMMError& e) {
//        qWarning() << "Failed to create property" << propName << "for device" << deviceLabel
//            << ":" << e.getMsg().c_str();
//    }
//}