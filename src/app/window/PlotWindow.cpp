#include "PlotWindow.h"
#include "QtPropertyEditor.h"
#include "extensioneventfilter.h"
#include "messageconsole.h"

#include "optodevkit/HardConfigPanel.h"
#include "optodevkit/CameraWorker.h"
#include "optodevkit/settings.h"
#include "optodevkit/qmmcore.h"
#include "optodevkit/util/OriStatusBar.h"

#include "MMCore/MMEventCallback.h"
#include "ImageView/imagedisplay.h"

#include "PlotIntf.h"
#include "TableIntf.h"

#include <QMenuBar>
#include <QProgressBar>
#include <QToolTip>
#include <QDesktopServices>
#include <QLabel>
#include <QGuiApplication>
#include <QScreen>
#include <QPluginLoader>
#include <QFileDialog>

enum StatusPanels
{
	STATUS_CAMERA,
	STATUS_SEPARATOR_1,
	STATUS_RESOLUTION,
	STATUS_ROI_ICON,
	STATUS_ROI,
	STATUS_SEPARATOR_2,
	STATUS_FPS,
	STATUS_SEPARATOR_3,
	STATUS_NO_DATA,
	STATUS_BGND,
	STATUS_SEPARATOR_4,
	STATUS_MOUSE_POS,

	STATUS_PANEL_COUNT,
};

static QIcon svgIcon(const QString& File)
{

	// This is a workaround, because in item views SVG icons are not
	// properly scaled and look blurry or pixelate
	QIcon SvgIcon(File);
	SvgIcon.addPixmap(SvgIcon.pixmap(92));
	return SvgIcon;

}

class MeasureProgressBar : public QProgressBar
{
public:
	void reset(int duration, const QString& fileName)
	{
		setElapsed(0);
		setMaximum(duration);
		setVisible(true);
		setFormat("%p%");
		_fileName = fileName;
	}

	void setElapsed(qint64 ms) {
		_secs = ms / 1000;
		setValue(_secs);
		if (value() >= maximum()) {
			setFormat(tr("Finishing..."));
		}
	}

protected:
	bool event(QEvent* e) override {
		if (e->type() != QEvent::ToolTip)
			return QProgressBar::event(e);
		if (auto he = dynamic_cast<QHelpEvent*>(e); he)
			QToolTip::showText(he->globalPos(), formatTooltip());
		return true;
	}

	void contextMenuEvent(QContextMenuEvent* e) override {
		if (!_contextMenu) {
			_contextMenu = new QMenu(this);
			_contextMenu->addAction(tr("Open file location"), this, &MeasureProgressBar::openFileLocation);
		}
		_contextMenu->popup(e->globalPos());
	}

private:
	int _secs;
	QString _fileName;
	QMenu* _contextMenu = nullptr;

	QString formatTooltip() const {
		int max = maximum();
		if (max == 0)
			return tr("Measurements"
				"<br>Elapsed: <b>%1</b>"
				"<br>File: <b>%2</b>")
			.arg(formatSecs(_secs), _fileName);
		return tr("Measurements"
			"<br>Duration: <b>%1</b>"
			"<br>Elapsed: <b>%2</b>"
			"<br>Remaining: <b>%3</b>"
			"<br>File: <b>%4</b>")
			.arg(formatSecs(max), formatSecs(_secs), formatSecs(max - _secs), _fileName);
	}

	void openFileLocation() {
		QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(_fileName).dir().absolutePath()));
	}
};

PlotWindow::PlotWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setObjectName("PlotWindow");
	//ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
	ads::CDockManager::setAutoHideConfigFlags({ ads::CDockManager::DefaultAutoHideConfig });
	_dockManager = new ads::CDockManager(this);
	_dockManager->setDockWidgetToolBarStyle(Qt::ToolButtonIconOnly, ads::CDockWidget::StateFloating);

	this->aboutWindow = new AboutDialog(this);

	this->sysManager = new SystemManager();
	this->sysChooser = new SystemChooser();
	this->currSystem = nullptr;
	this->currSystemName = "";
	this->extManager = new ExtensionManager();

	createMenuBar();
    createContent();
	createStatusBar();

	Settings* settings = Settings::getInstance();
	connect(settings, &Settings::info, this, &PlotWindow::info);
	connect(settings, &Settings::error, this, &PlotWindow::error);
	connect(this, &PlotWindow::info, console, &MessageConsole::displayInfo);
	connect(this, &PlotWindow::error, console, &MessageConsole::displayError);

	this->loadSettings();

	this->loadSystemsAndExtensions();

	// Default window geometry - center on screen
	resize(1280, 640);
	setGeometry(QStyle::alignedRect(
		Qt::LeftToRight, Qt::AlignCenter, frameSize(),
		QGuiApplication::primaryScreen()->availableGeometry()
	));
}

