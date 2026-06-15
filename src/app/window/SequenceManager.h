

#ifndef SEQUENCEMANAGER_H
#define SEQUENCEMANAGER_H

#include <QObject>
#include <QList>

class SequenceProcessor;
class SequenceManager : public QObject
{
	Q_OBJECT
public:
	explicit SequenceManager(QObject *parent = nullptr);

	void addSequence(SequenceProcessor* sequence);
	SequenceProcessor* getSequenceByName(QString name);
	QList<SequenceProcessor*> getSequence(){return this->sequences;}
	QList<QString> getSequenceNames(){return this->sequenceNames;}

private:
	QList<SequenceProcessor*> sequences;
	QList<QString> sequenceNames;

signals:

public slots:
};

#endif // SYSTEMMANAGER_H
