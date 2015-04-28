#include "camerapreviewdlg.h"

CameraPreviewDlg::CameraPreviewDlg(QWidget *parent)
    : QDockWidget(parent)
{
    setFloating(true);
    setAllowedAreas(Qt::NoDockWidgetArea);

    QSize size(1280, 720);
    setFixedSize(size);
    setMaximumSize(size);
    setMinimumSize(size);

    //build gray scale color table
    for (int i = 0; i < 256; i++)
        m_grayColorTable.push_back(QColor(i, i, i).rgb());
}

void CameraPreviewDlg::setSize(QSize size)
{
    setFixedSize(size);
    setMaximumSize(size);
    setMinimumSize(size);
}

void CameraPreviewDlg::updateFrame(QImage& image, QString caption)
{
    m_image = image;
    m_title = caption;
    update();
}

void CameraPreviewDlg::updateFrame(Mat& mat, QString caption)
{
    QImage image;

    if (mat.type() == CV_8UC3) //bgr image
    {
        image = QImage((uchar*)mat.data, mat.cols, mat.rows, QImage::Format_RGB888);
        image = image.rgbSwapped();
    }
    else if (mat.type() == CV_8UC1) //gray scale image
    {
        image = QImage(mat.data, mat.cols, mat.rows, QImage::Format_Indexed8);
        image.setColorTable(m_grayColorTable);
    }
    else
    {
        Q_ASSERT(0); //other formats are not suported
    }

    //image.save(QString("laserOn.jpg"), "JPEG");
    updateFrame(image, caption);
}

void CameraPreviewDlg::paintEvent(QPaintEvent * event)
{
    setWindowTitle(m_title);

    QRect rect(0, 0, width(), height());
    backingStore()->beginPaint(rect);

    QPaintDevice *device = backingStore()->paintDevice();
    QPainter painter(device);

    painter.drawImage(QRect(0, 0, width(), height()), m_image, QRect(0, 0, m_image.width(), m_image.height()));

    backingStore()->endPaint();
    backingStore()->flush(rect);
}

void CameraPreviewDlg::closeEvent(QCloseEvent * /*event*/)
{
    emit SGN_Closing();
}
