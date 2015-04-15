#ifndef WEBCAMDLG_H
#define WEBCAMDLG_H

#include <QDockWidget>
#include "ui_webcamdlg.h"

class WebCamDlg : public QDockWidget
{
    Q_OBJECT

public:
    WebCamDlg(QDockWidget *parent = 0);
    ~WebCamDlg();

    void updateFrame(QImage& image);

private:
    Ui::WebCamDlg ui;

Q_SIGNALS:
    void SGN_Closing();

private:
    virtual void closeEvent(QCloseEvent * event);
};

#endif // WEBCAMDLG_H
