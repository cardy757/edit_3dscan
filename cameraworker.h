#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QThread>
#include <QTimer>

class webcam;

class cameraworker : public QThread
{
    Q_OBJECT
public:
    cameraworker(webcam *cam);
    ~cameraworker();

private slots:
    void onTimeout();

private:
    void run();
    webcam *m_webcam;
};

#endif // CAMERAWORKER_H
