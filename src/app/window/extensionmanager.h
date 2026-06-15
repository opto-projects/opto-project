

#ifndef EXTENSIONMMANAGER_H
#define EXTENSIONMMANAGER_H

#include <QObject>
#include <QList>
#include "optodevkit/devkit.h"

class ExtensionManager : public QObject
{
	Q_OBJECT
public:
	explicit ExtensionManager(QObject *parent = nullptr);
	~ExtensionManager();

	void addExtension(QString name, Extension* plugin);
	Extension* getExtensionByName(QString name);
	QList<Extension*> getExtensions() { return this->extensions; }
	QList<QString> getExtensionNames() { return this->extensionNames; }

private:
	QList<Extension*> extensions;
	QList<QString> extensionNames;

signals:

public slots:
	//void slot_connectExtensionAndSystem(AcquisitionSystem* cameraworker);

};

#endif // EXTENSIONMMANAGER_H
