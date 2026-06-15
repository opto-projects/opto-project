#include "camerasettingsdialog.h"
#include <QCameraImageProcessing>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QVideoFrame>
#include <QCameraZoomControl>


CameraSettingsDialog::CameraSettingsDialog(QCamera* existingCamera, QList<QCameraViewfinderSettings> existingSupportedSettings, QWidget* parent)
	: QDialog(parent), 
	  camera(existingCamera),
	  supportedSettings(existingSupportedSettings)
{
	setupUi();
}


CameraSettingsDialog::~CameraSettingsDialog() {

}

void CameraSettingsDialog::setupUi() {
	this->setWindowTitle(tr("Camera Settings"));
	this->layout = new QVBoxLayout(this);
	QCameraImageProcessing *imageProcessing = camera->imageProcessing();

	if (imageProcessing->isAvailable()) {
		addCameraImageProcessingControl(tr("Brightness"), QCameraImageProcessingControl::Brightness);
		addCameraImageProcessingControl(tr("Contrast"), QCameraImageProcessingControl::Contrast);
		addCameraImageProcessingControl(tr("Saturation"), QCameraImageProcessingControl::Saturation);
		addCameraImageProcessingControl(tr("Sharpening"), QCameraImageProcessingControl::Sharpening);
	}
	this->addResolutionAndFpsControl();
	this->addPixelFormatControl();
	this->addColorFilterControl();
	this->addZoomControl();

	QPushButton *closeButton = new QPushButton(tr("Close"), this);
	connect(closeButton, &QPushButton::clicked, this, &CameraSettingsDialog::accept);
	this->layout->addWidget(closeButton);
	
	this->layout->setSizeConstraint(QLayout::SetMinimumSize);
	this->setMinimumSize(layout->minimumSize());
}

void CameraSettingsDialog::addCameraImageProcessingControl(const QString &labelText, QCameraImageProcessingControl::ProcessingParameter param) {
	QHBoxLayout *hLayout = new QHBoxLayout();
	QLabel *label = new QLabel(labelText, this);
	this->standardizeLabelSize(label);
	QSlider *slider = new QSlider(Qt::Horizontal, this);
	QDoubleSpinBox *doubleSpinBox = new QDoubleSpinBox(this);

	qreal minValue = -1.0, maxValue = 1.0, stepValue = 0.01;
	slider->setRange(static_cast<int>(minValue * 100), static_cast<int>(maxValue * 100));
	doubleSpinBox->setRange(minValue, maxValue);
	doubleSpinBox->setSingleStep(stepValue);

	QCameraImageProcessing *imageProcessing = this->camera->imageProcessing();
	if (imageProcessing) {
		//set initial value based on the parameter
		qreal initialValue = 0;
		switch (param) {
			case QCameraImageProcessingControl::Brightness:
				initialValue = imageProcessing->brightness();
				break;
			case QCameraImageProcessingControl::Contrast:
				initialValue = imageProcessing->contrast();
				break;
			case QCameraImageProcessingControl::Saturation:
				initialValue = imageProcessing->saturation();
				break;
			case QCameraImageProcessingControl::Sharpening:
				initialValue = imageProcessing->sharpeningLevel();
				break;
			default:
				break;
		}
		//scale and set the initial slider value
		slider->setValue(static_cast<int>(initialValue * 100));
		doubleSpinBox->setValue(initialValue);

		connect(slider, &QSlider::valueChanged, this, [doubleSpinBox](int value) {
			doubleSpinBox->setValue(value / 100.0);
		});
		connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [slider](double value) {
			slider->setValue(static_cast<int>(value * 100));
		});
		connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [param, imageProcessing](double value) {
			switch (param) {
				case QCameraImageProcessingControl::Brightness:
					imageProcessing->setBrightness(value);
					break;
				case QCameraImageProcessingControl::Contrast:
					imageProcessing->setContrast(value);
					break;
				case QCameraImageProcessingControl::Saturation:
					imageProcessing->setSaturation(value);
					break;
				case QCameraImageProcessingControl::Sharpening:
					imageProcessing->setSharpeningLevel(value);
					break;
				default:
					break;
			}
		});

	} else {
		slider->setEnabled(false);
		doubleSpinBox->setEnabled(false);
	}

	hLayout->addWidget(label);
	hLayout->addWidget(slider);
	hLayout->addWidget(doubleSpinBox);
	this->layout->addLayout(hLayout);
}

