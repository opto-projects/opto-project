
#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <qvariant.h>
#include <QIcon>

#define NEW_PLUGININSTANCE(InterfaceClass) \
    InterfaceClass* newInst = new InterfaceClass(); \
    newInst->setBasePlugin(this); \
    InterfaceBase * base = qobject_cast<InterfaceBase *>(newInst); \
   /* m_InstList.append(base);*/ \
	return base;



enum PLUGIN_TYPE {
	SYSTEM,
	EXTENSION
};

class InterfaceBase;

class Plugin : public QObject
{
	Q_OBJECT
	
public:
	explicit Plugin(QObject* parent = nullptr) : QObject(parent) {}
	virtual ~Plugin(){
	}

	PLUGIN_TYPE getType() { return this->type;}
	QString getName() { return this->name;}
	QString getDescr() const { return this->descr; }
	
	void setIcon(QIcon icon) { this->icon = icon; }
	QIcon getIcon() const { return this->icon; }

	virtual InterfaceBase* getInterfaceInstance() { return nullptr; }

protected:
	void setType(PLUGIN_TYPE type) { this->type = type; }
	void setName(QString name) { this->name = name; }
	void setDescr(QString descr) { this->descr = descr; }
	
	PLUGIN_TYPE type;
	QString name;
	int version;
	QString descr;
	QIcon icon;
	
	//QList<InterfaceBase* > m_InstList;
};

class InterfaceBase : public QObject
{
	Q_OBJECT

public:
	Plugin* getBasePlugin(void) const { return m_pPlugin.get(); }
	virtual void settingsLoaded(QVariantMap settings) {}

signals:
	void info(QString);
	void error(QString);
	void storeSettings(QString, QVariantMap);

protected:
	// constructor (doc in source)
	InterfaceBase(QObject* parent = nullptr) : QObject(parent), m_pPlugin(nullptr) {}

	// destructor (doc in source)
	virtual ~InterfaceBase() {
	}

	void setBasePlugin(Plugin* base) { m_pPlugin .reset( base); }
private:
	QSharedPointer<Plugin> m_pPlugin;
};

#define Plugin_iid "optochecker.interface"

Q_DECLARE_INTERFACE(Plugin, Plugin_iid)

#endif // PLUGIN_H
