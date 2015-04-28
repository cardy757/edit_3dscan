#include "ScanProc.h"
#include "camerapreviewdlg.h"
#include "geometries.h"
#include "edit_3dscan.h"

ScanProc::ScanProc(QObject *parent)
    : QThread(parent)
{
    fstop = false;
    mesh = NULL;
    gla = NULL;
    pPreviewWnd = NULL;
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
    //Mat matLaserOn = m_image;
#ifdef WIN32
    Mat matLaserOn = imread("input_laser_on.jpg", CV_LOAD_IMAGE_COLOR);
#else
    matLaserOn = imread("/Users/justin/MeshLabSrc/laserOn.jpg");
#endif
    m_mutexImage.unlock();

    // turn off laser
    // todo

    // delay
    //msleep(3000);

    // get one image
    m_mutexImage.lock();
#ifdef WIN32
    //Mat matLaserOff = m_image;
    Mat matLaserOff = imread("input_laser_off.jpg", CV_LOAD_IMAGE_COLOR);
#else
    matLaserOff = imread("/Users/justin/MeshLabSrc/laserOff.jpg");
#endif
    m_mutexImage.unlock();

    Mat matLaserLine = DetectLaser(matLaserOn, matLaserOff);
    MapLaserPointToGlobalPoint(matLaserLine, matLaserOff);

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
    Q_ASSERT(pPreviewWnd);

    unsigned int cols = laserOn.cols;
    unsigned int rows = laserOn.rows;
    Mat grayLaserOn(rows,cols,CV_8U,Scalar(0));
    Mat grayLaserOff(rows,cols,CV_8U,Scalar(0));
    Mat diffImage(rows,cols,CV_8U,Scalar(0));
    Mat gaussImage(rows,cols,CV_8U,Scalar(0));
    Mat laserImage(rows,cols,CV_8U,Scalar(0));
    Mat result(rows,cols,CV_8UC3,Scalar(0));

    //imshow("laserOn", laserOn);
    //msleep(3000);
    //destroyAllWindows();

    //QImage imglaserOn((uchar*)laserOn.data, laserOn.cols, laserOn.rows, QImage::Format_RGB888);
    //imglaserOn = imglaserOn.rgbSwapped();
    //imglaserOn.save(QString("laserOn.jpg"),"JPEG");

    pPreviewWnd->updateFrame(laserOn, "laserOn");
    msleep(3000);

    //imshow("laserOff", laserOff);
    //msleep(3000);
    //destroyAllWindows();

    //QImage imglaserOff((uchar*)laserOff.data, laserOff.cols, laserOff.rows, QImage::Format_RGB888);
    //imglaserOff = imglaserOff.rgbSwapped();
    //imglaserOff.save(QString("laserOff.jpg"),"JPEG");

    pPreviewWnd->updateFrame(laserOff, "laserOff");
    msleep(3000);

    // convert to grayscale
    cvtColor(laserOn, grayLaserOn, CV_BGR2GRAY);
    cvtColor(laserOff, grayLaserOff, CV_BGR2GRAY);

    pPreviewWnd->updateFrame(grayLaserOn, "grayLaserOn");
    msleep(3000);

    //imshow("grayLaserOn", grayLaserOn);
    //msleep(3000);
    //destroyAllWindows();

    pPreviewWnd->updateFrame(grayLaserOff, "grayLaserOff");
    msleep(3000);

    //imshow("grayLaserOff", grayLaserOff);
    //msleep(3000);
    //destroyAllWindows();

    // diff image
    subtract(grayLaserOn,grayLaserOff,diffImage);
    //imshow("diff", diffImage);
    //msleep(3000);
    //destroyAllWindows();
    pPreviewWnd->updateFrame(diffImage, "diffImage");
    msleep(3000);

    /*
    // apply gaussian
    GaussianBlur(diffImage,gaussImage,Size(15,15),12,12);
    diffImage = diffImage-gaussImage;
    //imshow("gaussian", diffImage);
    //msleep(3000);
    //destroyAllWindows();
    pPreviewWnd->updateFrame(diffImage, "gaussian");
    msleep(3000);
    */

    // apply threshold
    double threshold = 10;
    cv::threshold(diffImage, diffImage, threshold, 255, THRESH_TOZERO);
    //imshow("threshold", diffImage);
    //msleep(3000);
    //destroyAllWindows();
    pPreviewWnd->updateFrame(diffImage, "threshold");
    msleep(3000);

    // apply erode
    erode(diffImage,diffImage,Mat(3,3,CV_8U,Scalar(1)) );
    //imshow("erode", diffImage);
    //msleep(3000);
    //destroyAllWindows();
    pPreviewWnd->updateFrame(diffImage, "erode");
    msleep(3000);

    // apply canny
    Canny(diffImage,diffImage,20,50);
    //imshow("canny", diffImage);
    //msleep(3000);
    //destroyAllWindows();
    pPreviewWnd->updateFrame(diffImage, "canny");
    msleep(3000);

    int* edges = new int[cols]; //contains the cols index of the detected edges per row
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
    delete [] edges;

    //imshow("laserLine", laserImage);
    //msleep(3000);
    //destroyAllWindows();
    pPreviewWnd->updateFrame(laserImage, "laserLine");
    msleep(3000);

    cvtColor(laserImage, result, CV_GRAY2RGB); //convert back ro rgb
    //imshow("laserLine color", result);
    //msleep(3000);
    //destroyAllWindows();
    pPreviewWnd->updateFrame(result, "result");
    msleep(3000);

    QImage imgResult((uchar*)result.data, result.cols, result.rows, QImage::Format_RGB888);
    imgResult.save(QString("result.jpg"),"JPEG");

    return laserImage;
}