void CameraSettingsDialog::addColorFilterControl() {
	QCameraImageProcessing *imageProcessing = this->camera->imageProcessing();
	if (!imageProcessing->isAvailable()) {
		return;
	}
	QComboBox *comboBox = new QComboBox(this);
	
	const QMetaObject &mo = QCameraImageProcessing::staticMetaObject;
	int index = mo.indexOfEnumerator("ColorFilter");
	QMetaEnum metaEnum = mo.enumerator(index);

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QCameraImageProcessing::ColorFilter filter = static_cast<QCameraImageProcessing::ColorFilter>(metaEnum.value(i));
		//check if the color filter is supported
		if (imageProcessing->isColorFilterSupported(filter)) {
			QString name = metaEnum.valueToKey(metaEnum.value(i));
			comboBox->addItem(name, QVariant(metaEnum.value(i)));
		}
	}

	//set the current selection based on the camera's current color filter
	QCameraImageProcessing::ColorFilter currentFilter = imageProcessing->colorFilter();
	int currentFilterIndex = comboBox->findData(static_cast<int>(currentFilter));
	if (currentFilterIndex != -1) {
		comboBox->setCurrentIndex(currentFilterIndex);
	}

	//connect the combo box selection change to set the new color filter
	connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [imageProcessing, comboBox](int index) {
		int filterValue = comboBox->itemData(index).toInt();
		QCameraImageProcessing::ColorFilter newFilter = static_cast<QCameraImageProcessing::ColorFilter>(filterValue);
		if (imageProcessing->isColorFilterSupported(newFilter)) {
			imageProcessing->setColorFilter(newFilter);
		}
	});

	//only show gui elements -if there are any color filters that can be changed
	if (comboBox->count() > 0) {
		QHBoxLayout *hLayout = new QHBoxLayout();
		QLabel *label = new QLabel(tr("Color Filter"), this);
		this->standardizeLabelSize(label);
		hLayout->addWidget(label);
		hLayout->addWidget(comboBox);
		layout->addLayout(hLayout);
	} else {
		delete comboBox;
	}
}

void CameraSettingsDialog::addPixelFormatControl() {
	QComboBox *comboBox = new QComboBox(this);

	//get supported viewfinder settings to prevent duplicates in the combo box
	const QList<QCameraViewfinderSettings> supportedSettings = this->supportedSettings;
	QSet<QVideoFrame::PixelFormat> seenFormats;

	for (const QCameraViewfinderSettings &settings : supportedSettings) {
		if (!seenFormats.contains(settings.pixelFormat())) {
			seenFormats.insert(settings.pixelFormat());
			comboBox->addItem(pixelFormatToString(settings.pixelFormat()), QVariant::fromValue(settings.pixelFormat()));
		}
	}

	// Connect the combo box selection change to update the camera's viewfinder settings
	connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, comboBox]() {
		auto pixelFormat = comboBox->currentData().value<QVideoFrame::PixelFormat>();
		QCameraViewfinderSettings viewfinderSettings = this->camera->viewfinderSettings();
		viewfinderSettings.setPixelFormat(pixelFormat);
		this->camera->setViewfinderSettings(viewfinderSettings);
	});

	//only show gui elements -if there are any color filters that can be changed
	if (comboBox->count() > 0) {
		QHBoxLayout *hLayout = new QHBoxLayout();
		QLabel *label = new QLabel(tr("Pixel Format"), this);
		this->standardizeLabelSize(label);
		hLayout->addWidget(label);
		hLayout->addWidget(comboBox);
		this->layout->addLayout(hLayout);
	} else {
		delete comboBox;
	}
}

