#ifndef SCANDIALOG_H
#define SCANDIALOG_H

#include <QDockWidget>
#include "ui_scanDialog.h"

class ScanDialog : public QDockWidget
{
	Q_OBJECT

public:
	ScanDialog(QWidget *parent = 0);
	~ScanDialog();

	Ui::scanDialog ui;

Q_SIGNALS:
	void SGN_Closing();

private:
	virtual void closeEvent(QCloseEvent * event);
};

#endif // SCANDIALOG_H