void ScanProc::MapLaserPointToGlobalPoint(Mat &laserLine, Mat &laserOff)
{
    configuration* config = Edit3DScanPlugin::getConfiguration();
    FSTurntable* turntable = Edit3DScanPlugin::getTurntable();

    //calculate position of laser on the back plane in cv frame
    GlobalPoint gLaserLinePosition = config->getLaserPositionOnBackPlane();
    CvPoint cvLaserLinePosition = convertFSPointToCvPoint(gLaserLinePosition);
    int laserPos = cvLaserLinePosition.x; //const over all y

    unsigned int cols = laserLine.cols;
    unsigned int rows = laserLine.rows;

    int dpiVertical = 1; // 1 for the best resolution

    for(int y = config->UPPER_ANALYZING_FRAME_LIMIT;
            y < laserLine.rows - (config->LOWER_ANALYZING_FRAME_LIMIT);
            y += dpiVertical )
    {
        for(int x = laserLine.cols - 1;
                x >= laserPos + config->ANALYZING_LASER_OFFSET;
                x -= 1)
        {
            //qDebug() << "Pixel value: " << laserLine.at<uchar>(y,x);
            if(laserLine.at<uchar>(y,x) == 255)
            {
                //qDebug() << "found point at x=" << x;
                CvPoint cvNewPoint;
                cvNewPoint.x = x;
                cvNewPoint.y = y;

                //convert to world coordinates withouth depth
                GlobalPoint fsNewPoint = convertCvPointToGlobalPoint(cvNewPoint);
                //cout << fsNewPoint.x << ":" << fsNewPoint.y << ":" << fsNewPoint.z << endl;
                GlobalLine l1 = computeLineFromPoints(config->getCameraPosition(), fsNewPoint);
                GlobalLine l2 = computeLineFromPoints(config->getLaserPosition(), config->getLaserPositionOnBackPlane());

                GlobalPoint i = computeIntersectionOfLines(l1, l2);
                fsNewPoint.x = i.x;
                fsNewPoint.z = i.z;


                //At this point we know the depth=z. Now we need to consider the scaling depending on the depth.
                //First we move our point to a camera centered cartesion system.
                fsNewPoint.y -= (config->getCameraPosition()).y;
                fsNewPoint.y *= ((config->getCameraPosition()).z - fsNewPoint.z)/(config->getCameraPosition()).z;
                //Redo the translation to the box centered cartesion system.
                fsNewPoint.y += (config->getCameraPosition()).y;

                //get color from picture without laser
                unsigned char r = laserOff.at<cv::Vec3b>(y,x)[2];
                unsigned char g = laserOff.at<cv::Vec3b>(y,x)[1];
                unsigned char b = laserOff.at<cv::Vec3b>(y,x)[0];
                fsNewPoint.color = MakeRGBColor(r, g, b);

                //turning new point according to current angle of turntable
                //translate coordinate system to the middle of the turntable
                fsNewPoint.z -= config->TURNTABLE_POS_Z; //7cm radius of turntbale plus 5mm offset from back plane
                GlobalPoint alphaDelta = turntable->getRotation();
                double alphaOld = (float)atan(fsNewPoint.z / fsNewPoint.x);
                double alphaNew = alphaOld + alphaDelta.y * (M_PI / 180.0f);
                double hypotenuse = (float)sqrt(fsNewPoint.x * fsNewPoint.x + fsNewPoint.z * fsNewPoint.z);

                if(fsNewPoint.z < 0 && fsNewPoint.x < 0)
                {
                    alphaNew += M_PI;
                }
                else if(fsNewPoint.z > 0 && fsNewPoint.x < 0)
                {
                    alphaNew -= M_PI;
                }
                fsNewPoint.z = (float)sin(alphaNew) * hypotenuse;
                fsNewPoint.x = (float)cos(alphaNew) * hypotenuse;

                if(fsNewPoint.y > 0.5 && hypotenuse < 7)
                {
                    //eliminate points from the grounds, that are not part of the model
                    //qDebug("adding point");
                    //model->addPointToPointCloud(fsNewPoint);
                }
                break;
            }
        }
    }
}

