#ifndef PLOT_WINDOW_H
#define PLOT_WINDOW_H

#include <QMainWindow>
#include "systemmanager.h"
#include "extensionmanager.h"
#include "systemchooser.h"
#include "optodevkit/devkit.h"
#include "aboutdialog.h"
#include "./ADS/src/DockManager.h"


#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include <QStatusBar>
#include <QTreeWidget>
#include <QTableWidget>

class ImageDisplay;
class MessageConsole;
class MeasureProgressBar;
class StatusBar;
class MeasureSaver;
class HardConfigPanel;

#define MAIN_WINDOW_SETTINGS_GROUP "main_window_settings"
#define MAIN_GEOMETRY "main_geometry"
#define MAIN_STATE "main_state"
#define MAIN_DOCK_STATE "main_dock_state"
class PlotWindow : public QMainWindow
{
	Q_OBJECT

public:
	PlotWindow(QWidget* parent = nullptr);
	~PlotWindow();


	void stopCapture();
signals:
	void info(QString info);
	void error(QString error);

	void start();
	void stop();
	void loadPluginSettings(QVariantMap);
	void pluginSettingsRequest();

protected:
	void closeEvent(QCloseEvent* event);
	bool event(QEvent* event) override;
private:
	ImageDisplay* _plot2d;
	ads::CDockManager* _dockManager = nullptr;

	QMenu* fileMenu, * viewMenu, * toolsMenu, *helpMenu;
	QToolBar* viewToolBar,* controlToolBar;
	QAction* actionStart, * actionStop, * actionMeasure,* actionWelcome,
		 * actionSaveRaw,
		* actionTemp, * actionSelectSystem, * actionSystemSettings,
		* actionMessageConsole, *actionResultsPanel, *actionHardConfig;

	MeasureProgressBar* _measureProgress ;
	QTreeWidget* _tree;
	StatusBar* _statusBar;

	AboutDialog* aboutWindow;
	MessageConsole* console;
	HardConfigPanel*hardConfigPanel,* stubConfigPanel;
	QTableWidget* resultPanel;

	TableIntf* _tableIntf;
	PlotIntf* _plotIntf;

	QVariantMap mainWindowSettings;

	QThread acquisitionThread;
	QThread processingThread;
	QThread notifierThread;

	SystemManager* sysManager;
	SystemChooser* sysChooser;
	QSharedPointer<AcquisitionSystem> currSystem;
	QSharedPointer<MeasureSaver> _saver;
	ExtensionManager* extManager;
	QString currSystemName;
	QList<QAction*> systemActions;
	QList<QAction*> extensionActions;
	void loadSystemsAndExtensions();
	void initExtensionsMenu();
	void setSystem(QString systemName);
	void activateSystem(AcquisitionSystem* system);
	void deactivateSystem();

	void createMenuBar();
	void createPlot();
	void createContent();
	void createStatusBar();

	ads::CDockWidget* messageConsoleDockWidget, *hardConfigDockWidget, * propDockWidget, * resultDockWidget;
	void createMessageConsoleDockWidget();
	void createResultsPanelDockWidget();
	void createHardConfigDockWidget();
	void createSystemSettingsDockWidget();
	ads::CDockWidget* prepareDockWidget(QWidget* widgetForDock, QString title,ads::DockWidgetArea area, QAction*& action, const QIcon& icon, QString iconText);

	void loadSettings();
	void saveSettings();
	void loadMainWindowSettings();
	void saveMainWindowSettings();
	void loadSettingsFromFile(const QString& settingsFilePath);
	void updateHardConfigDockWidget();

	void updateControls();
	void showFps(double fps, double hardFps);
public slots:
	void slot_start();
	void slot_stop();
	void slot_selectSystem();
	void slot_menuSystemSettings();
	void slot_menuUserManual();
	void slot_menuAbout();
	void slot_measurements();
	void slot_selectAndLoadSettingsFile();
	void slot_selectAndSaveSettingsToFile();
	void slot_loadSettingsFromFile(QString filePath);
	void slot_saveSettingsToFile(QString filePath);
	void slot_storePluginSettings(QString pluginName, QVariantMap settings);	
	void slot_menuExtensions();
	void slot_uncheckExtensionInMenu(Extension* extension);

	void statsReceived(const CameraStats& stats);
	void dataReady();
};

#endif // PLOT_WINDOW_H