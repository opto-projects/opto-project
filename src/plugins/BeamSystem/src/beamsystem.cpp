#include "beamsystem.h"
#include "IdsCameraConfig.h"
#include "IdsHardConfig.h"

#include "optodevkit/MeasureSaver.h"
#include "ImageView/imagedisplay.h"

#include <QTableWidget>
#include <QDebug>


#define LOG_ID "BeamSystem:"


BeamSystemPlugin::BeamSystemPlugin(QObject* parent ) : Plugin(parent)
{
	type = PLUGIN_TYPE::SYSTEM;
	name = "BeamSystem";
	descr = "StillImage System";
}
BeamSystemPlugin::~BeamSystemPlugin()
{
}

InterfaceBase* BeamSystemPlugin::getInterfaceInstance() {
	NEW_PLUGININSTANCE(BeamSystem)
}


BeamSystem::BeamSystem(QObject* parent) : AcquisitionSystem(parent){
    _initSuccessfull = false;
    _configGroup = "BeamSystem";
    _tableIntf = nullptr;
    _plotIntf = nullptr;
    _cfg.reset(new IdsCameraConfig);
   

	this->systemDialog = new BeamSystemSettingsDialog();
	this->settingsForm = static_cast<QDialog*>(systemDialog);
	this->_name = "BeamSystem";
	this->isCleanupPending  = false;

    _qmmCore = new QMMCore();
    _qmmCore->CreatePropertyBrowser();
   

	connect(this->systemDialog, &BeamSystemSettingsDialog::settingsUpdated, this, &BeamSystem::slot_updateParams);
	//connect(this, &BeamSystem::enableGui, this->systemDialog, &BeamSystemSettingsDialog::slot_enableGui);
    connect(this, &BeamSystem::enableGui, this->_qmmCore, &QMMCore::slot_enableGui);

	//default values
	this->currParams.filePath = "";
	this->currParams.depth = 8;
	this->currParams.width = 256;
	this->currParams.height = 256;
	this->currParams.waitTimeUs = 100;
	this->currParams.copyFileToRam = true;
}


BeamSystem::~BeamSystem() {
	this->cleanup();
}

bool BeamSystem::init() {
    if (!_initSuccessfull)
    {
        _qmmCore->LoadSystem();
        if (_qmmCore->OpenSystem())
        {
            _initSuccessfull = true;
        }
    }
    


    qDebug() << "BeamSystem init. Thread ID: " << QThread::currentThreadId();
	//check if user selected file can be opened 
    std::vector<std::string> proplist = _qmmCore->core_.getAllowedPropertyValues("Camera", "PixelType");
    unsigned index = 1;
    _qmmCore->core_.setProperty("Camera", "PixelType", proplist[index].c_str());
    _qmmCore->snapImage();

	this->currParams.width = _qmmCore->getImageWidth();
	this->currParams.height = _qmmCore->getImageHeight();
	this->currParams.bytes = _qmmCore->getBytesPerPixel();
	this->currParams.depth = _qmmCore->getImageBitDepth();
	this->currParams.components = _qmmCore->getNumberOfComponents();
	this->currParams.channels = _qmmCore->getNumberOfComponents();

	this->systemDialog->updateGui(this->currParams);

    CameraWorker::setup(this);

	emit info (tr("BeamSystem initialized!"));
	
	return _initSuccessfull;
}


QString BeamSystem::name() const
{
	return _name;
}

QString BeamSystem::descr() const
{
	return _descr;
}

int BeamSystem::width() const
{
	return this->currParams.width;
}

int BeamSystem::height() const
{
	return this->currParams.height;
}

int BeamSystem::bpp() const
{
	return this->currParams.depth;
}

void BeamSystem::startAcquisition(){
	//check if cleanup is pending from previous acquisition
	if(this->isAcquisition()){
        return;
	}

	//init acquisition
	bool initSuccessfull = this->init();
	if(!initSuccessfull){
		emit enableGui(true);
		emit info(tr("Initialization unsuccessful. Acquisition stopped."));
		this->cleanup();
		emit acquisitionStopped();
		return;
	}
	qDebug() << "Plugin Thread ID : " << QThread::currentThreadId();
	//start acquisition
	emit info("Acquisition started");
    //acquisition begins!
    emit enableGui(false);
    

    emit acquisitionStarted(this);
    this->acqusitionRunning = true;
    this->stopInterruption = false;

    //worker
    this->run();

	this->isCleanupPending = true;
	emit enableGui(true);
	emit info("Acquisistion stopped!");
	emit acquisitionStopped();
	QCoreApplication::processEvents();
	this->cleanup();
	this->isCleanupPending = false;
    this->acqusitionRunning = false;
}

