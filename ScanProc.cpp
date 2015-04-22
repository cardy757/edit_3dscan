#include "ScanProc.h"

ScanProc::ScanProc(QObject *parent)
	: QThread(parent)
{
	fstop = false;
	mesh = NULL;
	gla = NULL;
    bGotImage = false;
}

ScanProc::~ScanProc()
{

}

void ScanProc::run()
{
    while (!bGotImage)
    {
        msleep(1000);
    }

    // turn on laser
    // todo

    // get one image
    m_mutexImage.lock();
    Mat matLaserOn = m_image;
    m_mutexImage.unlock();

    // turn off laser
    // todo

    // delay
    msleep(3000);

    // get one image
    m_mutexImage.lock();
    Mat matLaserOff = m_image;
    m_mutexImage.unlock();

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
    bGotImage = false;
}

void ScanProc::updateFrame(Mat &image)
{
    QMutexLocker locker(&m_mutexImage);
    m_image = image;
    bGotImage = true;
}

Mat ScanProc::DetectLaser(Mat &laserOn, Mat &laserOff)
{
    unsigned int cols = laserOn.cols;
    unsigned int rows = laserOn.rows;
    Mat grayLaserOn(rows,cols,CV_8U,Scalar(0));
    Mat grayLaserOff(rows,cols,CV_8U,Scalar(0));
    Mat diffImage(rows,cols,CV_8U,Scalar(0));
    Mat gaussImage(rows,cols,CV_8U,Scalar(0));
    Mat laserImage(rows,cols,CV_8U,Scalar(0));
    Mat result(rows,cols,CV_8UC3,Scalar(0));

    // convert to grayscale
    imshow("laserOn", laserOn);
    msleep(3000);
    destroyAllWindows();

    QImage imglaserOn((uchar*)laserOn.data, laserOn.cols, laserOn.rows, QImage::Format_RGB888);
    imglaserOn.save(QString("laserOn.jpg"),"JPEG");

    imshow("laserOff", laserOff);
    msleep(3000);
    destroyAllWindows();

    QImage imglaserOff((uchar*)laserOff.data, laserOff.cols, laserOff.rows, QImage::Format_RGB888);
    imglaserOff.save(QString("laserOff.jpg"),"JPEG");

    cvtColor(laserOn, grayLaserOn, CV_RGB2GRAY);
    cvtColor(laserOff, grayLaserOff, CV_RGB2GRAY);

    // diff image
    subtract(grayLaserOn,grayLaserOff,diffImage);
    imshow("diff", diffImage);
    msleep(3000);
    destroyAllWindows();

    // apply gaussian
    GaussianBlur(diffImage,gaussImage,Size(15,15),12,12);
    diffImage = diffImage-gaussImage;
    imshow("gaussian", diffImage);
    msleep(3000);
    destroyAllWindows();

    // apply threshold
    double threshold = 10;
    cv::threshold(diffImage, diffImage, threshold, 255, THRESH_TOZERO);
    imshow("threshold", diffImage);
    msleep(3000);
    destroyAllWindows();

    // apply erode
    erode(diffImage,diffImage,Mat(3,3,CV_8U,Scalar(1)) );
    imshow("erode", diffImage);
    msleep(3000);
    destroyAllWindows();

    // apply canny
    Canny(diffImage,diffImage,20,50);
    imshow("canny", diffImage);
    msleep(3000);
    destroyAllWindows();

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

    imshow("laserLine", laserImage);
    msleep(3000);
    destroyAllWindows();

    cvtColor(laserImage, result, CV_GRAY2RGB); //convert back ro rgb
    imshow("laserLine color", result);
    msleep(3000);
    destroyAllWindows();

    QImage imgResult((uchar*)result.data, result.cols, result.rows, QImage::Format_RGB888);
    imgResult.save(QString("result.jpg"),"JPEG");

    return result;
}