PlotWindow::~PlotWindow()
{
	
}

void PlotWindow::closeEvent(QCloseEvent* event) {
	this->saveSettings();

	if (this->currSystem != nullptr) {

		deactivateSystem();
		if (acquisitionThread.isRunning())
		{
			acquisitionThread.quit();
			acquisitionThread.wait();
		}
	}
	event->accept();
}

bool PlotWindow::event(QEvent* event)
{
	if (auto e = dynamic_cast<ImageEvent*>(event); e) {
		QImage img((const uchar*)e->buf.data(), currSystem->width(), currSystem->height(), currSystem->bpp() > 8 ? QImage::Format_Grayscale16 : QImage::Format_Grayscale8);
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "../filename", "Image files (*.png)");
		
		return img.save(fileName);
	}
	return QMainWindow::event(event);
}

void PlotWindow::stopCapture()
{
	if (!this->currSystem) return;
	if (this->currSystem->isAcquisition())
	{
		this->currSystem->stopAcquisition();
	}
	QCoreApplication::processEvents();
	while (currSystem->isAcquisition())
	{
		QThread::msleep(100);
	}

}

void PlotWindow::createContent()
{
	createPlot();
    createHardConfigDockWidget();
	createSystemSettingsDockWidget();
	createMessageConsoleDockWidget();
    createResultsPanelDockWidget();

}

void PlotWindow::createStatusBar()
{
	_statusBar = new StatusBar(STATUS_PANEL_COUNT);
	_statusBar->setText(STATUS_CAMERA, "Ready");
	_statusBar->setHint(STATUS_CAMERA, tr("System status"));
	_statusBar->setText(STATUS_MOUSE_POS, "");
	_statusBar->setHint(STATUS_MOUSE_POS, tr("Mouse coordinates"));
	_statusBar->setMargin(STATUS_ROI_ICON, 6, 0);
	_statusBar->setMargin(STATUS_ROI, 0, 6);

	auto verLabel = _statusBar->addVersionLabel();
	_measureProgress = new MeasureProgressBar;
	_measureProgress->setVisible(false);
	_statusBar->addWidget(_measureProgress);

	setStatusBar(_statusBar);
}

void PlotWindow::createMessageConsoleDockWidget()
{
	this->console = new MessageConsole(this);
	this->console->setObjectName("Message Console");

	messageConsoleDockWidget = prepareDockWidget(this->console, tr("Message"), ads::BottomDockWidgetArea,
		this->actionMessageConsole, QIcon(":/icons/octproz_log_icon.png"), tr("MessageConsole"));

	this->viewToolBar->addAction(actionMessageConsole);
}

void PlotWindow::createHardConfigDockWidget()
{
	stubConfigPanel = new StubHardConfigPanel(this);
	this->hardConfigPanel = stubConfigPanel;

	hardConfigDockWidget = prepareDockWidget(this->hardConfigPanel, tr("Control"), ads::LeftDockWidgetArea,
		this->actionHardConfig, QIcon(":/icons/octproz_extensions_icon.png"), tr("Control"));

	this->viewToolBar->addAction(actionHardConfig);
}

class DataTableWidget : public QTableWidget {
	QSize sizeHint() const override { return { 200, 100 }; }
};

void PlotWindow::createResultsPanelDockWidget()
{
	this->resultPanel = new DataTableWidget();
	_tableIntf = new TableIntf(resultPanel);

	this->resultPanel->setObjectName("Result");
	resultDockWidget = prepareDockWidget(this->resultPanel, tr("Result"), ads::RightDockWidgetArea,
		this->actionResultsPanel , QIcon(":/icons/octproz_klincurve_icon.png"), tr("Result"));

	this->viewToolBar->addAction(actionResultsPanel);
}

void PlotWindow::createSystemSettingsDockWidget()
{
	this->_tree = new QTreeWidget(this);
	this->_tree->setObjectName("Property");
	
	prepareDockWidget(this->_tree, tr("Property"), ads::LeftDockWidgetArea, 
		this->actionSystemSettings, QIcon(":/icons/octproz_rawsignal_icon.png"), tr("Property"));

	this->viewToolBar->addAction(actionSystemSettings);
}

