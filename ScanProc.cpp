#include "ScanProc.h"

ScanProc::ScanProc(QObject *parent)
	: QThread(parent)
{
	fstop = false;
	mesh = NULL;
	gla = NULL;
}

ScanProc::~ScanProc()
{

}

void ScanProc::run()
{
    // turn on laser
    // todo

    // get one image
    m_mutexImage.lock();
    QImage imageLaserOn = m_image;
    m_mutexImage.unlock();
    imageLaserOn.save(QString("LaserOn.jpg"),"JPEG");

    // turn off laser
    // todo

    // delay
    msleep(3000);

    // get one image
    m_mutexImage.lock();
    QImage imageLaserOff = m_image;
    m_mutexImage.unlock();
    imageLaserOff.save(QString("LaserOff.jpg"),"JPEG");

    // detec laser
    cv::Mat matLaserOn(imageLaserOn.height(),
                       imageLaserOn.width(),
                CV_8UC3,
                (uchar*)imageLaserOn.bits(), imageLaserOn.bytesPerLine());
    cv::Mat matLaserOff(imageLaserOff.height(),
                imageLaserOff.width(),
                CV_8UC3,
                (uchar*)imageLaserOff.bits(), imageLaserOff.bytesPerLine());
    DetectLaser(matLaserOn, matLaserOff);

	if (mesh && gla)
	{
		mutex.lock();
		fstop = false;
		mutex.unlock();

		//?? synchronization problem
		mesh->Clear();

		int step = 90, c = 0;
		double radius = 0.5, theta = 0, phi = 0;

		vcg::tri::Allocator<CMeshO>::AddVertices(mesh->cm, step * step);
		for (int i = 0; i < step; i++)
		{
			theta = M_PI / 180 * i * 4;
			for (int j = 0; j < step; j++)
			{
				phi = M_PI / 180 * j * 4;
				mesh->cm.vert[c].P() = vcg::Point3f(
					radius * sin(theta) * cos(phi),
					radius * sin(theta) * sin(phi),
					radius * cos(theta)
					);
				mesh->cm.vert[c].C() = vcg::Color4b(255, 0, 0, 255);
				c++;
			}
			mesh->meshModified() = true;
			gla->update();

            msleep(300);

			QMutexLocker locker(&mutex);
			if (fstop) break;
		}
		mesh->UpdateBoxAndNormals();
		mesh->meshModified() = true;
		gla->update();
	}
}

void ScanProc::stop()
{
	QMutexLocker locker(&mutex);
    fstop = true;
}

void ScanProc::updateFrame(QImage& image)
{
    QMutexLocker locker(&m_mutexImage);
    m_image = image;
}

cv::Mat ScanProc::DetectLaser(cv::Mat &laserOn, cv::Mat &laserOff)
{
    unsigned int cols = laserOn.cols;
    unsigned int rows = laserOn.rows;
    cv::Mat grayLaserOn(rows,cols,CV_8U,cv::Scalar(0));
    cv::Mat grayLaserOff(rows,cols,CV_8U,cv::Scalar(0));
    cv::Mat diffImage(rows,cols,CV_8U,cv::Scalar(0));
    cv::Mat gaussImage(rows,cols,CV_8U,cv::Scalar(0));
    cv::Mat laserImage(rows,cols,CV_8U,cv::Scalar(0));
    cv::Mat result(rows,cols,CV_8UC3,cv::Scalar(0));

    // convert to grayscale
    cv::cvtColor(laserOn, grayLaserOn, CV_RGB2GRAY);
    cv::cvtColor(laserOff, grayLaserOff, CV_RGB2GRAY);

    // diff image
    cv::subtract(grayLaserOn,grayLaserOff,diffImage);
    cv::namedWindow("laserLine");
    cv::imshow("diff", diffImage);
    msleep(3000);
    cv::destroyAllWindows();

    // apply gaussian
    cv::GaussianBlur(diffImage,gaussImage,cv::Size(15,15),12,12);
    diffImage = diffImage-gaussImage;
    cv::imshow("gaussian", diffImage);
    msleep(3000);
    cv::destroyAllWindows();

    // apply threshold
    double threshold = 10;
    cv::threshold(diffImage,diffImage,threshold,255,cv::THRESH_TOZERO);
    cv::imshow("threshold", diffImage);
    msleep(3000);
    cv::destroyAllWindows();

    // apply erode
    cv::erode(diffImage,diffImage,cv::Mat(3,3,CV_8U,cv::Scalar(1)) );
    cv::imshow("erode", diffImage);
    msleep(3000);
    cv::destroyAllWindows();

    // apply canny
    cv::Canny(diffImage,diffImage,20,50);
    cv::imshow("canny", diffImage);
    msleep(3000);
    cv::destroyAllWindows();

    int edges[cols]; //contains the cols index of the detected edges per row
    for(unsigned int y = 0; y < rows; y++)
    {
        //reset the detected edges
        for(unsigned int j=0; j < cols; j++)
        {
            edges[j] = -1;
        }

        int j=0;
        for(unsigned int x = 0; x < cols; x++)
        {
            if(diffImage.at<uchar>(y,x) > 250)
            {
                edges[j] = x;
                j++;
            }
        }

        //iterate over detected edges, take middle of two edges
        for(unsigned int j = 0; j < cols - 1; j += 2)
        {
            if(edges[j] >= 0 && edges[j+1] >= 0 && edges[j+1] - edges[j] < 40)
            {
                int middle = (int)(edges[j] + edges[j+1]) / 2;
                laserImage.at<uchar>(y,middle) = 255;
            }
        }
    }

    cv::imshow("laserLine", laserImage);
    msleep(3000);
    cv::destroyAllWindows();

    cv::cvtColor(laserImage, result, CV_GRAY2RGB); //convert back ro rgb
    cv::imshow("laserLine color", result);
    msleep(3000);
    cv::destroyAllWindows();

    QImage imgResult((uchar*)result.data, result.cols, laserImage.rows, QImage::Format_RGB888);
    imgResult.save(QString("result.jpg"),"JPEG");

    return result;
}

