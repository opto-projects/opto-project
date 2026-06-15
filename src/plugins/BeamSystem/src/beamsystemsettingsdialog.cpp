

#include "beamsystemsettingsdialog.h"
#include "beamsystem.h"

BeamSystemSettingsDialog::BeamSystemSettingsDialog(QWidget *parent)
	: ui(new Ui::BeamSystemSettingsDialog) //QDialog(parent)
{
	qRegisterMetaType<beamParams >("beamParams");

	ui->setupUi(this);
	initGui();

}

BeamSystemSettingsDialog::~BeamSystemSettingsDialog()
{
}

void BeamSystemSettingsDialog::setSettings(QVariantMap settings){
	this->ui->lineEdit->setText(settings.value(FILEPATH).toString());
	this->ui->spinBox_bitDepth->setValue(settings.value(BITDEPTH).toInt());
	this->ui->spinBox_width->setValue(settings.value(WIDTH).toInt());
	this->ui->spinBox_height->setValue(settings.value(HEIGHT).toInt());
	this->ui->spinBox_waitTime->setValue(settings.value(WAITTIME).toInt());
	this->ui->checkBox_copyFileToRam->setChecked(settings.value(COPY_TO_RAM).toBool());
	this->slot_apply();
}

void BeamSystemSettingsDialog::getSettings(QVariantMap* settings) {
	settings->insert(FILEPATH, this->ui->lineEdit->text());
	settings->insert(BITDEPTH, this->ui->spinBox_bitDepth->value());
	settings->insert(WIDTH, this->ui->spinBox_width->value());
	settings->insert(HEIGHT, this->ui->spinBox_height->value());
	settings->insert(WAITTIME, this->ui->spinBox_waitTime->value());
	settings->insert(COPY_TO_RAM, this->ui->checkBox_copyFileToRam->isChecked());
}

void BeamSystemSettingsDialog::initGui(){
	this->setWindowTitle(tr("Still Image System Settings"));
	connect(this->ui->pushButton_selectFile, &QPushButton::clicked, this, &BeamSystemSettingsDialog::slot_selectFile);
	connect(this->ui->okButton, &QPushButton::clicked, this, &BeamSystemSettingsDialog::slot_apply);
	connect(this->ui->spinBox_width, &QSpinBox::editingFinished, this, &BeamSystemSettingsDialog::slot_checkWidthValue);
}

void BeamSystemSettingsDialog::slot_selectFile(){
	QString currentPath = this->ui->lineEdit->text();
	QString standardLocation = this->params.filePath.size() == 0 ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : this->params.filePath;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raw Image"), standardLocation, tr("Images (*.png *.tif *.pgm *.jpg);;All Files (*.*)"));
	if (fileName == "") {
		fileName = currentPath;
	}
	this->ui->lineEdit->setText(fileName);
}

void BeamSystemSettingsDialog::updateGui(beamParams param)
{
	this->ui->spinBox_bitDepth->setValue(param.depth);
	this->ui->spinBox_width->setValue(param.width);
	this->ui->spinBox_height->setValue(param.height);
}

void BeamSystemSettingsDialog::slot_apply() {
	this->params.filePath = this->ui->lineEdit->text();
	this->params.depth = this->ui->spinBox_bitDepth->value();
	this->params.width = this->ui->spinBox_width->value();
	this->params.height = this->ui->spinBox_height->value();
	this->params.waitTimeUs = this->ui->spinBox_waitTime->value();
	this->params.copyFileToRam = this->ui->checkBox_copyFileToRam->isChecked();
	emit settingsUpdated(this->params);
}

void BeamSystemSettingsDialog::slot_enableGui(bool enable){
	this->ui->lineEdit->setEnabled(enable);
	this->ui->pushButton_selectFile->setEnabled(enable);
	this->ui->spinBox_bitDepth->setEnabled(enable);
	this->ui->spinBox_width->setEnabled(enable);
	this->ui->spinBox_height->setEnabled(enable);
	//this->ui->spinBox_waitTime->setEnabled(enable);  //waitTime does not need to be disabled. It can be safely changed during processing
	this->ui->checkBox_copyFileToRam->setEnabled(enable);
}

void BeamSystemSettingsDialog::slot_checkWidthValue(){
	int width = this->ui->spinBox_width->value();
	if(width % 2 != 0){
		this->ui->spinBox_width->setValue(width-1);
	}
}