void PlotWindow::loadSettings()
{
	this->loadMainWindowSettings();
}

void PlotWindow::saveSettings()
{
	QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz");
	Settings::getInstance()->setTimestamp(timestamp);
	//this->sidebar->saveSettings();
	this->saveMainWindowSettings();
}

void PlotWindow::loadMainWindowSettings()
{
	Settings* settingsManager = Settings::getInstance();

	//load setting maps
	this->mainWindowSettings = settingsManager->getStoredSettings(MAIN_WINDOW_SETTINGS_GROUP);

	//apply loaded settings
	this->restoreGeometry(this->mainWindowSettings.value(MAIN_GEOMETRY).toByteArray());
	this->restoreState(this->mainWindowSettings.value(MAIN_STATE).toByteArray());
	this->restoreState(this->mainWindowSettings.value(MAIN_DOCK_STATE).toByteArray());
	_dockManager->restoreState(this->mainWindowSettings.value(MAIN_DOCK_STATE).toByteArray());

}

void PlotWindow::saveMainWindowSettings()
{
	this->mainWindowSettings.insert(MAIN_GEOMETRY, this->saveGeometry());
	this->mainWindowSettings.insert(MAIN_STATE, this->saveState());
	this->mainWindowSettings.insert(MAIN_DOCK_STATE, _dockManager->saveState());
	Settings* settingsManager = Settings::getInstance();
	settingsManager->storeSettings(MAIN_WINDOW_SETTINGS_GROUP, this->mainWindowSettings);
	
}

void PlotWindow::createMenuBar()
{
	auto actionClose = new QAction(tr("Exit"), this);
	connect(actionClose, &QAction::triggered, this, &PlotWindow::close);

	this->actionStart = new QAction("Start", this);
	this->actionStart->setIcon(QIcon(":/icons/octproz_play_icon.png"));
	connect(actionStart, &QAction::triggered, this, &PlotWindow::start);

	this->actionStop = new QAction("Stop", this);
	this->actionStop->setIcon(QIcon(":/icons/octproz_stop_icon.png"));
	connect(actionStop, &QAction::triggered, this, &PlotWindow::stop);

	this->actionSelectSystem = new QAction("Open System", this);
	this->actionSelectSystem->setIcon(QIcon(":/icons/octproz_connect_icon.png"));
	connect(this->actionSelectSystem, &QAction::triggered, this, &PlotWindow::slot_selectSystem);

	this->actionSystemSettings = new QAction("System Settings", this);
	this->actionSystemSettings->setIcon(QIcon(":/icons/octproz_settings_icon.png"));
	this->actionSystemSettings->setStatusTip(tr("Settings of the currently loaded acquisition system"));
	connect(this->actionSystemSettings, &QAction::triggered, this, &PlotWindow::slot_menuSystemSettings);

	this->actionMeasure = new QAction(tr("Start Measurements"), this);
	this->actionMeasure->setIcon(QIcon(":/icons/octproz_record_icon.png"));
	connect(actionMeasure, &QAction::triggered, this, &PlotWindow::slot_measurements);
	
	
	this->fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(actionSelectSystem);
    fileMenu->addAction(actionSystemSettings);
	//load settings from file
	fileMenu->addSeparator(); 

	actionSaveRaw = new QAction(tr("Save Raw Image"), this);
	actionSaveRaw->setIcon(QIcon(":/icons/octproz_load_icon.png"));
	connect(actionSaveRaw, &QAction::triggered, this, [this] {/* currSystem->requestRawImg(this);*/ });
	fileMenu->addAction(actionSaveRaw);
	fileMenu->addSeparator();

	QAction* loadSettingsAction = new QAction(tr("Load Settings from File"), this);
	loadSettingsAction->setIcon(QIcon(":/icons/octproz_load_icon.png"));
	connect(loadSettingsAction, &QAction::triggered, this, &PlotWindow::slot_selectAndLoadSettingsFile);
	fileMenu->addAction(loadSettingsAction);

	//save settings to file
	QAction* saveSettingsAction = new QAction(tr("Save Settings to File"), this);
	saveSettingsAction->setIcon(QIcon(":/icons/octproz_save_icon.png"));
	connect(saveSettingsAction, &QAction::triggered, this, &PlotWindow::slot_selectAndSaveSettingsToFile);
	fileMenu->addAction(saveSettingsAction);
	fileMenu->addSeparator();
	fileMenu->addAction(actionClose);
	this->viewMenu = menuBar()->addMenu(tr("&View"));


	//extras menu
	this->toolsMenu = this->menuBar()->addMenu(tr("&Tools"));
    
	this->helpMenu = menuBar()->addMenu(tr("&Help"));
	//user manual
	QAction* manualAct = helpMenu->addAction(tr("&User Manual"), this, &PlotWindow::slot_menuUserManual);
	manualAct->setStatusTip(tr("user manual"));
	manualAct->setIcon(QIcon(":/icons/octproz_manual_icon.png"));
	manualAct->setShortcut(QKeySequence::HelpContents);
	//about dialog
	QAction* aboutAct = helpMenu->addAction(tr("&About"), this, &PlotWindow::slot_menuAbout);
	aboutAct->setStatusTip(tr("About OptoChecker"));
	aboutAct->setIcon(QIcon(":/icons/octproz_info_icon.png"));
	

	this->controlToolBar = this->addToolBar(tr("Control Toolbar"));
	this->controlToolBar->setObjectName("Control Toolbar");
	this->controlToolBar->setVisible(true);
	this->controlToolBar->setMovable(false);
	this->controlToolBar->addAction(actionSelectSystem);
	this->controlToolBar->addAction(actionSystemSettings);
	this->controlToolBar->addSeparator();
	this->controlToolBar->addAction(actionStart);
	this->controlToolBar->addAction(actionStop);
	this->controlToolBar->addAction(actionMeasure);

	this->actionStart->setEnabled(false);
	this->actionStop->setEnabled(false);
	this->actionMeasure->setEnabled(false);

	viewToolBar = this->addToolBar(tr("View Toolbar"));
	viewToolBar->setObjectName("View Toolbar");
	viewToolBar->setVisible(true);
	
}

