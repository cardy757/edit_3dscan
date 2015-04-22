#include "webcam.h"
#include <QtGlobal>

#ifndef WIN32
#include "opencv2/videoio/videoio_c.h" // need for CV_CAP_PROP... ??
#endif

webcam::webcam(): m_timer(this)
{
    m_iRefCount = 0;
    m_vcap = NULL;
}

webcam::~webcam()
{
    Q_ASSERT(m_iRefCount == 0);
    if (m_vcap != NULL)
    {
        m_vcap->release();
        delete m_vcap;
        m_vcap = NULL;
    }
    m_timer.stop();
}

void webcam::start()
{

    if (m_iRefCount == 0)
    {
        Q_ASSERT(m_vcap == NULL);
        qDebug("enable camera");
        //Initialize opencv video capture device
        m_vcap = new VideoCapture(0); // open the video camera no. 0
        if (!m_vcap->isOpened())
        {
            Q_ASSERT(0);
        }

        m_vcap->set(CV_CAP_PROP_FRAME_WIDTH, 1280);
        m_vcap->set(CV_CAP_PROP_FRAME_HEIGHT, 720);

        //timer to get new frame from camera
        m_timer.start(30);
    }
    m_iRefCount++;
}

void webcam::stop()
{
    m_iRefCount--;

    if (m_iRefCount == 0)
    {
        Q_ASSERT(m_vcap != NULL);
        qDebug("disable camera");

        m_vcap->release();
        delete m_vcap;
        m_vcap = NULL;

        m_timer.stop();
    }
}

bool webcam::read(Mat& image)
{
    if (m_iRefCount == 0)
    {
        return false;
    }
    return m_vcap->read(image);
}
