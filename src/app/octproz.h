

#ifndef OCTPROZ_H
#define OCTPROZ_H

#ifdef _WIN32
	#define WINDOWS_LEAN_AND_MEAN
	//#define NOMINMAX
	#include <windows.h>

#endif
#ifdef WITH_CUDA
	#include "GL/glew.h"
#endif // WITH_CUDA

#include <QMainWindow>
#include <QThread>
#include <QVariantMap>
#include <qdir.h>
#include <qpluginloader.h>
#include "sidebar.h"
#include "messageconsole.h"
#include "plotwindow1d.h"
#include "glwindow2d.h"
#include "glwindow3d.h"
#include "systemmanager.h"
#include "systemchooser.h"
#include "extensionmanager.h"
#include "extensioneventfilter.h"
#include "processing.h"
#include "octproz_devkit.h"
#include "octalgorithmparameters.h"
#include "octalgorithmparametersmanager.h"
#include "aboutdialog.h"

#include "ui_octproz.h"

#define APP_VERSION "1.7.0"
#define APP_VERSION_DATE "2 December 2024"
#define APP_NAME "OptoChecker"

#define MAIN_WINDOW_SETTINGS_GROUP "main_window_settings"
#define MAIN_GEOMETRY "main_geometry"
#define MAIN_STATE "main_state"
#define MAIN_ACTIVE_SYSTEM "main_active_system"
#define MESSAGE_CONSOLE_BOTTOM "message_console_bottom"
#define MESSAGE_CONSOLE_HEIGHT "message_console_height"
#define DOCK_BSCAN_VISIBLE "dock_bscan_visible"
#define DOCK_BSCAN_GEOMETRY "dock_bscan_geometry"
#define DOCK_ENFACEVIEW_VISIBLE "dock_enfaceview_visible"
#define DOCK_ENFACEVIEW_GEOMETRY "dock_enfaceview_geometry"
#define DOCK_VOLUME_VISIBLE "dock_volume_visible"
#define DOCK_VOLUME_GEOMETRY "dock_volume_geometry"





namespace Ui {
class OCTproZ;
}

class OCTproZ : public QMainWindow
{
	Q_OBJECT
	QThread acquisitionThread;
	QThread processingThread;
	QThread notifierThread;

public:
	explicit OCTproZ(QWidget* parent = 0);
	~OCTproZ();

	void closeEvent(QCloseEvent* event);

	void initMenu();
	void initGui();
	void prepareDockWidget(QDockWidget*& dock, QWidget* widgetForDock,  QAction*& action, const QIcon& icon, QString iconText);
	void initActionsAndDocks();
	void loadSystemsAndExtensions();
	void initExtensionsMenu();

public slots:
	void slot_start();
	void slot_stop();
	void slot_record();
	void slot_selectSystem();
	void slot_selectAndLoadSettingsFile();
	void slot_selectAndSaveSettingsToFile();
	void slot_loadSettingsFromFile(QString filePath);
	void slot_saveSettingsToFile(QString filePath);
	void slot_menuUserManual();
	void slot_menuAbout();
	void slot_menuApplicationSettings();
	void slot_menuSystemSettings();
	void slot_menuExtensions();
	void slot_uncheckExtensionInMenu(Extension* extension);
	void slot_enableStopAction();
	void slot_updateAcquistionParameter(AcquisitionParams newParams);
	void slot_closeOpenGLwindows();
	void slot_reopenOpenGLwindows();
	void slot_storePluginSettings(QString pluginName, QVariantMap settings);
	void slot_prepareGpu2HostForProcessedRecording();
	void slot_resetGpu2HostSettings();
	void slot_recordingDone();
	void slot_enableEnFaceViewProcessing(bool enable);
	void slot_enableBscanViewProcessing(bool enable);
	void slot_enableVolumeViewProcessing(bool enable);
	void slot_easterEgg();
	void slot_useCustomResamplingCurve(bool use);
	void slot_loadCustomResamplingCurve();
	void slot_setKLinCoeffs(double* k0, double* k1, double* k2, double* k3);
	void slot_setCustomResamplingCurve(QVector<float> resamplingCurve);


private:
	void setSystem(QString systemName);
	void activateSystem(AcquisitionSystem* system);
	void deactivateSystem(AcquisitionSystem* system);
	void reactivateSystem(AcquisitionSystem* system);
	void forceUpdateProcessingParams();
	void loadMainWindowSettings();
	void saveMainWindowSettings();
	void loadSettings();
	void loadSettingsFromFile(const QString &settingsFilePath);
	void saveSettings();
	void updateSettingsMap();
	void loadResamplingCurveFromFile(QString fileName);

	Ui::OCTproZ *ui;
	QVariantMap mainWindowSettings;
	AboutDialog* aboutWindow;
	Sidebar* sidebar;
	SystemManager* sysManager;
	SystemChooser* sysChooser;
	AcquisitionSystem* currSystem;
	ExtensionManager* extManager;
	Processing* signalProcessing; 
	OctAlgorithmParameters* octParams;
	OctAlgorithmParametersManager* paramsManager;

#ifdef WITH_CUDA
	Gpu2HostNotifier* processedDataNotifier;
#endif // WITH_CUDA

	bool processingInThread;
	bool streamToHostMemorized;
	unsigned int streamingBuffersToSkipMemorized;

	QString currSystemName;
	QList<QString> activatedSystems;
	QList<QAction*> extensionActions;

	QMenu* extrasMenu;
	QToolBar* viewToolBar;
	QToolBar* view2DExtrasToolBar;
	QToolBar* controlToolBar;

	QAction* actionStart;
	QAction* actionStop;
	QAction* actionRecord;
	QAction* actionSelectSystem;
	QAction* actionSystemSettings;
	QAction* action1D;
	QAction* action2D;
	QAction* actionEnFaceView;
	QAction* action3D;
	QAction* actionConsole;
	QAction* actionUseSidebarKLinCurve;
	QAction* actionUseCustomKLinCurve;
	QAction* actionSetCustomKLinCurve;

	MessageConsole* console;

	QDockWidget* dockConsole;
	QDockWidget* dock1D;
	QDockWidget* dock2D;
	QDockWidget* dockEnFaceView;
	QDockWidget* dockVolumeView;

	GLWindow3D* volumeWindow;
	GLWindow2D* bscanWindow;
	GLWindow2D* enFaceViewWindow;
	PlotWindow1D* plot1D;

	bool isDock2DClosed;
	bool isDockEnFaceViewClosed;
	bool isDockVolumeViewClosed;

	QOpenGLContext *context;

	int rerunCounter = 0;

signals:
	void start();
	void stop();
	void allowRawGrabbing(bool allowed);
	void record();
	void enableRecording(RecordingParams recParams);
	void pluginSettingsRequest();
	void newSystemSelected();
	void newSystem(AcquisitionSystem*);
	void error(QString);
	void info(QString);
	void glBufferTextureSizeBscan(unsigned int width, unsigned int height, unsigned int depth);
	void glBufferTextureSizeEnFaceView(unsigned int width, unsigned int height, unsigned int depth);
	void linesPerBufferChanged(int linesPerBuffer);
	void closeDock2D();
	void reopenDock2D();
	void loadPluginSettings(QVariantMap);


};

#endif // OCTPROZ_H