void PlotWindow::createPlot()
{
	_plot2d = new ImageDisplay();
	_plotIntf = new PlotIntf(_plot2d->graphItem(), _plot2d->getTextDisplay(), _plot2d->getRoi());

	ads::CDockWidget* centerDockWidget = new ads::CDockWidget(tr("Plot"));
	centerDockWidget->setIcon(svgIcon(":/images/photo.svg"));
	centerDockWidget->setWidget(_plot2d, ads::CDockWidget::ForceNoScrollArea);
	centerDockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
	centerDockWidget->setFeature(ads::CDockWidget::DockWidgetForceCloseWithArea, true);
	centerDockWidget->setFeature(ads::CDockWidget::CustomCloseHandling, true);
	centerDockWidget->resize(QSize(640, 480));
	auto toolBar = centerDockWidget->createDefaultToolBar();
	QList<QAction*> size = _plot2d->actions();
	toolBar->addActions(_plot2d->actions());
	_dockManager->setCentralWidget(centerDockWidget);
	_dockManager->addDockWidget(ads::CenterDockWidgetArea, centerDockWidget);
}

void PlotWindow::loadSystemsAndExtensions() {
	QDir pluginsDir = QDir(qApp->applicationDirPath());

	//check if plugins dir exists. if it does not exist change to the share_dev directory. this makes software development easier as plugins can be copied to the share_dev during the build process
	bool pluginsDirExists = pluginsDir.cd("plugins");
	if (!pluginsDirExists) {
#if defined(Q_OS_WIN)
		if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release") {
			pluginsDir.cdUp();
		}
#endif

		pluginsDir.cdUp();
		pluginsDir.cdUp();
		pluginsDir.cd("plugins");

	}
	QStringList nameFilters;
	nameFilters << "*.dll" ;
	for (auto fileName : pluginsDir.entryList(nameFilters, QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject* plugin = loader.instance();
		if (plugin) {
			Plugin* actualPlugin = qobject_cast<Plugin*>(plugin);
			Q_ASSERT(actualPlugin);
			enum PLUGIN_TYPE type = actualPlugin->getType();
			QString name = actualPlugin->getName();
			InterfaceBase* base = actualPlugin->getInterfaceInstance();
			Q_ASSERT(base);
			switch (type) {
			case SYSTEM: {
				this->sysManager->addSystem(name, static_cast<AcquisitionSystem*>(base));
				break;
			}
			case EXTENSION: {
				Extension* extension = static_cast<Extension*>(base);
				
				this->extManager->addExtension(name, extension);
				if (extension->getDisplayStyle() == SEPARATE_WINDOW) {
					//init extension window
					QWidget* extensionWidget = extension->getWidget();
					extensionWidget->setWindowFlags(Qt::Window);
					extensionWidget->setAttribute(Qt::WA_DeleteOnClose, false);
					extensionWidget->setWindowTitle(actualPlugin->getName());
					ExtensionEventFilter* extensionCloseSignalForwarder = new ExtensionEventFilter(extensionWidget); //setting extensionWidget as parent causes ExtensionEventFilter object to be destroyed when extensionWidget gets destroyed
					extensionCloseSignalForwarder->setExtension(extension);
					extensionWidget->installEventFilter(extensionCloseSignalForwarder);
					connect(extensionCloseSignalForwarder, &ExtensionEventFilter::extensionWidgetClosed, this, &PlotWindow::slot_uncheckExtensionInMenu); //this connection is used to automatically uncheck extension in menu if user closes a separate window extension by clicking on x
				}
				break;
			}
			default: {
				emit error(tr("Could not load Plugin"));
			}
			}
		}
		else {
			emit error(tr("Could not load ") + fileName);
		}
	}
	if (this->extManager->getExtensions().size() > 0) {
		this->initExtensionsMenu();
	}
}

