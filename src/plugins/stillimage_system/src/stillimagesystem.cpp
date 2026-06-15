#include "StillImageSystem.h"
#include "optodevkit/CameraWorker.h"

#include <QDebug>

StillImageSystemPlugin::StillImageSystemPlugin(QObject* parent ) : Plugin(parent)
{
	type = PLUGIN_TYPE::SYSTEM;
	name = "StillImageSystem";
	descr = "StillImage System";
}
StillImageSystemPlugin::~StillImageSystemPlugin()
{
}

InterfaceBase* StillImageSystemPlugin::getInterfaceInstance() {
	NEW_PLUGININSTANCE(StillImageSystem)
}


StillImageSystem::StillImageSystem(QObject* parent) : AcquisitionSystem(parent) {
	_configGroup = "StillImageSystem";
	loadConfig();

	this->systemDialog = new StillImageSystemSettingsDialog();
	this->settingsForm = static_cast<QDialog*>(this->systemDialog);
	this->isCleanupPending  = false;

	connect(this->systemDialog, &StillImageSystemSettingsDialog::settingsUpdated, this, &StillImageSystem::slot_updateParams);
	connect(this, &StillImageSystem::enableGui, this->systemDialog, &StillImageSystemSettingsDialog::slot_enableGui);

	//default values
	this->currParams.filePath = "";
	this->currParams.depth = 8;
	this->currParams.width = 256;
	this->currParams.height = 256;
	this->currParams.waitTimeUs = 100;
	this->currParams.copyFileToRam = true;
}

enum CamDataRow { ROW_LOAD_TIME, ROW_CALC_TIME };

StillImageSystem::~StillImageSystem() {
	this->cleanup();
	qDebug() << "StillImageSystem destructor. Thread ID: " << QThread::currentThreadId();
}

bool StillImageSystem::init() {
	//check if user selected file can be opened
	if(!this->openFileToCopyToRam()){
		return false;
	}

	this->currParams.depth = _image.depth();
	currParams.width = _image.width();
	currParams.height = _image.height();

	this->systemDialog->updateGui(this->currParams);

	emit info (tr("StillImageSystem initialized!"));
	
	return true;
}


//QString StillImageSystem::name() const
//{
//	QFileInfo fi(this->currParams.filePath);
//	return fi.fileName();
//}

QString StillImageSystem::descr() const
{
	QFileInfo fi(this->currParams.filePath);
	return fi.fileName();
}

int StillImageSystem::width() const
{
	return _image.width();
}

int StillImageSystem::height() const
{
	return _image.height();
}

int StillImageSystem::bpp() const
{
	return _image.depth();
}

TableRowsSpec StillImageSystem::tableRows() const
{
	auto rows = AcquisitionSystem::tableRows();
	rows.aux
		<< qMakePair(ROW_LOAD_TIME, qApp->tr("Load time"))
		<< qMakePair(ROW_CALC_TIME, qApp->tr("Calc time"));
	return rows;
}

