#ifndef SCANPROC_H
#define SCANPROC_H

#include <QThread>
#include <common/interfaces.h>
#include <meshlab/glarea.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui.hpp>

class CameraPreviewDlg;

using namespace cv;

class ScanProc : public QThread
{
    Q_OBJECT

public:
    ScanProc(QObject *parent);
    ~ScanProc();

    void SetMesh(MeshModel *m) { mesh = m; }
    void SetGLArea(GLArea* g) { gla = g; }
    void SetPreviewWnd(CameraPreviewDlg *p) { pPreviewWnd = p; }

    void run();
    void stop();
    void updateFrame(Mat &image);

private:
    QMutex mutex;
    QMutex m_mutexImage;
    bool fstop;

    MeshModel *mesh;
    GLArea *gla;
    Mat m_image;
    bool bGotImage;

    CameraPreviewDlg* pPreviewWnd;

    Mat DetectLaser(Mat &laserOn, Mat &laserOff);
};

#endif // SCANPROC_H
