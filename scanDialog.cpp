#include "scanDialog.h"

ScanDialog::ScanDialog(QWidget *parent)
	: QDockWidget(parent)
{
	ui.setupUi(this);
	this->setFloating(true);
	this->setAllowedAreas(Qt::NoDockWidgetArea);

	QPoint p = parent->mapToGlobal(QPoint(0, 0));
	this->setGeometry(p.x() + (parent->width() - width()- 30), p.y() + 100, width(), height());
}

ScanDialog::~ScanDialog()
{

}

void ScanDialog::closeEvent(QCloseEvent * /*event*/)
{
	emit SGN_Closing();
}