void PlotWindow::initExtensionsMenu() {
	QMenu* extensionMenu = this->toolsMenu->addMenu(tr("&Extensions"));
	extensionMenu->setIcon(QIcon(":/icons/octproz_extensions_icon.png"));

	auto extensionNames = this->extManager->getExtensionNames();
	foreach(QString extensionName, extensionNames) {
		QAction* extAction = extensionMenu->addAction(extensionName, this, &PlotWindow::slot_menuExtensions);
		this->extensionActions.append(extAction);
		extAction->setCheckable(true);
		extAction->setChecked(false);
		Extension* extension = this->extManager->getExtensionByName(extensionName);
		QString extensionToolTip = extension == nullptr ? "" : extension->getToolTip(); //todo: error handling if extension is nullptr
		extAction->setStatusTip(extensionToolTip);
	}
}

void PlotWindow::slot_loadSettingsFromFile(QString fileName) {
	this->loadSettingsFromFile(fileName);
}

void PlotWindow::slot_saveSettingsToFile(QString fileName) {
	if (fileName.isEmpty()) {
		emit error(tr("Settings file not saved."));
		return;
	}

	//ensure the file has an extension. use ini as default
	QFileInfo fileInfo(fileName);
	if (fileInfo.suffix().isEmpty()) {
		fileName += ".ini";
		fileInfo.setFile(fileName);
	}

	//ensure current settings are saved before copying
	this->saveSettings();

	//save the settings by copying the existing settings file
	Settings* settingsManager = Settings::getInstance();
	settingsManager->copySettingsFile(fileName);

	emit info(tr("Settings have been saved successfully to: ") + fileName);
}

void PlotWindow::loadSettingsFromFile(const QString& settingsFilePath) {
	if (settingsFilePath.isEmpty()) {
		emit error(tr("Invalid File: ") + tr("No settings file selected or file path is invalid."));
		return;
	}


	// Backup current settings file
	QString backupPath = SETTINGS_PATH + ".backup";
	QFile::remove(backupPath);  // Remove any existing backup
	QFile::copy(SETTINGS_PATH, backupPath);

	// Attempt to copy the selected settings file to the default settings path
	if (!QFile::remove(SETTINGS_PATH)) {
		emit error(("Failed to remove the existing settings file."));
		return;
	}

	if (!QFile::copy(settingsFilePath, SETTINGS_PATH)) {
		emit error(("Failed to load settings from: " + settingsFilePath));
		QFile::copy(backupPath, SETTINGS_PATH);
		return;
	}

	// Reload the settings
	this->loadSettings();
	if (!this->currSystemName.isEmpty()) {
		//emit loadPluginSettings(Settings::getInstance()->getStoredSettings(this->currSystemName));
	}
	// Inform the user that the settings have been loaded
	emit info(tr("Settings have been loaded successfully from: ") + settingsFilePath);
}

void PlotWindow::updateHardConfigDockWidget()
{
	if (hardConfigPanel && hardConfigPanel != (HardConfigPanel*)(currSystem->hardConfgPanel(this)))
	{
		hardConfigPanel->deleteLater();
		hardConfigPanel = (HardConfigPanel*)(currSystem->hardConfgPanel(this));
		hardConfigDockWidget->setWidget(hardConfigPanel);
	}
}

