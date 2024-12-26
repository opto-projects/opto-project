/**
**  This file is part of OCTproZ.
**  OCTproZ is an open source software for processig of optical
**  coherence tomography (OCT) raw data.
**  Copyright (C) 2019-2022 Miroslav Zabic
**
**  OCTproZ is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program. If not, see http://www.gnu.org/licenses/.
**
****
** Author:	Miroslav Zabic
** Contact:	zabic
**			at
**			spectralcode.de
****
**/

#include "controlpanel.h"
#include <QCoreApplication>


ControlPanel3D::ControlPanel3D(QWidget *parent) : QWidget(parent){
	this->panel = new QWidget(parent);
	QPalette pal;
	pal.setColor(QPalette::Background, QColor(32,32,32,128));
	this->panel->setAutoFillBackground(true);
	this->panel->setPalette(pal);
	this->widgetLayout = new QHBoxLayout(this);
	this->widgetLayout->addWidget(this->panel);

	this->spinBoxSmoothFactor = new QSpinBox(this->panel);
	this->doubleSpinBoxAlphaExponent = new QDoubleSpinBox(this->panel);
	this->doubleSpinBoxDepthWeight = new QDoubleSpinBox(this->panel);
	this->doubleSpinBoxThreshold = new QDoubleSpinBox(this->panel);
	this->doubleSpinBoxStepLength = new QDoubleSpinBox(this->panel);
	this->stringBoxModes = new StringSpinBox(this->panel);
	this->checkBoxShading = new QCheckBox(this->panel);
	this->labelSmoothFactor = new QLabel(tr("Smooth Factor: "), this->panel);
	this->labelAlphaExponent = new QLabel(tr("Alpha Exponent: "), this->panel);
	this->labelDepthWeight = new QLabel(tr("Depth Weight: "), this->panel);
	this->labelThreshold = new QLabel(tr("Threshold:"), this->panel);
	this->labelStepLength = new QLabel(tr("Ray Step: "), this->panel);
	this->labelMode = new QLabel(tr("Mode:"), this->panel);
	this->checkBoxShading->setText(tr("Shading"));

	this->doubleSpinBoxStretchX = new QDoubleSpinBox(this->panel);
	this->labelStretchX = new QLabel(tr("Stretch X:"), this->panel);
	this->doubleSpinBoxStretchY = new QDoubleSpinBox(this->panel);
	this->labelStretchY = new QLabel(tr("Stretch Y:"), this->panel);
	this->doubleSpinBoxStretchZ = new QDoubleSpinBox(this->panel);
	this->labelStretchZ = new QLabel(tr("Stretch Z:"), this->panel);

	this->doubleSpinBoxGamma = new QDoubleSpinBox(this->panel);
	this->labelGamma = new QLabel(tr("Gamma:"), this->panel);

	this->checkBoxLUT = new QCheckBox(this->panel);
	this->checkBoxLUT->setText(tr("LUT"));

	this->toolButtonOpenLUT = new QToolButton(this->panel);
	this->toolButtonOpenLUT->setText(tr("Load LUT..."));
	this->toolButtonOpenLUT->setAutoRaise(true);
	connect(this->toolButtonOpenLUT, &QToolButton::clicked, this, &ControlPanel3D::openLUTDialog);

	this->toolButtonMore = new QToolButton(this->panel);
	this->toolButtonMore->setArrowType(Qt::DownArrow);
	this->toolButtonMore->setAutoRaise(true);


	this->layout = new QGridLayout(this->panel);
	this->layout->setContentsMargins(0,0,0,0);
	this->layout->setVerticalSpacing(1);
	this->layout->addWidget(this->labelMode, 0, 0, 1, 1, Qt::AlignRight);
	this->layout->addWidget(this->stringBoxModes, 0, 1, 1, 1);
	this->layout->addWidget(this->labelThreshold, 1, 0, 1, 1, Qt::AlignRight);
	this->layout->addWidget(this->doubleSpinBoxThreshold, 1, 1, 1, 1);

	QHBoxLayout* hLayoutDepthWeight = createHBoxLayout(this->labelDepthWeight, this->doubleSpinBoxDepthWeight);
	QHBoxLayout* hLayoutAlphaExponent = createHBoxLayout(this->labelAlphaExponent, this->doubleSpinBoxAlphaExponent);
	QHBoxLayout* hLayoutSmoothFactor = createHBoxLayout(this->labelSmoothFactor, this->spinBoxSmoothFactor);
	QHBoxLayout* hLayoutStepLength = createHBoxLayout(this->labelStepLength, this->doubleSpinBoxStepLength);
	QHBoxLayout* hLayoutGamma = createHBoxLayout(this->labelGamma, this->doubleSpinBoxGamma);
	QHBoxLayout* hLayoutLUT = createHBoxLayout(this->checkBoxLUT, this->toolButtonOpenLUT);


	this->layout->addLayout(hLayoutSmoothFactor, 0, 3, 1, 1);
	this->layout->addLayout(hLayoutDepthWeight, 1, 3, 1, 1);

	this->layout->addLayout(hLayoutAlphaExponent, 0, 3, 1, 1);
	this->layout->setColumnStretch(2, 10);
	this->layout->addWidget(this->toolButtonMore, 1, 2, 1, 1, Qt::AlignCenter);

	this->layout->addWidget(this->checkBoxShading, 1, 3, 1, 1, Qt::AlignCenter);

	this->layout->addWidget(this->labelStretchX, 3, 0, 1, 1, Qt::AlignRight);
	this->layout->addWidget(this->doubleSpinBoxStretchX,3, 1, 1, 1);
	this->layout->addWidget(this->labelStretchY, 4, 0, 1, 1, Qt::AlignRight);
	this->layout->addWidget(this->doubleSpinBoxStretchY,4, 1, 1, 1);
	this->layout->addWidget(this->labelStretchZ, 5, 0, 1, 1, Qt::AlignRight);
	this->layout->addWidget(this->doubleSpinBoxStretchZ, 5, 1, 1, 1);

	this->layout->addLayout(hLayoutLUT, 3, 3, 1, 1);
	this->layout->addLayout(hLayoutStepLength ,4, 3, 1, 1);
	this->layout->addLayout(hLayoutGamma, 5, 3, 1, 1);

	this->doubleSpinBoxStepLength->setMinimum(0.001);
	this->doubleSpinBoxStepLength->setMaximum(10.0);
	this->doubleSpinBoxStepLength->setSingleStep(0.001);
	this->doubleSpinBoxStepLength->setValue(0.01);
	this->doubleSpinBoxStepLength->setDecimals(3);

	this->doubleSpinBoxThreshold->setMinimum(0.0);
	this->doubleSpinBoxThreshold->setMaximum(1);
	this->doubleSpinBoxThreshold->setSingleStep(0.01);
	this->doubleSpinBoxThreshold->setValue(0.5);
	this->doubleSpinBoxThreshold->setDecimals(2);

	this->doubleSpinBoxDepthWeight->setMinimum(0);
	this->doubleSpinBoxDepthWeight->setMaximum(1);
	this->doubleSpinBoxDepthWeight->setSingleStep(0.01);
	this->doubleSpinBoxDepthWeight->setValue(0.5);
	this->doubleSpinBoxDepthWeight->setDecimals(2);

	this->spinBoxSmoothFactor->setMinimum(0);
	this->spinBoxSmoothFactor->setMaximum(3);
	this->spinBoxSmoothFactor->setValue(0);

	this->doubleSpinBoxAlphaExponent->setMinimum(0.1);
	this->doubleSpinBoxAlphaExponent->setMaximum(10.0);
	this->doubleSpinBoxAlphaExponent->setSingleStep(0.1);
	this->doubleSpinBoxAlphaExponent->setValue(2.0);
	this->doubleSpinBoxAlphaExponent->setDecimals(1);


	this->checkBoxShading->setChecked(false);

	//todo: refactor this class. avoid repeating code
	this->doubleSpinBoxStretchX->setMaximum(9999);
	this->doubleSpinBoxStretchX->setMinimum(0.1);
	this->doubleSpinBoxStretchX->setSingleStep(0.1);
	this->doubleSpinBoxStretchX->setValue(1.0);

	this->doubleSpinBoxStretchY->setMaximum(9999);
	this->doubleSpinBoxStretchY->setMinimum(0.1);
	this->doubleSpinBoxStretchY->setSingleStep(0.1);
	this->doubleSpinBoxStretchY->setValue(1.0);

	this->doubleSpinBoxStretchZ->setMaximum(9999);
	this->doubleSpinBoxStretchZ->setMinimum(0.1);
	this->doubleSpinBoxStretchZ->setSingleStep(0.1);
	this->doubleSpinBoxStretchZ->setValue(1.0);

	this->doubleSpinBoxGamma->setMaximum(10.00);
	this->doubleSpinBoxGamma->setMinimum(0.1);
	this->doubleSpinBoxGamma->setSingleStep(0.1);
	this->doubleSpinBoxGamma->setValue(2.2);

	connect(this->toolButtonMore, &QToolButton::clicked, this, &ControlPanel3D::toggleExtendedView);

	this->enableExtendedView(false);
	this->updateDisplayParameters();
	this->findGuiElements();
	this->connectGuiToSettingsChangedSignal();
	connect(this, &ControlPanel3D::settingsChanged, this, &ControlPanel3D::updateDisplayParameters);
}


