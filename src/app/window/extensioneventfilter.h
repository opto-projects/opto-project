
#pragma once

#include <QWidget>
#include <QEvent>
#include "optodevkit/devkit.h"

class ExtensionEventFilter : public QObject
{
	Q_OBJECT
public:
	explicit ExtensionEventFilter(QObject *parent);
	void setExtension(Extension* extension){this->extension = extension;}

private:
	Extension* extension;

protected:
	bool eventFilter(QObject* object, QEvent* event) override;


signals:
	void extensionWidgetClosed(Extension* extension);


public slots:
};
