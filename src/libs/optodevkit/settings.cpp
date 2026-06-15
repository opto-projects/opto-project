
#include "settings.h"


Settings* Settings::settings = nullptr;

Settings::Settings() {
	//if settings file does not exists, copy default settings file with reasonable initial values
	QDir settingsDir(SETTINGS_DIR);
	if(!QFileInfo::exists(SETTINGS_PATH) || !settingsDir.exists(SETTINGS_PATH)){
		settingsDir.mkpath(SETTINGS_DIR);
		bool success = QFile::copy(":default/settings.ini", SETTINGS_PATH);
		if(!success){
			emit error(tr("Could not create settings file in: ") + SETTINGS_PATH); //todo: this signal is not received by any slot since the signal is only connected after the constructor has been executed
		}
		QFile::setPermissions(SETTINGS_PATH, QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadOther | QFileDevice::WriteOther);
	}

	_settings = new QSettings(SETTINGS_PATH, QSettings::IniFormat);
}

Settings* Settings::getInstance() {
	settings = settings != nullptr ? settings : new Settings();
	return settings;
}

Settings::~Settings() {
}

QString Settings::getSettingsFile()
{
	return QString(SETTINGS_PATH);
}

QSettings* Settings::getQSettings()
{
	return _settings;
}

void Settings::setTimestamp(QString timestamp) {
	this->timestamp = timestamp;
	_settings->setValue(TIMESTAMP, this->timestamp);
}

void Settings::storeSettings(QString settingsGroupName, QVariantMap settingsMap) {
	this->storeValues(_settings, settingsGroupName, settingsMap);
}

QVariantMap Settings::getStoredSettings(QString settingsGroupName) {
	QVariantMap settingsMap;
	this->loadValues(_settings, settingsGroupName, &settingsMap); //todo: loadValues should return a QVariantMap instead of passing a QVariantMap by pointer
	return settingsMap;
}

void Settings::copySettingsFile(QString path) {
	QString originPath = SETTINGS_PATH;
	QString destinationPath = path;

	// Remove the destination file if it exists
	if (QFile::exists(destinationPath)) {
		if (!QFile::remove(destinationPath)) {
			emit error(tr("Could not overwrite existing file: ") + destinationPath);
			return;
		}
	}

	// Copy the settings file to the new location
	bool success = QFile::copy(originPath, destinationPath);
	if (!success) {
		emit error(tr("Could not store settings file to: ") + destinationPath);
	}
}

void Settings::storeValues(QSettings* settings, QString groupName, QVariantMap settingsMap) {
	QMapIterator<QString, QVariant> i(settingsMap);
	settings->beginGroup(groupName);
	while (i.hasNext()) {
		i.next();
		settings->setValue(i.key(), i.value());
	}
	settings->endGroup();
}

void Settings::loadValues(QSettings* settings, QString groupName, QVariantMap* settingsMap) {
	settings->beginGroup(groupName);
	QStringList keys = settings->allKeys();
	for (int i = 0; i < keys.size(); i++) {
		settingsMap->insert(keys.at(i), settings->value(keys.at(i)));
	}
	settings->endGroup();
}