ads::CDockWidget* PlotWindow::prepareDockWidget(QWidget* widgetForDock,
	QString docktitle, 
	ads::DockWidgetArea area, 
	QAction*& action, 
	const QIcon& icon, 
	QString iconText)
{
	ads::CDockWidget* dockWidget = new ads::CDockWidget(docktitle);
	dockWidget->setWidget(widgetForDock);
	_dockManager->addDockWidget(area, dockWidget);

	dockWidget->setVisible(true);
	//dockWidget->setFloating();

	action = dockWidget->toggleViewAction();
	if (!icon.isNull()) {
		action->setIcon(icon);
	}
	action->setIconText(iconText);
	this->viewMenu->addAction(action);
	return dockWidget;
}

void PlotWindow::updateControls()
{
	bool opened = currSystem && currSystem->isAcquisition();
	bool started = (bool)_saver;

	actionSelectSystem->setDisabled(started);
	actionSystemSettings->setDisabled(started || !opened);
	actionStart->setDisabled(opened);
	actionStop->setDisabled(started || !opened);
	actionMeasure->setText(started ? tr("Stop Measurements") : tr("Start Measurements"));
	actionMeasure->setDisabled(!opened);

	//hardConfigPanel->setReadOnly(started || !opened);
}

void PlotWindow::slot_menuExtensions() {
	//todo: refactor this method. Think of a better way to asociate corresponding extensions and qactions. Maybe store extensions and qactins in a qmap or in a qlist with qpairs.
	QAction* currAction = qobject_cast<QAction*>(sender());
	if (currAction == 0) { return; }
	QString extensionName = currAction->text();
	Extension* extension = this->extManager->getExtensionByName(extensionName); //this just works if extension names are unique
	extension->settingsLoaded(Settings::getInstance()->getStoredSettings(extensionName)); //todo: use only signal slot to interact with extension (do not call methods directly like in this line) and move extensions to threads. Similar to AcquisitionSystems, see: implementation of activateSystem(AcquisitionSystem* system)
	if (extension == nullptr) {
		emit error(tr("No Extension with name ") + extensionName + tr(" exists."));
		return;
	}
	QWidget* extensionWidget = extension->getWidget();
	
	//if extension is deactivated (i. e. not visible as tab within sidebar and not visible as separate window) and user checked the extension in the menu then activate it.
	if ((extension->getDisplayStyle() == SIDEBAR_TAB )&& _dockManager->findDockWidget(extensionName) == nullptr || (extension->getDisplayStyle() == SEPARATE_WINDOW && !extensionWidget->isVisible())) {
		if (currAction->isChecked()) {
			if (extension->getDisplayStyle() == SIDEBAR_TAB) {
				prepareDockWidget(extensionWidget, extensionName, ads::LeftDockWidgetArea,
					this->actionTemp, QIcon(":/icons/octproz_rawsignal_icon.png"), extensionName);
			}
			else if (extension->getDisplayStyle() == SEPARATE_WINDOW) {
				extensionWidget->setWindowFlag(Qt::WindowStaysOnTopHint);
				extensionWidget->show();
			}
			connect(extension, &Extension::info, this, &PlotWindow::info);
			connect(extension, &Extension::error, this, &PlotWindow::error);
			connect(extension, &Extension::storeSettings, this, &PlotWindow::slot_storePluginSettings);
			extension->activateExtension(); //todo: do not call extension methods directly, use signal slot (or invokeMethod -> see below) and run extension in separate thread
			
		}
	}
	//else (i.e. extension is visible within sidebar or as separate window) deactivate extension if user unchecked extension in menu
	else {
		if (!currAction->isChecked()) {
			if (extension->getDisplayStyle() == SIDEBAR_TAB) {
				if (_dockManager->findDockWidget(extensionName) != nullptr)
				{
					auto deck = _dockManager->findDockWidget(extensionName);
                    deck->hide();
					_dockManager->removeDockWidget(deck);
					deck->deleteLater();
				}

				extension->deactivateExtension();
				disconnect(extension, &Extension::info, this, &PlotWindow::info);
				disconnect(extension, &Extension::error, this, &PlotWindow::error);
				disconnect(extension, &Extension::storeSettings, this, &PlotWindow::slot_storePluginSettings);
				
			}
			else if (extension->getDisplayStyle() == SEPARATE_WINDOW) {
				extensionWidget->close();
			}
		}
	}
}