ControlPanel3D::~ControlPanel3D()
{

}

void ControlPanel3D::setModes(QStringList modes) {
	this->stringBoxModes->setStrings(modes);
}

void ControlPanel3D::eraseLUTFileName() {
	this->params.lutFileName = "";
}


GLWindow3DParams ControlPanel3D::getParams() {
	this->params.extendedViewEnabled = this->extendedView;
	this->params.displayMode = this->stringBoxModes->getText();
	this->params.displayModeIndex = this->stringBoxModes->getIndex();
	this->params.smoothFactor = this->spinBoxSmoothFactor->value();
	this->params.depthWeight = this->doubleSpinBoxDepthWeight->value();
	this->params.threshold = this->doubleSpinBoxThreshold->value();
	this->params.rayMarchStepLength = this->doubleSpinBoxStepLength->value();
	this->params.stretchX = this->doubleSpinBoxStretchX->value();
	this->params.stretchY = this->doubleSpinBoxStretchY->value();
	this->params.stretchZ = this->doubleSpinBoxStretchZ->value();
	this->params.gamma = this->doubleSpinBoxGamma->value();
	this->params.alphaExponent = this->doubleSpinBoxAlphaExponent->value();
	this->params.shading = this->checkBoxShading->isChecked();
	this->params.lutEnabled = this->checkBoxLUT->isChecked();

	return this->params;
}

