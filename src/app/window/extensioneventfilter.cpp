

#include "extensioneventfilter.h"

ExtensionEventFilter::ExtensionEventFilter(QObject *parent) : QObject(parent)
{
	this->extension = nullptr;
}

bool ExtensionEventFilter::eventFilter(QObject *object, QEvent *event){
	if (event->type() == QEvent::Close){
		emit extensionWidgetClosed(this->extension);
	}
	return QObject::eventFilter(object, event);
}
