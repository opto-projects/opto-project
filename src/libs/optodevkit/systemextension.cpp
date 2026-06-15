

#include "systemextension.h"


Extension::Extension(QObject* parent) : InterfaceBase(parent) {
	this->rawGrabbingAllowed = true;
	this->processedGrabbingAllowed = true;
	this->extensionWidget = nullptr;
	this->displayStyle = SIDEBAR_TAB;
}

Extension::~Extension() {
}