void PlotWindow::showFps(double fps, double hardFps)
{
	if (fps <= 0) {
		_statusBar->setText(STATUS_FPS, QStringLiteral("FPS: NA"));
		_statusBar->setHint(STATUS_FPS, {});
		_statusBar->setStyleSheet(STATUS_FPS, {});
		return;
	}
	_statusBar->setText(STATUS_FPS, QStringLiteral("FPS: ") % QString::number(fps, 'f', 2));
	if (hardFps > 0 && qCeil(fps) < qFloor(hardFps)) {
		_statusBar->setHint(STATUS_FPS, tr("The system likely run out of CPU resources.\nActual FPS is lower than camera produces (%1).").arg(hardFps));
		_statusBar->setStyleSheet(STATUS_FPS, QStringLiteral("QLabel{background:red;font-weight:bold;color:white}"));
	}
	else {
		_statusBar->setHint(STATUS_FPS, {});
		_statusBar->setStyleSheet(STATUS_FPS, {});
	}
}
void PlotWindow::statsReceived(const CameraStats& stats)
{
	showFps(stats.fps, stats.hardFps);
	if (stats.measureTime >= 0)
		_measureProgress->setElapsed(stats.measureTime);
}

void PlotWindow::slot_uncheckExtensionInMenu(Extension* extension) {
	QString extensionName = extension->getBasePlugin()->getName();
	QAction* currAction = nullptr;
	foreach(auto action, this->extensionActions) {
		if (action->text() == extensionName) {
			currAction = action;
			break;
		}
	}

	//uncheck action in menu
	currAction->setChecked(false);

	//disconnect signal slots from closed extension
	extension->deactivateExtension();
	disconnect(extension, &Extension::info, this, &PlotWindow::info);
	disconnect(extension, &Extension::error, this, &PlotWindow::error);
	disconnect(extension, &Extension::storeSettings, this, &PlotWindow::slot_storePluginSettings);
}

void PlotWindow::setSystem(QString systemName) {
	if (this->currSystemName == systemName) { //system already activated
		emit info(tr("System is already open."));
		return;
	}

	AcquisitionSystem* system = this->sysManager->getSystemByName(systemName);

	

	system->initResultTableIntf(_tableIntf);
	system->initPlotIntf(_plotIntf);

	if (system == nullptr) {
		emit error(tr("Opening failed. Could not find a system with the name: ") + systemName);
		return;
	}

	if (this->currSystem != nullptr) {
		deactivateSystem();
	}


	activateSystem(system);
	
	this->currSystem.reset(system);
	this->currSystemName = systemName;
	this->setWindowTitle("Optochecker - " + systemName);
	emit loadPluginSettings(Settings::getInstance()->getStoredSettings(systemName));
	this->actionStart->setEnabled(true);
	emit info(tr("System opened: ") + this->currSystemName);

	
}

void PlotWindow::activateSystem(AcquisitionSystem* system) {
	if (system != nullptr) {
		system->moveToThread(&acquisitionThread);
		connect(this, &PlotWindow::start, system, &AcquisitionSystem::startAcquisition);
		connect(this, &PlotWindow::stop, system, &AcquisitionSystem::stopAcquisition);
		connect(this, &PlotWindow::loadPluginSettings, system, &AcquisitionSystem::settingsLoaded);
		connect(system, &AcquisitionSystem::storeSettings, this, &PlotWindow::slot_storePluginSettings);
		connect(system, &AcquisitionSystem::acquisitionStarted, this, &PlotWindow::slot_start);
		connect(system, &AcquisitionSystem::acquisitionStopped, this, &PlotWindow::slot_stop);
		connect(system, &AcquisitionSystem::newImageReady, _plot2d, &ImageDisplay::receiveFrame);
		connect(system, &AcquisitionSystem::ready, this, &PlotWindow::dataReady);
		//connect(this, &PlotWindow::pluginSettingsRequest, system->settingsForm, &QWidget::show);
		//connect(this, &PlotWindow::pluginSettingsRequest, system->settingsForm, &QWidget::raise);
		connect(this, SIGNAL(PlotWindow::pluginSettingsRequest()), system, SLOT(AcquisitionSystem::editConfig(1)));
		connect(system, &AcquisitionSystem::status, this, &PlotWindow::statsReceived);
		connect(system, &AcquisitionSystem::info, this, &PlotWindow::info);
		connect(system, &AcquisitionSystem::error, this, &PlotWindow::error);
		connect(this, &PlotWindow::info, system, &AcquisitionSystem::logInfo);
		connect(this, &PlotWindow::error, system, &AcquisitionSystem::logError);
		connect(qApp, &QCoreApplication::aboutToQuit, system, &QObject::deleteLater);
		acquisitionThread.start();
	}
}

