
#include "extensionmanager.h"


ExtensionManager::ExtensionManager(QObject *parent) : QObject(parent)
{
}

ExtensionManager::~ExtensionManager()
{
	qDeleteAll(this->extensions);
	this->extensions.clear();
	this->extensionNames.clear();
}

void ExtensionManager::addExtension(Extension* extension) {
	if (extension != nullptr) {
		if (!extensions.contains(extension)) {
			this->extensions.append(extension);
			this->extensionNames.append(extension->getName());
			this->extensionNames.last().detach(); //force deep copy of appended extension name to avoid possible problems if plugin lives at some point in a thread
		}
	}
}

Extension* ExtensionManager::getExtensionByName(QString name) {
	int index = this->extensionNames.indexOf(name);
	return index == -1 ? nullptr : this->extensions.at(index);
}
