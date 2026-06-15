
#include "systemchooser.h"


SystemChooser::SystemChooser() : QDialog()
{
	this->selectedSystem = "";
	layout = new QGridLayout(this);
	label = new QLabel("The following systems are available:");
	layout->addWidget(label);
	listView = new QListWidget();
	listView->setSelectionMode(QAbstractItemView::SingleSelection);
	layout->addWidget(listView);
	pushButton_ok = new QPushButton("Select");
	layout->addWidget(pushButton_ok);
	connect(this->pushButton_ok, &QPushButton::clicked, this, &SystemChooser::onOkClicked);
	connect(this->listView, &QListWidget::itemDoubleClicked, this, &SystemChooser::onDoubleClicked);
}

SystemChooser::~SystemChooser()
{
	delete this->layout;
	delete this->label;
	delete this->listView;
	delete this->pushButton_ok;
}

void SystemChooser::populate(QList<QString> systems) {
	this->listView->clear();
	for(auto cameraworker : systems){
		this->listView->addItem(new QListWidgetItem(cameraworker));
	}
}

QString SystemChooser::selectSystem(QList<QString> systems) {
	this->populate(systems);
	if(this->listView->count() > 0){
		this->listView->setCurrentItem(this->listView->item(0));
	}
	this->exec();
	return selectedSystem;
}

void SystemChooser::onOkClicked() {
	if(!listView->selectedItems().isEmpty()){
		QListWidgetItem* selectedItem = listView->selectedItems()[0];
		this->selectedSystem = selectedItem->text();
	}
	this->close();
	this->listView->clear();
}

void SystemChooser::onDoubleClicked(QListWidgetItem *item) {
	this->selectedSystem = item->text();
	this->close();
	this->listView->clear();
}