void PlotWindow::deactivateSystem() {

	stopCapture();

	if (currSystem->isAcquisition())
	{
		emit error(tr("System stop faild!"));
	}
	else
	{
		currSystem.reset(nullptr);
	}
}

void PlotWindow::slot_selectSystem() {
	QString selectedSystem = this->sysChooser->selectSystem(this->sysManager->getSystemNames());
	this->setSystem(selectedSystem);

	this->saveSettings();

	//disable start/stop buttons
	this->actionStart->setEnabled(false);
	this->actionStop->setEnabled(true);
	this->actionMeasure->setEnabled(true);

	emit start();
	
}

void PlotWindow::slot_menuSystemSettings() {
	if (this->currSystem != nullptr) {
		currSystem->editConfig();
		emit pluginSettingsRequest();
	}
	else {
		emit error(tr("No system opened!"));
	}
}
void PlotWindow::slot_selectAndLoadSettingsFile() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select Settings File"), "", tr("Settings Files (*.ini *.txt)"));

	if (!fileName.isEmpty()) {
		this->loadSettingsFromFile(fileName);
	}
}
void PlotWindow::slot_selectAndSaveSettingsToFile() {
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Settings File"), "", tr("Settings Files (*.ini *.txt);;All Files (*.*)"));
	this->slot_saveSettingsToFile(fileName);
}
void PlotWindow::slot_menuUserManual() {
	QDesktopServices::openUrl(QUrl("file:///" + QCoreApplication::applicationDirPath() + "/docs/index.html"));
}

void PlotWindow::slot_menuAbout() {
	this->aboutWindow->show();
}

void PlotWindow::slot_measurements()
{
	if (!currSystem) return;

	if (!currSystem->isAcquisition()) {
		emit error(tr("Camera is not opened"));
		return;
	}

	if (_saver)
	{
		if (!yes(tr("Interrupt measurements?")))
			return;
		currSystem->stopMeasure();
		// Process the last MeasureEvent
		qApp->processEvents();
		_saver.reset(nullptr);
		_measureProgress->setVisible(false);
		updateControls();
		return;
	}

	auto cfg = MeasureSaver::configure();
	if (!cfg)
		return;

	auto saver = new MeasureSaver();
	auto res = saver->start(*cfg, currSystem.get());
	if (!res.isEmpty()) {
		error(tr("Failed to start measuments:\n%1").arg(res));
		return;
	}

	connect(saver, &MeasureSaver::finished, this, [this] {
		slot_measurements();
		emit info(tr("<b>Measurements saver finished<b>"));
		});
	connect(saver, &MeasureSaver::interrupted, this, [this](const QString& error) {
		slot_measurements();
		emit info(tr("<b>Measurements saver interrupted</b><p>") + QString(error).replace("\n", "<br>"));
		});
	_saver.reset(saver);

	currSystem->startMeasure(_saver.get());

	_measureProgress->reset(cfg->durationInf ? 0 : cfg->durationSecs(), cfg->fileName);

	updateControls();
}

void PlotWindow::slot_storePluginSettings(QString pluginName, QVariantMap settings) {
	Settings::getInstance()->storeSettings(pluginName, settings);
}

void PlotWindow::slot_start() {
	if (this->currSystem == nullptr) {
		if (!this->currSystem->isAcquisition()) {
			return;
		}
	}
	updateHardConfigDockWidget();

	//save current parameters to hdd
	this->saveSettings();

	//disable  buttons
	updateControls();

	//for debugging purposes: read out thread affinity of current thread
	qDebug() << "Main Thread ID start emit: " << QThread::currentThreadId();

}

void PlotWindow::slot_stop() {
	//adjust  buttons
	updateControls();
}

void PlotWindow::dataReady()
{
	_tableIntf->showResult();
	_plotIntf->showResult();
	_statusBar->setVisible(STATUS_NO_DATA, 1);
	
	_plot2d->getScene()->update();
}