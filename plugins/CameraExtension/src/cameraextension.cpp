#include "cameraextension.h"


CameraExtension::CameraExtension() : Extension() {
	qRegisterMetaType<CameraExtensionParameters>("CameraExtensionParameters");
	
	//init extension
	this->setType(EXTENSION);
	this->displayStyle = SEPARATE_WINDOW;
	this->name = "Camera Extension";
	this->toolTip = "Live display of connected camera";

	//init gui
	this->form = new CameraExtensionForm();
	connect(this->form, &CameraExtensionForm::info, this, &CameraExtension::info);
	connect(this->form, &CameraExtensionForm::error, this, &CameraExtension::error);
	
	//settings
	connect(this->form, &CameraExtensionForm::paramsChanged, this, &CameraExtension::storeParameters);
}


CameraExtension::~CameraExtension() {
	delete this->form;
}

QWidget* CameraExtension::getWidget() {
	//this->widgetDisplayed = true;
	return this->form;
}

void CameraExtension::activateExtension() {
	//this method is called by OCTproZ as soon as user activates the extension. If the extension controls hardware components, they can be prepared, activated, initialized or started here.
	//this->active = true;
}

void CameraExtension::deactivateExtension() {
	//this method is called by OCTproZ as soon as user deactivates the extension. If the extension controls hardware components, they can be deactivated, resetted or stopped here.
	//this->active = false;
}

void CameraExtension::settingsLoaded(QVariantMap settings) {
	//this method is called by OCTproZ and provides a QVariantMap with stored settings/parameters.
	this->form->setSettings(settings); //update gui with stored settings
}

void CameraExtension::storeParameters() {
	//update settingsMap, so parameters can be reloaded into gui at next start of application
	this->form->getSettings(&this->settingsMap);
	emit storeSettings(this->name, this->settingsMap);
}

void CameraExtension::rawDataReceived(void* buffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	//do nothing here as we do not need the raw data. Q_UNUSED is used to suppress compiler warnings
	Q_UNUSED(buffer)
	Q_UNUSED(bitDepth)
	Q_UNUSED(samplesPerLine)
	Q_UNUSED(linesPerFrame)
	Q_UNUSED(framesPerBuffer)
	Q_UNUSED(buffersPerVolume)
	Q_UNUSED(currentBufferNr)
}

void CameraExtension::processedDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	Q_UNUSED(buffer)
	Q_UNUSED(bitDepth)
	Q_UNUSED(samplesPerLine)
	Q_UNUSED(linesPerFrame)
	Q_UNUSED(framesPerBuffer)
	Q_UNUSED(buffersPerVolume)
	Q_UNUSED(currentBufferNr)
}
