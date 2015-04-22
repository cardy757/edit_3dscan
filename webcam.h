#ifndef WEBCAM_H
#define WEBCAM_H

#include <opencv2/highgui/highgui.hpp>
#include <QObject>
#include <QTimer>

using namespace cv;

class webcam: public QObject
{
public:
    webcam();
    ~webcam();

    void start();
    void stop();
    bool read(Mat& image);

    QTimer m_timer;

private:
    int m_iRefCount;
    VideoCapture *m_vcap;
};

#endif // WEBCAM_H