CvPoint ScanProc::convertFSPointToCvPoint(GlobalPoint fsPoint)
{
    configuration* config = Edit3DScanPlugin::getConfiguration();

    CvSize cvImageSize = cvSize(config->CAM_IMAGE_WIDTH,
                                config->CAM_IMAGE_HEIGHT);
    GlobalSizeFS fsImageSize = MakeGlobalSize(config->FRAME_WIDTH,
                                            config->FRAME_WIDTH * (config->CAM_IMAGE_HEIGHT / config->CAM_IMAGE_WIDTH),
                                            0.0f);
    CvPoint origin;
    origin.x = cvImageSize.width / 2.0f;
    origin.y = cvImageSize.height * config->ORIGIN_Y;

    CvPoint cvPoint;

    cvPoint.x = fsPoint.x * cvImageSize.width / fsImageSize.width;
    cvPoint.y = -fsPoint.y * cvImageSize.height / fsImageSize.height;

    //translate
    cvPoint.x += origin.x;
    cvPoint.y += origin.y;

    return cvPoint;
}


GlobalPoint ScanProc::convertCvPointToGlobalPoint(CvPoint cvPoint)
{
    configuration* config = Edit3DScanPlugin::getConfiguration();
    CvSize cvImageSize = cvSize(config->CAM_IMAGE_WIDTH, config->CAM_IMAGE_HEIGHT);
    GlobalSizeFS fsImageSize = MakeGlobalSize(config->FRAME_WIDTH,
                                            (double)config->FRAME_WIDTH * (double)(config->CAM_IMAGE_HEIGHT / (double)config->CAM_IMAGE_WIDTH),
                                            0.0f);

    //here we define the origin of the cvImage, we place it in the middle of the frame and in the corner of the two perpendiculair planes
    CvPoint origin;
    origin.x = cvImageSize.width / 2.0f;
    origin.y = cvImageSize.height * config->ORIGIN_Y;

    GlobalPoint fsPoint;
    //translate
    cvPoint.x -= origin.x;
    cvPoint.y -= origin.y;
    //scale
    fsPoint.x = cvPoint.x * fsImageSize.width / cvImageSize.width;
    fsPoint.y = -cvPoint.y * fsImageSize.height / cvImageSize.height;
    fsPoint.z = 0.0f;

    return fsPoint;
}
