
#ifndef SYSTEM_EXTENSION_H
#define SYSTEM_EXTENSION_H

#include "plugin.h"

#include <QObject>
#include <QEventLoop>
#include <QDialog>
#include <QDebug>
#include <QThread>
#include <QCloseEvent>


enum DISPLAY_STYLE {
	SIDEBAR_TAB,
	SEPARATE_WINDOW
};


class Extension : public InterfaceBase
{
	Q_OBJECT
public:
	Extension(QObject* parent = nullptr);
	~Extension();

	/*!
	 * \brief getDisplayStyle is called by system to get the display style of the extension. The gui of the extensions may be displayed within the sidebar as a tab or as a seperate window which is always on top.
	 * \return display style of the extension
	 */
	DISPLAY_STYLE getDisplayStyle(){return this->displayStyle;}

	/*!
	 * \brief getToolTip is called by system to get the tooltip of the extension
	 * \return tooltip string which will be displayd in system
	 */
	QString getToolTip(){return this->toolTip;}

	/*!
	 * \brief getWidget needs to be implemented and is called by system to get the GUI of the extension.
	 * \return GUI of extension
	 */
	virtual QWidget* getWidget() = 0;

	/*!
	 * \brief activateExtension needs to be implemented and is called by system as soon as extension is activated by the user. Hardware componets (if controlled by the extension) could be activated here.
	 */
	virtual void activateExtension() = 0;

	/*!
	 * \brief deactivateExtension needs to be implemented and is called by system as soon as extension is deactivated by the user. If the extension executes some long computation tasks, they could be aborted here. Hardware componets (if controlled by the extension) could be deactivated here.
	 */
	virtual void deactivateExtension() = 0;


protected:
	bool rawGrabbingAllowed; ///< Indicates if grabbing raw data is safe. Buffer received by rawDataReceived(...) should not be accessed if rawGrabbingAllowed is false.
	bool processedGrabbingAllowed; ///< Indicates if grabbing proceessed data is safe. Buffer received by processedDataReceived(...) should not be accessed if processedGrabbingAllowed is false.
	QWidget* extensionWidget; ///< GUI of extension
	DISPLAY_STYLE displayStyle; ///< Determines how the extension is displayed to the user. displayStle can be SIDEBAR_TAB or SEPARATE_WINDOW
	QString toolTip; ///< Tooltip that is displayed in system
	QString name;
	QVariantMap settingsMap;

public slots:
	/*!
	 * \brief rawDataReceived is called automatically every time as soon as new raw data is available. This slot can be used to grab raw OCT data, i.e. spectral fringe patterns.
	 * \param buffer array with raw data
	 * \param bitDepth bit depth of each elements
	 * \param samplesPerLine number of elements in a single line (In SDOCT this is usually the number of pixels in the line sensor of the spectrometer. In SSOCT this is usually the number of samples per sweep.)
	 * \param linesPerFrame number of lines in one B-scan
	 * \param framesPerBuffer number of B-scans in buffer
	 * \param buffersPerVolume number of buffers in volume
	 * \param currentBufferNr current buffer id within volume. This is number in the range of 0 to buffersPerVolume-1
	 */
	virtual void rawDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr){}

	/*!
	 * \brief processedDataReceived is called automatically every time as soon as new processed data is available and the "stream processed data to ram" option is activated. This slot can be used to grab processed OCT data, i.e. A-scans.
	 * \param buffer array with processed OCT data
	 * \param bitDepth bit depth of each elements
	 * \param samplesPerLine number of elements in a single A-scan
	 * \param linesPerFrame A-scans per B-scan
	 * \param framesPerBuffer number of B-scans in buffer
	 * \param buffersPerVolume number of buffers in volume
	 * \param currentBufferNr current buffer id within volume. This is number in the range of 0 to buffersPerVolume-1
	 */
	virtual void processedDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr){}

	/*!
	 * \brief enableRawDataGrabbing is called by system to indicate if grabbing of raw data is safe
	 */
	void enableRawDataGrabbing(bool enabled){this->rawGrabbingAllowed = enabled;}

	/*!
	 * \brief enableProcessedDataGrabbing is called by system to indicate if grabbing of processed data is safe
	 */
	void enableProcessedDataGrabbing(bool enabled){this->processedGrabbingAllowed = enabled;}


};



#endif // SYSTEM_EXTENSION_H
