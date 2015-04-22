#ifndef SCANPROC_H
#define SCANPROC_H

#include <QThread>
#include <common/interfaces.h>
#include <meshlab/glarea.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui.hpp>

class ScanProc : public QThread
{
	Q_OBJECT

public:
	ScanProc(QObject *parent);
	~ScanProc();

	void SetMesh(MeshModel *m) { mesh = m; }
	void SetGLArea(GLArea* g) { gla = g; }

	void run();
	void stop();
    void updateFrame(QImage& image);

private:
	QMutex mutex;
    QMutex m_mutexImage;
	bool fstop;

	MeshModel *mesh;
	GLArea *gla;
    QImage m_image;

    cv::Mat DetectLaser(cv::Mat &laserOn, cv::Mat &laserOff);
};

#endif // SCANPROC_H
