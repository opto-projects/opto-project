
#include "systemmanager.h"

#define DELETE_AND_SET_NULL(pointer) if(pointer != nullptr) { delete pointer; pointer = nullptr;};
#define DELETE_AND_SET_NULL_ARRAY(pointer) if(pointer != nullptr) { delete[] pointer; pointer = nullptr;};

SystemManager::SystemManager(QObject *parent) : QObject(parent)
{

}

SystemManager::~SystemManager()
{
	/*while (systems.size() > 0)
	{
		AcquisitionSystem* sys = systems[0];
		DELETE_AND_SET_NULL(sys)
		systems.removeFirst();
	}*/
}

void SystemManager::addSystem(QString name, AcquisitionSystem* cameraworker){
	if(cameraworker != nullptr){
		if(!systems.contains(cameraworker)){
			this->systems.append(cameraworker);
			this->systemNames.append(name);
			this->systemNames.last().detach(); //force deep copy of appended cameraworker name to avoid possible problems if plugin lives at some point in a thread
		}
	}
}

AcquisitionSystem* SystemManager::getSystemByName(QString name){
	int index = this->systemNames.indexOf(name);
	if(index < 0){
		return nullptr;
	} else {
		return this->systems.at(index);
	}
}
