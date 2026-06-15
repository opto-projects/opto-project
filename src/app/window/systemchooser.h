

#ifndef SYSTEMCHOOSER_H
#define SYSTEMCHOOSER_H

#include <QObject>
#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QGridLayout>
#include <QLayout>
#include <QPushButton>


class SystemChooser : public QDialog
{
	Q_OBJECT
public:
	SystemChooser();
	~SystemChooser();

	QString selectSystem(QList<QString> systems);

private:
	QString selectedSystem;
	QLayout* layout;
	QListWidget* listView;
	QLabel* label;
	QPushButton* pushButton_ok;

	void populate(QList<QString> systems);

public slots:
	void onOkClicked();
	void onDoubleClicked(QListWidgetItem* item);
};

#endif //SYSTEMCHOOSER_H
