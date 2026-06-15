#ifndef CAMERAEXTENSIONFORM_H
#define CAMERAEXTENSIONFORM_H

#include <QWidget>
#include "cameraextensionparameters.h"

namespace Ui {
class CameraExtensionForm;
}

class CameraExtensionForm : public QWidget
{
	Q_OBJECT

public:
	explicit CameraExtensionForm(QWidget *parent = 0);
	~CameraExtensionForm();

	void setSettings(QVariantMap settings);
	void getSettings(QVariantMap* settings);

	Ui::CameraExtensionForm* ui;

protected:


public slots:
	void connectToSelectedCamera();
	void disconnectCurrentCamera();
	void openSettingsDialog();

private:
	void fillCameraComboBox();
	void connectToCamera(QString deviceName);
	CameraExtensionParameters parameters;

signals:
	void paramsChanged();
	void connectClicked();
	void disconnectClicked();
	void aboutToClose();
	void info(QString);
	void error(QString);

};

#endif // CAMERAEXTENSIONFORM_H
