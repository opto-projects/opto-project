

#include "demoextension.h"

DemoPlugin::DemoPlugin(QObject* parent ) : Plugin(parent)
{
	type = PLUGIN_TYPE::EXTENSION;
	name = "DemoExtension";
	descr = "This is a demo Extension intended for developers.";
}
DemoPlugin::~DemoPlugin()
{
}

InterfaceBase* DemoPlugin::getInterfaceInstance(){
	NEW_PLUGININSTANCE(DemoExtension)
}

DemoExtension::DemoExtension(QObject* parent ) : Extension(parent) {
	
	this->displayStyle = SIDEBAR_TAB;
	this->name = "DemoExtension";
	this->toolTip = "This is a demo Extension intended for developers.";

	this->form = new DemoExtensionForm();
	this->widgetDisplayed = false;
	this->isCalculating = false;
	this->active = false;

	connect(this->form, &DemoExtensionForm::parametersUpdated, this, &DemoExtension::setParameters);
}

DemoExtension::~DemoExtension() {
	if(!this->widgetDisplayed){
		//delete this->form;
	}
}

QWidget *DemoExtension::getWidget() {
	this->widgetDisplayed = true;
	return this->form;
}

void DemoExtension::activateExtension() {
	//this method is called by OCTproZ as soon as user activates the extension. If the extension controls hardware components, they can be prepared, activated, initialized or started here.
	this->active = true;
}

void DemoExtension::deactivateExtension() {
	//this method is called by OCTproZ as soon as user deactivates the extension. If the extension controls hardware components, they can be deactivated, resetted or stopped here.
	this->active = false;
}

void DemoExtension::settingsLoaded(QVariantMap settings) {
	//this method is called by OCTproZ and provides a QVariantMap with stored settings.
	this->form->setSettings(settings); //update gui with stored settings
}

void DemoExtension::setParameters(demoParams params) {
	//copy parameters from gui
	this->currentParameters = params;

	//update settingsMap, so parameters can be reloaded into gui at next start of application
	this->form->getSettings(&this->settingsMap);
	emit storeSettings(this->name, this->settingsMap);
}

void DemoExtension::rawDataReceived(void* buffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	//the raw data buffer may be accessed similar to the processed data buffer. See processedDataReceived() below
}

void DemoExtension::processedDataReceived(void* buffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	//check if extension is active, if this slot is already running and if buffer can be accessed
	if(this->active && !this->isCalculating && this->processedGrabbingAllowed){

		//indicate that slot is running
		this->isCalculating = true;

		//check bit depth
		if(bitDepth >= 9 && bitDepth <= 16){

			//cast buffer type according to bit depth (8bit = char, 16bit = uchar, 32bit = uint)
			unsigned short* bufferArray = (unsigned short*)buffer;

			//access buffer for short calculation (sum of pixel values of first line in buffer)
			unsigned int sum = 0;
			for(int i = 0; i < samplesPerLine && this->processedGrabbingAllowed; i++){
				sum += bufferArray[i];
			}
			//Note: if you want to perform larger calculations you should copy the received buffer first and use the copied buffer for your calculations

			this->isCalculating = false;
		}
	}
}
