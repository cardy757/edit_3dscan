#include "webcam.h"
#include <QtGlobal>

#ifndef WIN32
#include "opencv2/videoio/videoio_c.h" // need for CV_CAP_PROP... ??
#endif

webcam::webcam()
{
    m_iRefCount = 0;
    m_vcap = NULL;
    m_cameraworker = NULL;
    m_bGotImage = false;
}

webcam::~webcam()
{
    //Q_ASSERT(m_iRefCount == 0);

    if (m_cameraworker != NULL)
    {
        m_cameraworker->quit();
        m_cameraworker->wait();
        delete m_cameraworker;
        m_cameraworker = NULL;
    }

    if (m_vcap != NULL)
    {
        m_vcap->release();
        delete m_vcap;
        m_vcap = NULL;
    }
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
        m_vcap->set(CV_CAP_PROP_FRAME_HEIGHT, 960);

        m_cameraworker = new cameraworker(this);
        m_cameraworker->start();
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

        m_cameraworker->quit();
        m_cameraworker->wait();
        delete m_cameraworker;
        m_cameraworker = NULL;

        m_vcap->release();
        delete m_vcap;
        m_vcap = NULL;
        m_bGotImage = false;
    }
}

bool webcam::read(Mat& image)
{
    QMutexLocker locker(&m_mutexImage);
    if (m_iRefCount == 0)
    {
        return false;
    }
    if (!m_bGotImage)
    {
        static int count;
        qDebug("!!!");
        return false;
    }
    image = m_image.clone();

    return true;
}

bool webcam::getImageFromCamera()
{
    if (m_iRefCount == 0)
    {
        return false;
    }
    Mat image;
    if (m_vcap->read(image))
    {
        m_mutexImage.lock();
        m_image = image.clone();
        m_mutexImage.unlock();
        m_bGotImage = true;
        return true;
    }
    return false;
}