void ControlPanel3D::updateDisplayParameters() {
	emit displayParametersChanged(this->getParams());

//todo: better way to add gui elements for new display modes
	if(this->params.displayMode == "DMIP"){
		this->labelDepthWeight->setVisible(true);
		this->doubleSpinBoxDepthWeight->setVisible(true);
		this->labelSmoothFactor->setVisible(false);
		this->spinBoxSmoothFactor->setVisible(false);
		this->labelAlphaExponent->setVisible(true);
		this->doubleSpinBoxAlphaExponent->setVisible(true);
		this->checkBoxShading->setVisible(false);

	} else if(this->params.displayMode == "Isosurface"){
		this->labelDepthWeight->setVisible(false);
		this->doubleSpinBoxDepthWeight->setVisible(false);
		this->labelSmoothFactor->setVisible(true);
		this->spinBoxSmoothFactor->setVisible(true);
		this->labelAlphaExponent->setVisible(false);
		this->doubleSpinBoxAlphaExponent->setVisible(false);
		this->checkBoxShading->setVisible(false);

	} else if(this->params.displayMode == "MIDA" || this->params.displayMode == "Alpha blending" || this->params.displayMode == "OCT Depth"){
		this->labelDepthWeight->setVisible(false);
		this->doubleSpinBoxDepthWeight->setVisible(false);
		this->labelSmoothFactor->setVisible(false);
		this->spinBoxSmoothFactor->setVisible(false);
		this->labelAlphaExponent->setVisible(true);
		this->doubleSpinBoxAlphaExponent->setVisible(true);
		this->checkBoxShading->setVisible(true);

	} else if(this->params.displayMode == "X-ray"){
		this->labelDepthWeight->setVisible(false);
		this->doubleSpinBoxDepthWeight->setVisible(false);
		this->labelSmoothFactor->setVisible(false);
		this->spinBoxSmoothFactor->setVisible(false);
		this->labelAlphaExponent->setVisible(true);
		this->doubleSpinBoxAlphaExponent->setVisible(true);
		this->checkBoxShading->setVisible(false);

	}else{
		this->labelDepthWeight->setVisible(false);
		this->doubleSpinBoxDepthWeight->setVisible(false);
		this->labelSmoothFactor->setVisible(false);
		this->spinBoxSmoothFactor->setVisible(false);
		this->labelAlphaExponent->setVisible(true);
		this->doubleSpinBoxAlphaExponent->setVisible(true);
		this->checkBoxShading->setVisible(false);
	}
}

void ControlPanel3D::toggleExtendedView(){
	this->enableExtendedView(!this->extendedView);
}