void CameraSettingsDialog::addResolutionAndFpsControl() {
	QComboBox *comboBox = new QComboBox(this);

	//get resolution/fps combinations and populate combobox
	const QList<QCameraViewfinderSettings> supportedSettings = this->supportedSettings;
	if(!supportedSettings.isEmpty()){
		for(const QCameraViewfinderSettings &settings : supportedSettings) {
			QString resolutionFps = QString("%1x%2, %3 FPS")
										.arg(settings.resolution().width())
										.arg(settings.resolution().height())
										.arg(settings.minimumFrameRate());
			comboBox->addItem(resolutionFps, QVariant::fromValue(settings));
		}
	} else {
		//on some tested systems (Jetson Nano) supportedViewfinderSettings() is empty. In this case we generate some common resolution fps combinations and hope that these are supported by the used camera system
		struct ResolutionFps { int width; int height; qreal fps; };
		QList<ResolutionFps> standardSettings = {
			{320, 240, 20},
			{640, 480, 20},
			{640, 480, 5},
			{960, 720, 20},
			{1280, 720, 20}
		};

		for (const auto &setting : standardSettings) {
			QCameraViewfinderSettings standardSetting;
			standardSetting.setResolution(setting.width, setting.height);
			standardSetting.setMinimumFrameRate(setting.fps);
			standardSetting.setMaximumFrameRate(setting.fps);

			QString resolutionFps = QString("%1x%2, %3 FPS").arg(setting.width).arg(setting.height).arg(setting.fps);
			comboBox->addItem(resolutionFps, QVariant::fromValue(standardSetting));
		}
	}
	
	if (comboBox->count() > 0) {
		this->addCurrentResolutionSettingsToComboBox(comboBox, this->camera);
		//generate gui elements
		QHBoxLayout *hLayout = new QHBoxLayout();
		QLabel *label = new QLabel(tr("Resolution"), this);
		this->standardizeLabelSize(label);
		hLayout->addWidget(label);
		hLayout->addWidget(comboBox);
		layout->addLayout(hLayout);
		//apply changed settings
		connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, comboBox]() {
			QCameraViewfinderSettings settings = comboBox->currentData().value<QCameraViewfinderSettings>();
			this->camera->setViewfinderSettings(settings);
		});
	} else {
		delete comboBox; 
	}
}

void CameraSettingsDialog::addCurrentResolutionSettingsToComboBox(QComboBox* comboBox, const QCamera* camera) {
	QCameraViewfinderSettings currentSettings = camera->viewfinderSettings();
	QString currentSettingsText = QString("%1x%2, %3 FPS")
										.arg(currentSettings.resolution().width())
										.arg(currentSettings.resolution().height())
										.arg(currentSettings.minimumFrameRate());

	bool found = false;
	for (int i = 0; i < comboBox->count(); ++i) {
		if (comboBox->itemText(i) == currentSettingsText) {
			found = true;
			comboBox->setCurrentIndex(i);
			break;
		}
	}

	if (!found) {
		comboBox->insertItem(0, currentSettingsText, QVariant::fromValue(currentSettings));
		comboBox->setCurrentIndex(0);
	}
}

void CameraSettingsDialog::addZoomControl() {
	QCameraZoomControl *zoomControl = this->camera->service()->requestControl<QCameraZoomControl*>();
	if (!zoomControl) {
		//zoom control is not supported
		return;
	}

	//optical Zoom Slider, minimum zoom is 1.0
	if (zoomControl->maximumOpticalZoom() > 1.0) {
		QSlider *opticalZoomSlider = new QSlider(Qt::Horizontal, this);
		opticalZoomSlider->setRange(10, static_cast<int>(zoomControl->maximumOpticalZoom() * 10)); // Multiplying by 10 for finer control
		opticalZoomSlider->setValue(static_cast<int>(zoomControl->currentOpticalZoom() * 10));
		connect(opticalZoomSlider, &QSlider::valueChanged, this, [zoomControl](int value) {
			zoomControl->zoomTo(value / 10.0, zoomControl->currentDigitalZoom());
		});

		QHBoxLayout *opticalZoomLayout = new QHBoxLayout();
		QLabel* label = new QLabel(tr("Optical Zoom"), this);
		this->standardizeLabelSize(label);
		opticalZoomLayout->addWidget(label);
		opticalZoomLayout->addWidget(opticalZoomSlider);
		layout->addLayout(opticalZoomLayout);
	}

	//digital Zoom Slider, minimum zoom is 1.0
	if (zoomControl->maximumDigitalZoom() > 1.0) {
		QSlider *digitalZoomSlider = new QSlider(Qt::Horizontal, this);
		digitalZoomSlider->setRange(10, static_cast<int>(zoomControl->maximumDigitalZoom() * 10)); // Multiplying by 10 for finer control
		digitalZoomSlider->setValue(static_cast<int>(zoomControl->currentDigitalZoom() * 10));
		connect(digitalZoomSlider, &QSlider::valueChanged, this, [zoomControl](int value) {
			zoomControl->zoomTo(zoomControl->currentOpticalZoom(), value / 10.0);
		});

		QHBoxLayout *digitalZoomLayout = new QHBoxLayout();
		QLabel* label = new QLabel(tr("Digital Zoom"), this);
		this->standardizeLabelSize(label);
		digitalZoomLayout->addWidget(label);
		digitalZoomLayout->addWidget(digitalZoomSlider);
		layout->addLayout(digitalZoomLayout);
	}
}