void BeamSystem::stopAcquisition(){
    QMutexLocker locker(&stopInterruptionMutex);
    this->stopInterruption = true;
	
	emit enableGui(true);
	qDebug() << "Plugin Thread ID stopAcq: " << QThread::currentThreadId();
}


void BeamSystem::cleanup() {

}


void BeamSystem::settingsLoaded(QVariantMap settings){

	this->systemDialog->setSettings(settings);
}

void BeamSystem::logInfo(const QString msg)
{
    _qmmCore->logInfo(msg);
}
void BeamSystem::logError(const QString msg)
{
    _qmmCore->logError(msg);
}


void BeamSystem::run()
{
    startAcqTime();

    while (true) {

        prevFrameAcqTime();

        _qmmCore->snapImage();

        int width = _qmmCore->getImageWidth();
        int height = _qmmCore->getImageHeight();
        int bytes = _qmmCore->getBytesPerPixel();
        int depth = _qmmCore->getImageBitDepth();
        int components = _qmmCore->getNumberOfComponents();
        int channels = _qmmCore->getNumberOfComponents();
        int bufsize = _qmmCore->getImageBufferSize();
        void* buf1 = _qmmCore->getImage();
        uchar* buf = (uchar*)(buf1);
       
        
        if (bufsize == 0) {
            qCritical() << LOG_ID << "FrameAcq" << " grab null image";
            emit this->error("FrameAcq: grab null image");
            return;
        }

        markAcqTime();

        if (bufsize > 1) {

            //_image = QImage(buf, width, height, depth == 8 ? QImage::Format_Grayscale8 : QImage::Format_Grayscale16).copy();
            //_image.fill(0);

            c.w = width;
            c.h = height;
            c.bpp = depth;
            cgnConvert(buf, bufsize);
            calcResult();

            markCalcTime();

            if (!(abs(tm - prevReady) < PLOT_FRAME_DELAY_MS))
            {
                emit newImageReady(buf, depth, width, height, channels);

                if (showResults())
                {
                    emit ready();
                }

                prevReady = tm;
            }


            checkReconfig();

            QCoreApplication::processEvents();

        }
        else {
            frameErrors();
        }

        auto st = finishFreme();

        emit status(st);

        QCoreApplication::processEvents();
        if (this->stopInterruption) {
            qDebug() << LOG_ID << "Interrupted by user";
            return;
        }
    }
}


void BeamSystem::startMeasure(MeasureSaver* saver)
{
    startMeasureSaver(saver);
}

void BeamSystem::stopMeasure()
{
    stopMeasureSaver();
}

HardConfigPanel* BeamSystem::hardConfgPanel(QWidget* parent)
{
    if (!_configPanel) {
        auto getCamProp = [this](IdsHardConfigPanel::CamProp prop) -> QVariant {
            switch (prop) {
            case IdsHardConfigPanel::AUTOEXP_LEVEL:
                return _cfg->autoExpLevel;
            case IdsHardConfigPanel::AUTOEXP_FRAMES_AVG:
                return _cfg->autoExpFramesAvg;
            case IdsHardConfigPanel::EXP_PRESETS:
                return QVariant::fromValue(&_cfg->expPresets);
            case IdsHardConfigPanel::FPS_LOCK:
                return _cfg->fpsLock;
            }
            return {};
        };
        auto setCamProp = [this](IdsHardConfigPanel::CamProp prop, QVariant value) {
            switch (prop) {
            case IdsHardConfigPanel::AUTOEXP_LEVEL:
                _cfg->autoExpLevel = value.toInt();
                break;
            case IdsHardConfigPanel::AUTOEXP_FRAMES_AVG:
                // this is not changed from hard config panel
                break;
            case IdsHardConfigPanel::EXP_PRESETS:
                break;
            case IdsHardConfigPanel::FPS_LOCK:
                _cfg->fpsLock = value.toDouble();
                break;
            }
        };
        auto requestBrightness = [this](QObject* s) { this->requestBrightness(s); };
        auto exposureChanged = [this]() {
            
            this->raisePowerWarning();
            //requestExpWarning();
        };
        _configPanel = new IdsHardConfigPanel(&_qmmCore->core_,
            getCamProp, setCamProp, requestBrightness, exposureChanged, parent);
    }
    return _configPanel;
}

