

// MMCorePropertyBrowser.h
#ifndef MMCOREPROPERTYBROWSER_H
#define MMCOREPROPERTYBROWSER_H

#include "HardConfigPanel.h"

#include <QWidget>
#include <QListWidget>
#include <QSplitter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QTreeWidget>
#include <QHeaderView>


// 前向声明 QtPropertyBrowser 相关类
class QtTreePropertyBrowser;
class QtVariantPropertyManager;
class QtVariantEditorFactory;
class QtProperty;
class QtBrowserItem;
class CMMCore;

class MMCorePropertyBrowser : public HardConfigPanel
{
    Q_OBJECT

public:
    explicit MMCorePropertyBrowser(CMMCore* core, QWidget* parent = nullptr);
    ~MMCorePropertyBrowser();
    void enableGui(bool );
public slots:
    void refreshDeviceList();
    void onDeviceSelectionChanged();
    void onPropertyValueChanged(QtProperty* property, const QVariant& value);
    void onPropertySelectionChanged(QtBrowserItem* selected);
private slots:
    void savePropertyState();
    void loadPropertyState();
    void resetCurrentProperties();
    void applyCurrentProperties();
    void expandAllProperties();
    void collapseAllProperties();

private:
    void setupUI();
    void populateProperties(const QString& deviceLabel);
    void clearProperties();
    void createPropertyForDevice(const QString& deviceLabel, const QString& propName);
    QtVariantPropertyManager* getVariantManager();
    QtVariantEditorFactory* getVariantFactory();

    CMMCore* mmCore_;
    bool enableGui_;

    // 设备列表
    QListWidget* deviceList_;

    // QtPropertyBrowser 组件
    QtTreePropertyBrowser* propertyBrowser_;
    QtVariantPropertyManager* variantManager_;
    QtVariantEditorFactory* variantFactory_;

    // 属性映射
    QMap<QtProperty*, QPair<QString, QString>> propertyMap_; // property -> (deviceLabel, propName)
    QMap<QString, QList<QtProperty*>> deviceProperties_; // deviceLabel -> properties

    // 当前选择的设备
    QString currentDevice_;

    // 工具栏按钮
    QPushButton* btnRefresh_;
    QPushButton* btnSaveState_;
    QPushButton* btnLoadState_;
    QPushButton* btnReset_;
    QPushButton* btnApply_;
    QPushButton* btnExpand_;
    QPushButton* btnCollapse_;

    QLabel* statusLabel_;
};

#endif

