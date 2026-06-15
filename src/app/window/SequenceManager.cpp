
#include "SequenceManager.h"

SequenceManager::SequenceManager(QObject *parent) : QObject(parent)
{

}

void SequenceManager::addSequence(SequenceProcessor* sequence){
	if(sequence != nullptr){
		if(!sequences.contains(sequence)){
			this->sequences.append(sequence);
			//this->sequenceNames.append(sequence->getName());
			this->sequenceNames.last().detach(); //force deep copy of appended cameraworker name to avoid possible problems if plugin lives at some point in a thread
		}
	}
}

SequenceProcessor* SequenceManager::getSequenceByName(QString name){
	int index = this->sequenceNames.indexOf(name);
	if(index < 0){
		return nullptr;
	} else {
		return this->sequences.at(index);
	}
}
