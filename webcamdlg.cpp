#include "webcamdlg.h"

WebCamDlg::WebCamDlg(QDockWidget *parent)
    : QDockWidget(parent)
{
    ui.setupUi(this);
    this->setFloating(true);
    this->setAllowedAreas(Qt::NoDockWidgetArea);

    setFixedSize(640, 360);
    ui.openGLWidget->setFixedSize(640, 360);
}

WebCamDlg::~WebCamDlg()
{

}

void WebCamDlg::updateFrame(QImage& image)
{
    ui.openGLWidget->updateFrame(image);
}

void WebCamDlg::closeEvent(QCloseEvent * /*event*/)
{
    emit SGN_Closing();
}