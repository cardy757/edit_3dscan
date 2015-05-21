#ifndef SCANPROC_H
#define SCANPROC_H

#include <QThread>
#include <common/interfaces.h>
#include <meshlab/glarea.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui.hpp>
#include "geometries.h"

class CameraPreviewDlg;
class webcam;

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
    void SetWebcam(webcam *cam) { m_webcam = cam; }
    void Init();

    void run();
    void stop();

signals:
    void scanFinished(void);

private:
    QMutex mutex;
    bool fstop;

    MeshModel *mesh;
    GLArea *gla;
    Mat m_image;
    webcam *m_webcam;

    CameraPreviewDlg* pPreviewWnd;

    Mat DetectLaser(Mat &laserOn, Mat &laserOff);

    //algorithm from ATLAS 3D scanner
    struct LaserRange
    {
        int startCol;
        int endCol;
        int centerCol;
    };
    Mat DetectLaser2(Mat &laserOn, Mat &laserOff);
    int detectBestLaserRange(LaserRange * ranges, int numRanges, int prevLaserCol);
    int detectLaserRangeCenter(const int row, const LaserRange& range, const Mat& diffImage);

    void MapLaserPointToGlobalPoint(Mat &laserLine, Mat &laserOff);
    CvPoint convertGlobalPointToCvPoint(GlobalPoint fsPoint);
    GlobalPoint convertCvPointToGlobalPoint(CvPoint cvPoint);
};

#endif // SCANPROC_H