void StillImageSystem::startAcquisition(){
	//check if cleanup is pending from previous acquisition
	if(this->isCleanupPending){
		this->cleanup();
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

	this->acqcuisitionSimulation();

	//acuquisition stopped
	this->isCleanupPending = true;
	emit enableGui(true);
	emit info("Acquisistion stopped!");
	emit acquisitionStopped();
	//wait some time before releasing buffer memory to allow extensions and 1d plot window to process last raw buffer
	QCoreApplication::processEvents();
	QThread::msleep(500);
	QCoreApplication::processEvents();
	this->cleanup();
	this->isCleanupPending = false;
}

void StillImageSystem::stopAcquisition(){
	this->acqusitionRunning = false;
	emit enableGui(true);
	qDebug() << "Plugin Thread ID stopAcq: " << QThread::currentThreadId();
}


void StillImageSystem::cleanup() {
	
}

bool StillImageSystem::openFileToCopyToRam() {
	QString fileName;
	if (this->currParams.filePath.size() < 1) {
		emit error(tr("No file selected for StillImage system."));
		return false;
	}else{
		fileName = this->currParams.filePath;
	}
	_image = QImage(fileName);
	if (_image.isNull()) {
		emit error(tr("Unable to open file for StillImage system!"));
		return false;
	}
	

	return true;
}

void StillImageSystem::settingsLoaded(QVariantMap settings){
	loadConfig();

	this->systemDialog->setSettings(settings);
}


void StillImageSystem::acqcuisitionSimulation(){

	//acquisition begins!
	emit enableGui(false);

	emit acquisitionStarted(this);
	this->acqusitionRunning = true;
	while (this->acqusitionRunning) {

		QElapsedTimer timer;

		timer.start();

		if (!this->openFileToCopyToRam()) {
			emit info(tr("Unable to to load image file"));
			this->acqusitionRunning = false;
			return;
		}
		auto loadTime = timer.elapsed();


		//user defined wait time
		unsigned int channel = 1;
		switch (_image.format())
		{
		case QImage::Format_ARGB32:
		case QImage::Format_RGB32:
		case QImage::Format_ARGB32_Premultiplied:
			channel = 4;
			break;
		case QImage::Format_RGB888:
		case QImage::Format::Format_BGR888:
			channel = 3;
			break;
		default:
			break;
		}

		emit newImageReady(_image.bits(), _image.depth(), _image.width(), _image.height(), channel);
		QThread::usleep((this->currParams.waitTimeUs));
	}
}

void StillImageSystem::measBeamBkgnd()
{
	QElapsedTimer timer;

	timer.start();

	const uchar* buf = _image.bits();

	CgnBeamCalc c;
	auto fmt = _image.format();
	c.w = _image.width();
	c.h = _image.height();
	c.bpp = fmt == QImage::Format_Grayscale16 ? 16 : 8;
	c.buf = (uint8_t*)buf;
	//emit (c.w, c.h)
	//drawRaw Graph
	double* graph = new double[static_cast<int>(c.w * c.h)];

	CgnBeamResult r;
	memset(&r, 0, sizeof(CgnBeamResult));
	QList<CgnBeamResult> results;

	CgnBeamBkgnd g;
	memset(&g, 0, sizeof(CgnBeamBkgnd));
	g.max_iter = _config.bgnd.iters;
	g.precision = _config.bgnd.precision;
	g.corner_fraction = _config.bgnd.corner;
	g.nT = _config.bgnd.noise;
	g.mask_diam = _config.bgnd.mask;

	auto roiMode = _config.roiMode;

	auto setRoi = [&c, &g, &r, roiMode](const RoiRect& roi) {
		if (roiMode != ROI_NONE && roi.isValid()) {
			g.ax1 = qRound(roi.left * double(c.w));
			g.ay1 = qRound(roi.top * double(c.h));
			g.ax2 = qRound(roi.right * double(c.w));
			g.ay2 = qRound(roi.bottom * double(c.h));
		}
		else {
			g.ax1 = 0;
			g.ay1 = 0;
			g.ax2 = c.w;
			g.ay2 = c.h;
		}
		r.x1 = g.ax1;
		r.y1 = g.ay1;
		r.x2 = g.ax2;
		r.y2 = g.ay2;
	};

	bool subtract = _config.bgnd.on;
	QVector<double> subtracted;
	if (subtract) {
		subtracted = QVector<double>(c.w * c.h);
		g.subtracted = subtracted.data();
	}

	timer.restart();
	if (_config.roiMode == ROI_MULTI)
	{
		if (subtract) {
			g.min = 1e10;
			g.max = -1e10;
			g.subtract_bkgnd_v = 1;
			cgn_copy_to_f64(&c, g.subtracted, nullptr);
		}
		for (const auto& roi : std::as_const(_config.rois)) {
			setRoi(roi);
			cgn_calc_beam_bkgnd(&c, &g, &r);
			results << r;
		}
	}
	else
	{
		setRoi(_config.roi);
		cgn_calc_beam_bkgnd(&c, &g, &r);
		results << r;
	}
	auto calcTime = timer.elapsed();

	double minZ, maxZ;
	cgn_ext_copy_to_f64(&c, &g, graph, _config.plot.normalize, _config.plot.fullRange, &minZ, &maxZ);
	//_plot->invalidateGraph();
	//_plot->setResult(results, minZ, maxZ);

	/*_table->setResult(results, {}, {
		{ ROW_LOAD_TIME, {loadTime} },
		{ ROW_CALC_TIME, {calcTime} },
		});*/

}

void StillImageSystem::startMeasure(MeasureSaver* saver)
{
	//_render->startMeasure(saver);
}

void StillImageSystem::stopMeasure()
{
	//_render->stopMeasure();
}

void StillImageSystem::slot_updateParams(imageParams newParams){
	this->currParams = newParams;

	//store settings, so settings can be reloaded into gui at next start of application
	this->systemDialog->getSettings(&this->settingsMap);
	emit storeSettings(_configGroup, this->settingsMap);
}
