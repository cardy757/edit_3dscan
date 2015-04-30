#include "cameraworker.h"
#include "webcam.h"

cameraworker::cameraworker(webcam* cam)
{
    m_webcam = cam;
}

cameraworker::~cameraworker()
{
}

void cameraworker::onTimeout()
{
    m_webcam->getImageFromCamera();
}

void cameraworker::run()
{
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), this, SLOT(onTimeout()), Qt::DirectConnection);
    timer.start(33);

    exec();
}
