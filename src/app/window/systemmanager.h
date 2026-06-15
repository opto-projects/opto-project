

#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <QObject>
#include <QList>
#include "optodevkit/devkit.h"

class SystemManager : public QObject
{
	Q_OBJECT
public:
	explicit SystemManager(QObject *parent = nullptr);
	~SystemManager();

	void addSystem(QString name, AcquisitionSystem* plugin);
	AcquisitionSystem* getSystemByName(QString name);
	QList<AcquisitionSystem*> getSystems(){return this->systems;}
	QList<QString> getSystemNames(){return this->systemNames;}

private:
	QList<AcquisitionSystem*> systems;
	QList<QString> systemNames;

signals:

public slots:
};

#endif // SYSTEMMANAGER_H
