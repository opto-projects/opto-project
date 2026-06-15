

#pragma once

#define SYSNAME "sys_name"
#define FILEPATH "file_path"
#define BITDEPTH "depth"
#define WIDTH "width"
#define HEIGHT "height"
#define WAITTIME "wait_time"
#define COPY_TO_RAM "copy_file_to_ram"


#include <qstandardpaths.h>
#include <qvariant.h>
#include <QDialog>
#include <QString>
#include <QFileDialog>
#include "ui_stillimagesystemsettingsdialog.h"

struct imageParams {
	QString filePath;
	int depth;
	int width;
	int height;
	int waitTimeUs;
	bool copyFileToRam;
};

class StillImageSystemSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	StillImageSystemSettingsDialog(QWidget *parent = nullptr);
	~StillImageSystemSettingsDialog();

	void setSettings(QVariantMap settings);
	void getSettings(QVariantMap* settings);


private:
	Ui::StillImageSystemSettingsDialog* ui;
	imageParams params;

	void initGui();
public:
	void updateGui(imageParams param);
public slots:
	void slot_selectFile();
	void slot_apply();
	void slot_enableGui(bool enable);
	void slot_checkWidthValue();

signals:
	void settingsUpdated(imageParams newParams);
};