QWidget* BeamSystem::propPanel(QWidget* parent)
{
    _qmmCore->propertBrowser->refreshDeviceList();
    return _qmmCore->propertBrowser;
}

void BeamSystem::initPlotIntf(PlotIntf* plotIntf)
{
    _plotIntf = plotIntf;
    setPlotData = [this](const QVector<CgnBeamResult>& r, double min, double max) {
        if (_plotIntf)
        {
            _plotIntf->setResult(r, min, max);
        }
        
    };
}

void BeamSystem::initResultTableIntf(TableIntf* intf)
{
    _tableIntf = intf;
    _tableIntf->setRows(this->tableRows());
    _tableIntf->setScale(this->pixelScale());
    setTableData =
        [this](const QVector<CgnBeamResult>& val, const QVector<CgnBeamResult>& sdev, const QMap<ResultId, CamTableData>& tabledata) {
        if (_tableIntf)
        {
            _tableIntf->setResult(val, sdev, tabledata);
        }
    };
}

bool BeamSystem::editConfig(int page)
{
    return CameraWorker::editConfig(page);
}

void BeamSystem::setRoi(const RoiRect& roi)
{
    bool powerWarning = !roi.isEqual(_config.roi);
    _config.roi = roi;
    _config.roi.fix();
    saveConfig();
    if (powerWarning)
        raisePowerWarning();
}

void BeamSystem::setRois(const QList<RoiRect>& rois)
{
    bool powerWarning = false;
    if (_config.rois.size() != rois.size())
        powerWarning = true;
    else {
        for (int i = 0; i < rois.size(); i++)
            if (!_config.rois.at(i).isEqual(rois.at(i))) {
                powerWarning = true;
                break;
            }
    }
    _config.rois = rois;
    for (auto roi : std::as_const(_config.rois)) roi.fix();
    saveConfig();
    if (powerWarning)
        raisePowerWarning();
}

void BeamSystem::setRois(const QList<QPointF>& points)
{
    QList<RoiRect> rois;
    for (const auto& p : points) {
        RoiRect roi;
        roi.left = p.x() - _config.mroiSize.w / 2.0;
        roi.top = p.y() - _config.mroiSize.h / 2.0;
        roi.right = p.x() + _config.mroiSize.w / 2.0;
        roi.bottom = p.y() + _config.mroiSize.h / 2.0;
        rois << roi;
    }
    setRois(rois);
}

void BeamSystem::setRoisSize(const FrameSize& sz)
{
    _config.mroiSize = sz;

    QList<QPointF> points;
    for (const auto& roi : std::as_const(_config.rois)) {
        points << QPointF(
            (roi.left + roi.right) / 2.0,
            (roi.top + roi.bottom) / 2.0
        );
    }
    setRois(points);
}

void BeamSystem::setRoiMode(RoiMode mode)
{
    if (_config.roiMode != mode) {
        _config.roiMode = mode;
        saveConfig();
        raisePowerWarning();
    }
}

RoiMode BeamSystem::getRoiMode()
{
    return _config.roiMode;
}

RoiRect BeamSystem::getRoi()
{
    return _config.roi;
}

QList<RoiRect> BeamSystem::getRois()
{
    return _config.rois;
}

QList<QPair<int, QString>> BeamSystem::measurCols() const
{
    QList<QPair<int, QString>> cols;
    if (_cfg->saveBrightness)
        cols << qMakePair(COL_BRIGHTNESS, qApp->tr("Brightness"));
    if (_config.power.on)
        cols << qMakePair(COL_POWER, qApp->tr("Power"));
    return cols;
}

void BeamSystem::slot_updateParams(beamParams newParams){
	this->currParams = newParams;

	//store settings, so settings can be reloaded into gui at next start of application
	this->systemDialog->getSettings(&this->settingsMap);
	emit storeSettings(_configGroup, this->settingsMap);
}

