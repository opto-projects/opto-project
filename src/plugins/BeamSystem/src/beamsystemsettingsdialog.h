

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
#include "ui_beamsystemsettingsdialog.h"

struct beamParams {
	QString filePath;
	int depth;
	int width;
	int height;
	int bytes;
	int components;
	int channels;
	int waitTimeUs;
	bool copyFileToRam;
};
Q_DECLARE_METATYPE(beamParams)

class BeamSystemSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	BeamSystemSettingsDialog(QWidget *parent = nullptr);
	~BeamSystemSettingsDialog();

	void setSettings(QVariantMap settings);
	void getSettings(QVariantMap* settings);


private:
	Ui::BeamSystemSettingsDialog* ui;
	beamParams params;

	void initGui();
public:
	void updateGui(beamParams param);
public slots:
	void slot_selectFile();
	void slot_apply();
	void slot_enableGui(bool enable);
	void slot_checkWidthValue();

signals:
	void settingsUpdated(beamParams newParams);
};