void ControlPanel3D::connectGuiToSettingsChangedSignal() {
	foreach(auto element,this->spinBoxes) {
		connect(element, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->doubleSpinBoxes) {
		connect(element, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->stringSpinBoxes) {
		connect(element, &StringSpinBox::indexChanged, this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->lineEdits) {
		connect(element, &QLineEdit::textChanged, this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->checkBoxes) {
		connect(element, &QCheckBox::clicked, this, &ControlPanel3D::settingsChanged);
	}
}

void ControlPanel3D::disconnectGuiFromSettingsChangedSignal() {
	foreach(auto element,this->spinBoxes) {
		disconnect(element, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->doubleSpinBoxes) {
		disconnect(element, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->stringSpinBoxes) {
		disconnect(element, &StringSpinBox::indexChanged, this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->lineEdits) {
		disconnect(element, &QLineEdit::textChanged, this, &ControlPanel3D::settingsChanged);
	}
	foreach(auto element,this->checkBoxes) {
		disconnect(element, &QCheckBox::clicked, this, &ControlPanel3D::settingsChanged);
	}
}

QHBoxLayout *ControlPanel3D::createHBoxLayout(QWidget *firstWidget, QWidget *secondWidget) {
	QHBoxLayout* hLayout = new QHBoxLayout();
	hLayout->addWidget(firstWidget);
	hLayout->addWidget(secondWidget);
	return hLayout;
}

void ControlPanel3D::setParams(GLWindow3DParams params) {
	this->disconnectGuiFromSettingsChangedSignal();
	this->params = params;
	this->enableExtendedView(params.extendedViewEnabled);
	this->stringBoxModes->setIndex(params.displayModeIndex); //todo: get index from QStringList that contains the rendering mode names
	this->spinBoxSmoothFactor->setValue(params.smoothFactor);
	this->doubleSpinBoxDepthWeight->setValue(params.depthWeight);
	this->doubleSpinBoxThreshold->setValue(params.threshold);
	this->doubleSpinBoxStepLength->setValue(params.rayMarchStepLength);
	this->doubleSpinBoxStretchX->setValue(params.stretchX);
	this->doubleSpinBoxStretchY->setValue(params.stretchY);
	this->doubleSpinBoxStretchZ->setValue(params.stretchZ);
	this->doubleSpinBoxGamma->setValue(params.gamma);
	this->doubleSpinBoxAlphaExponent->setValue(params.alphaExponent);
	this->checkBoxShading->setChecked(params.shading);
	this->checkBoxLUT->setChecked(params.lutEnabled);
	this->updateDisplayParameters();
	this->connectGuiToSettingsChangedSignal();
}

void ControlPanel3D::openLUTDialog() {
	emit dialogAboutToOpen();
	QCoreApplication::processEvents();
	QString filters("(*.png);;(*.jpg);;(*.bmp)");
	QString defaultFilter("(*.png)");
	QDir dir;
	if(this->params.lutFileName != ""){
		dir = QFileInfo(this->params.lutFileName).absoluteDir();
	}else{
		dir = QCoreApplication::applicationDirPath() + "/luts/";
	}
	const QString fileName = QFileDialog::getOpenFileName(this, tr("Select image file with lookup table..."), dir.path(), filters, &defaultFilter);
	emit dialogClosed();
	if(fileName == ""){
		return;
	}
	if(fileName == this->params.lutFileName){
		emit info(tr("The selected LUT is already in use!"));
	} else {
		QImage lut(fileName);
		if(lut.isNull()){
			emit error(tr("Could not load LUT! File: ") + fileName);
			this->params.lutFileName = "";
			return;
		}
		this->params.lutFileName = fileName;
		this->params.lutEnabled = true;
		this->checkBoxLUT->setChecked(true);
		emit lutSelected(lut);
		emit info(tr("LUT for volume rendering loaded! File used: ") + fileName);
	}
}

void ControlPanel3D::enableExtendedView(bool enable) {
	this->extendedView = enable;
	enable ? this->toolButtonMore->setArrowType(Qt::UpArrow) : this->toolButtonMore->setArrowType(Qt::DownArrow);
	this->doubleSpinBoxStretchX->setVisible(enable);
	this->doubleSpinBoxStretchY->setVisible(enable);
	this->doubleSpinBoxStretchZ->setVisible(enable);
	this->labelStretchX->setVisible(enable);
	this->labelStretchY->setVisible(enable);
	this->labelStretchZ->setVisible(enable);
	this->labelStepLength->setVisible(enable);
	this->doubleSpinBoxStepLength->setVisible(enable);
	this->labelGamma->setVisible(enable);
	this->doubleSpinBoxGamma->setVisible(enable);
	this->checkBoxLUT->setVisible(enable);
	this->toolButtonOpenLUT->setVisible(enable);
	//this->checkBoxShading->setVisible(enable && (this->params.displayMode == "MIDA" || this->params.displayMode == "Alpha blending"));
}

void ControlPanel3D::findGuiElements() {
	this->lineEdits = this->findChildren<QLineEdit*>();
	this->checkBoxes = this->findChildren<QCheckBox*>();
	this->doubleSpinBoxes = this->findChildren<QDoubleSpinBox*>();
	this->spinBoxes = this->findChildren<QSpinBox*>();
	this->stringSpinBoxes = this->findChildren<StringSpinBox*>();
	this->comboBoxes = this->findChildren<QComboBox*>();
}
