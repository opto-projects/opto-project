

#include "acquisitionsystem.h"

#include "util/OriConfigDlg.h"
#include "util/OriDialogs.h"
#include "util/OriLayouts.h"
//#include "helpers/OriWidgets.h"
//#include "tools/OriSettings.h"
#include "util/OriValueEdit.h"
#include "settings.h"

#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QSettings>


AcquisitionSystem::AcquisitionSystem(QObject* parent) : InterfaceBase(parent) {
	this->acqusitionRunning = false;
}


AcquisitionSystem::~AcquisitionSystem(){
}