QString CameraSettingsDialog::pixelFormatToString(QVideoFrame::PixelFormat format) {
	switch(format) {
		case QVideoFrame::Format_Invalid: return QLatin1String("Invalid");
		case QVideoFrame::Format_ARGB32: return QLatin1String("ARGB32");
		case QVideoFrame::Format_ARGB32_Premultiplied: return QLatin1String("ARGB32 Premultiplied");
		case QVideoFrame::Format_RGB32: return QLatin1String("RGB32");
		case QVideoFrame::Format_RGB24: return QLatin1String("RGB24");
		case QVideoFrame::Format_RGB565: return QLatin1String("RGB565");
		case QVideoFrame::Format_RGB555: return QLatin1String("RGB555");
		case QVideoFrame::Format_ARGB8565_Premultiplied: return QLatin1String("ARGB8565 Premultiplied");
		case QVideoFrame::Format_BGRA32: return QLatin1String("BGRA32");
		case QVideoFrame::Format_BGRA32_Premultiplied: return QLatin1String("BGRA32 Premultiplied");;
		case QVideoFrame::Format_BGR32: return QLatin1String("BGR32");
		case QVideoFrame::Format_BGR24: return QLatin1String("BGR24");
		case QVideoFrame::Format_BGR565: return QLatin1String("BGR565");
		case QVideoFrame::Format_BGR555: return QLatin1String("BGR555");
		case QVideoFrame::Format_BGRA5658_Premultiplied: return QLatin1String("BGRA5658 Premultiplied");
		case QVideoFrame::Format_AYUV444: return QLatin1String("AYUV444");
		case QVideoFrame::Format_AYUV444_Premultiplied: return QLatin1String("AYUV444 Premultiplied");
		case QVideoFrame::Format_YUV444: return QLatin1String("YUV444");
		case QVideoFrame::Format_YUV420P: return QLatin1String("YUV420P");
		case QVideoFrame::Format_YV12: return QLatin1String("YV12");
		case QVideoFrame::Format_UYVY: return QLatin1String("UYVY");
		case QVideoFrame::Format_YUYV: return QLatin1String("YUYV");
		case QVideoFrame::Format_NV12: return QLatin1String("NV12");
		case QVideoFrame::Format_NV21: return QLatin1String("NV21");
		case QVideoFrame::Format_IMC1: return QLatin1String("IMC1");
		case QVideoFrame::Format_IMC2: return QLatin1String("IMC2");
		case QVideoFrame::Format_IMC3: return QLatin1String("IMC3");
		case QVideoFrame::Format_IMC4: return QLatin1String("IMC4");
		case QVideoFrame::Format_Y8: return QLatin1String("Y8");
		case QVideoFrame::Format_Y16: return QLatin1String("Y16");
		case QVideoFrame::Format_Jpeg: return QLatin1String("Jpeg");
		case QVideoFrame::Format_CameraRaw: return QLatin1String("CameraRaw");
		case QVideoFrame::Format_AdobeDng: return QLatin1String("AdobeDng");
		case QVideoFrame::Format_User: return QLatin1String("User");
		default: return QLatin1String("Unknown");
	}
}

void CameraSettingsDialog::standardizeLabelSize(QLabel* label) {
	//set minimum label width based on width of longest text string
	QFontMetrics metrics(label->font());
	int textWidth = metrics.horizontalAdvance("Optical Zoom");
	label->setMinimumWidth(textWidth );
}
