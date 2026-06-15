

#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QTextBrowser>
#include <QPushButton>
#include <QCoreApplication>
#include <QLabel>
#include <QDebug>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
	//window settings
	this->setWindowTitle(tr("About OptoChecker"));
	this->setWindowOpacity(0.90);
	this->setMinimumWidth(768);
	this->setMinimumHeight(256);

	//setup left area of about dialog with logo
	QVBoxLayout *vLayoutLeft = new QVBoxLayout();
	vLayoutLeft->setSpacing(0);
	QLabel* labelWithLogo = new QLabel(this);
	QPixmap pix(":/aboutdata/logo.png");
	labelWithLogo->setPixmap(pix);
	//logo ÉèÖÃ´óÐ¡
    labelWithLogo->setFixedSize(pix.width()/2, pix.height()/2);
    labelWithLogo->setScaledContents(true);
	vLayoutLeft->addWidget(labelWithLogo, 0, Qt::AlignHCenter);
	QLabel* labelWithAuthorInfo = new QLabel(this);
	labelWithAuthorInfo->setText("Author: optochecker.com\n" \
				  "Contact: contact" \
				  "@" \
				  "optochecker.com\n\n" \
				  "Version: " + qApp->applicationVersion());
	labelWithAuthorInfo->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	vLayoutLeft->addWidget(labelWithAuthorInfo);

	//setup right area of about dialog with tabwidget
	QHBoxLayout* hLayoutTop = new QHBoxLayout();
	hLayoutTop->addLayout(vLayoutLeft);
	vLayoutLeft->setContentsMargins(2, 15, 2, 0);
	QTabWidget *tabWidget = new QTabWidget(this);
	hLayoutTop->addWidget(tabWidget);

	//about
	QString aboutText = tr("<b>optochecker</b> is an application software for optics image processing and visualization. A plug-in cameraworker enables the integration of acquisition systems and software modules.");
	QTextEdit* aboutTextEdit = new QTextEdit(this);
	aboutTextEdit->setReadOnly(true);
	aboutTextEdit->setText(aboutText);
	tabWidget->addTab(aboutTextEdit, tr("About"));

	//license
	QString licenseText = ("All rights reserved.");

	QFile licenseFile(":/aboutdata/LICENSE");
	Q_ASSERT(licenseFile.exists());
	licenseFile.open(QIODevice::ReadOnly);
	QByteArray licenseByteArray = licenseFile.readAll();
	licenseText.append(QString::fromUtf8(licenseByteArray));
	licenseText.append("</pre></body></html>");
	QTextEdit* licenceTextEdit = new QTextEdit(this);
	licenceTextEdit->setReadOnly(true);
	licenceTextEdit->setText(licenseText);
	tabWidget->addTab(licenceTextEdit, tr("License"));

	//credits
	/*QString creditsText = "";
	QTextEdit* creditsTextEdit = new QTextEdit(this);
	creditsTextEdit->setReadOnly(true);

	QFile creditsFile(":/aboutdata/credits.txt");
	Q_ASSERT(creditsFile.exists());
	creditsFile.open(QIODevice::ReadOnly);
	QByteArray creditsByteArray = creditsFile.readAll();
	creditsText.append(QString::fromUtf8(creditsByteArray));
	creditsTextEdit->setText(creditsText);
	tabWidget->addTab(creditsTextEdit, tr("Credits"));*/

	//third-party software components
	QString thirdpartyText = tr("<html><body><h2>Third party components used by OptoChecker:</h2><ul>");
	QTextEdit* thirdpartyTextEdit = new QTextEdit(this);
	thirdpartyTextEdit->setReadOnly(true);
	thirdpartyTextEdit->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);

	QFile thirdpartyFile(":/aboutdata/thirdparty.txt");
	Q_ASSERT(thirdpartyFile.exists());
	thirdpartyFile.open(QIODevice::ReadOnly);
	QByteArray thirdpartyByteArray = thirdpartyFile.readAll();
	thirdpartyText.append(QString::fromUtf8(thirdpartyByteArray));
	thirdpartyText.append("</ul></body></html>");
	thirdpartyTextEdit->setText(thirdpartyText);
	tabWidget->addTab(thirdpartyTextEdit, tr("Third-party components"));

	//close buttom
	QPushButton *closeButton = new QPushButton(tr("Close"));
	connect(closeButton, &QPushButton::clicked, this, &AboutDialog::close);
	QHBoxLayout *hLayoutBottom = new QHBoxLayout;
	hLayoutBottom->setMargin(6);
	hLayoutBottom->addStretch(10);
	hLayoutBottom->addWidget(closeButton);

	//main layout
	QVBoxLayout *vLayoutMain = new QVBoxLayout(this);
	int defaultMargin = vLayoutMain->margin() +10;
	int defaultSpacing = vLayoutMain->spacing();
	vLayoutMain->setSpacing(defaultSpacing+10);
	vLayoutMain->setContentsMargins(defaultMargin, defaultMargin, defaultMargin, 0);
	vLayoutMain->addLayout(hLayoutTop);
	vLayoutMain->addLayout(hLayoutBottom);

	connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &AboutDialog::easterEgg);

	labelWithLogo->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	connect(labelWithLogo, &QLabel::customContextMenuRequested, this, &AboutDialog::easterEgg);
}

AboutDialog::~AboutDialog() {
}
