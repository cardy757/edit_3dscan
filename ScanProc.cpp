#include "ScanProc.h"
#include "camerapreviewdlg.h"
#include "geometries.h"
#include "edit_3dscan.h"

#define LASER_LINE_DETECTION_PREVIEW_DELAY 100

ScanProc::ScanProc(QObject *parent)
    : QThread(parent)
{
    fstop = false;
    mesh = NULL;
    gla = NULL;
    pPreviewWnd = NULL;
    m_webcam = NULL;
}

ScanProc::~ScanProc()
{

}

void ScanProc::Init()
{
    fstop = false;
}

void ScanProc::run()
{
    while (!m_webcam->read(m_image))
    {
        msleep(1000);
    }

    if (mesh == NULL || gla == NULL)
    {
        Q_ASSERT(false);
        return;
    }
    mesh->Clear();

    Edit3DScanPlugin::turntable->enable();
    Edit3DScanPlugin::turntable->setDirection(FS_DIRECTION_CCW);

    double degreesPerStep = 360.0f/200.0f/16.0f; //the size of a microstep
    degreesPerStep = 10; // turn faster for test

    for(double i = 0; i < 360; i += degreesPerStep)
    {
        // turn on laser
        Edit3DScanPlugin::arduino->writeChar(MC_TURN_LASER_ON);
        msleep(3000);
        // get one image
        m_webcam->read(m_image);
        Mat matLaserOn = m_image;
#ifdef WIN32
        //matLaserOn = imread("input_laser_on.jpg", CV_LOAD_IMAGE_COLOR);
#else
        //matLaserOn = imread("/Users/justin/MeshLabSrc/laserOn.jpg");
#endif

        // turn off laser
        Edit3DScanPlugin::arduino->writeChar(MC_TURN_LASER_OFF);

        // delay
        msleep(1000);

        // get one image
        m_webcam->read(m_image);
        Mat matLaserOff = m_image;
#ifdef WIN32
        //matLaserOff = imread("input_laser_off.jpg", CV_LOAD_IMAGE_COLOR);
#else
        //matLaserOff = imread("/Users/justin/MeshLabSrc/laserOff.jpg");
#endif
        //Mat matLaserLine = DetectLaser(matLaserOn, matLaserOff);
        Mat matLaserLine = DetectLaser2(matLaserOn, matLaserOff);
        MapLaserPointToGlobalPoint(matLaserLine, matLaserOff);

        //turn turntable a step
        Edit3DScanPlugin::turntable->turnNumberOfDegrees(degreesPerStep);

        if (fstop)
        {
            break;
        }
    }

    Edit3DScanPlugin::turntable->disable();

    emit scanFinished();

#if 0
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
#endif
}

void ScanProc::stop()
{
    QMutexLocker locker(&mutex);
    fstop = true;
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

    //QImage imglaserOn((uchar*)laserOn.data, laserOn.cols, laserOn.rows, QImage::Format_RGB888);
    //imglaserOn = imglaserOn.rgbSwapped();
    //imglaserOn.save(QString("laserOn.jpg"),"JPEG");

    pPreviewWnd->updateFrame(laserOn, "laserOn");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    //QImage imglaserOff((uchar*)laserOff.data, laserOff.cols, laserOff.rows, QImage::Format_RGB888);
    //imglaserOff = imglaserOff.rgbSwapped();
    //imglaserOff.save(QString("laserOff.jpg"),"JPEG");

    pPreviewWnd->updateFrame(laserOff, "laserOff");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    // convert to grayscale
    cvtColor(laserOn, grayLaserOn, CV_BGR2GRAY);
    cvtColor(laserOff, grayLaserOff, CV_BGR2GRAY);

    pPreviewWnd->updateFrame(grayLaserOn, "grayLaserOn");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    pPreviewWnd->updateFrame(grayLaserOff, "grayLaserOff");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    // diff image
    subtract(grayLaserOn,grayLaserOff,diffImage);
    pPreviewWnd->updateFrame(diffImage, "diffImage");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    /*
    // apply gaussian
    GaussianBlur(diffImage,gaussImage,Size(15,15),12,12);
    diffImage = diffImage-gaussImage;
    pPreviewWnd->updateFrame(diffImage, "gaussian");
    msleep(LASER_LINE_DETECTION_DELAY);
    */

    // apply threshold
    double threshold = 10;
    cv::threshold(diffImage, diffImage, threshold, 255, THRESH_TOZERO);
    pPreviewWnd->updateFrame(diffImage, "threshold");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    // apply erode
    erode(diffImage,diffImage,Mat(3,3,CV_8U,Scalar(1)) );
    pPreviewWnd->updateFrame(diffImage, "erode");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    // apply canny
    Canny(diffImage,diffImage,20,50);
    pPreviewWnd->updateFrame(diffImage, "canny");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

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

    pPreviewWnd->updateFrame(laserImage, "laserLine");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    //cvtColor(laserImage, result, CV_GRAY2RGB); //convert back ro rgb
    //QImage imgResult((uchar*)result.data, result.cols, result.rows, QImage::Format_RGB888);
    //imgResult.save(QString("result.jpg"),"JPEG");

    return laserImage;
}

Mat ScanProc::DetectLaser2(Mat &laserOn, Mat &laserOff)
{
    //some parameter need to be tuned
    int laserMagnitudeThreshold = 20;   //default: 10
    int maxLaserWidth = 40, minLaserWidth = 3; //default: 40, 3
    int rangeDistanceThreshold = 5; //default: 5

    int& rows = laserOff.rows;
    int& cols = laserOff.cols;
    Mat grayLaserOn(rows, cols, CV_8U, Scalar(0));
    Mat grayLaserOff(rows, cols, CV_8U, Scalar(0));
    Mat diffImage(rows, cols, CV_8U, Scalar(0));
    Mat thresholdImage(rows, cols, CV_8U, Scalar(0));
    Mat rangeImage(rows, cols, CV_8U, Scalar(0));
    Mat result(rows, cols, CV_8U, Scalar(0));

    // convert to grayscale
    cvtColor(laserOff, grayLaserOff, CV_BGR2GRAY);
    cvtColor(laserOn, grayLaserOn, CV_BGR2GRAY);
    subtract(grayLaserOn, grayLaserOff, diffImage);

    //const double MAX_MAGNITUDE_SQ = 255 * 255 * 3; // The maximum pixel magnitude sq we can see
    //const double INV_MAX_MAGNITUDE_SQ = 1.0f / MAX_MAGNITUDE_SQ;

    const int width = grayLaserOff.cols;
    const int height = grayLaserOff.rows;
    //unsigned components = before.getNumComponents();
    //unsigned rowStep = width * components;

    int numLocations = 0;

    int numMerged = 0;
    int numPixelsOverThreshold = 0;

    // The location that we last detected a laser line
    int maxNumLocations = height;
    int firstRowLaserCol = width / 2;
    int prevLaserCol = firstRowLaserCol;
    LaserRange* laserRanges = new LaserRange[width + 1];

    //unsigned char * ar = a;
    //unsigned char * br = b;
    //unsigned char * dr = d;
    for (unsigned iRow = 0; iRow < height && numLocations < maxNumLocations; iRow++)
    {
        // The column that the laser started and ended on
        int numLaserRanges = 0;
        laserRanges[numLaserRanges].startCol = -1;
        laserRanges[numLaserRanges].endCol = -1;

        int numRowOut = 0;
        int imageColumn = 0;
        for (unsigned iCol = 0; iCol < width; iCol += 1)
        {
            int mag = diffImage.at<uchar>(iRow, iCol);

            // Compare it against the threshold
            if (mag > laserMagnitudeThreshold)
            {
                thresholdImage.at<uchar>(iRow, iCol) = 255;

                // Flag that this pixel was over the threshold value
                numPixelsOverThreshold++;

                // The start of pixels with laser in them
                if (laserRanges[numLaserRanges].startCol == -1)
                {
                    laserRanges[numLaserRanges].startCol = imageColumn;
                }

            }
            // The end of pixels with laser in them
            else if (laserRanges[numLaserRanges].startCol != -1)
            {
                int laserWidth = imageColumn - laserRanges[numLaserRanges].startCol;
                if (laserWidth <= maxLaserWidth && laserWidth >= minLaserWidth)
                {
                    // If this range was real close to the previous one, merge them instead of creating a new one
                    bool wasMerged = false;
                    if (numLaserRanges > 0)
                    {
                        unsigned rangeDistance = laserRanges[numLaserRanges].startCol - laserRanges[numLaserRanges - 1].endCol;
                        if (rangeDistance < rangeDistanceThreshold)
                        {
                            laserRanges[numLaserRanges - 1].endCol = imageColumn;
                            laserRanges[numLaserRanges - 1].centerCol = round((laserRanges[numLaserRanges - 1].startCol + laserRanges[numLaserRanges - 1].endCol) / 2);
                            wasMerged = true;
                            numMerged++;
                        }
                    }

                    // Proceed to the next laser range
                    if (!wasMerged)
                    {
                        // Add this range as a candidate
                        laserRanges[numLaserRanges].endCol = imageColumn;
                        laserRanges[numLaserRanges].centerCol = round((laserRanges[numLaserRanges].startCol + laserRanges[numLaserRanges].endCol) / 2);

                        numLaserRanges++;
                    }

                    // Reinitialize the range
                    laserRanges[numLaserRanges].startCol = -1;
                    laserRanges[numLaserRanges].endCol = -1;
                }
                // There was a false positive
                else
                {
                    laserRanges[numLaserRanges].startCol = -1;
                }
            }

            // Go from image components back to image pixels
            imageColumn++;

        } // foreach column

        // If we have a valid laser region
        if (numLaserRanges > 0)
        {
            for (int i = 0; i < numLaserRanges; i++)
            {
                for (int j = laserRanges[i].startCol; j < laserRanges[i].endCol; j++)
                {
                    rangeImage.at<uchar>(iRow, j) = 255;
                }
            }

            int rangeChoice = detectBestLaserRange(laserRanges, numLaserRanges, prevLaserCol);
            prevLaserCol = laserRanges[rangeChoice].centerCol;

            int centerCol = detectLaserRangeCenter(iRow, laserRanges[rangeChoice], diffImage);

            result.at<uchar>(iRow, centerCol) = 255;

            //laserLocations[numLocations].x = centerCol;
            //laserLocations[numLocations].y = iRow;

            // If this is the first row that a laser is detected in, set the firstRowLaserCol member
            if (numLocations == 0)
            {
                firstRowLaserCol = laserRanges[rangeChoice].startCol;
            }

            numLocations++;

        }
    } // foreach row

    pPreviewWnd->updateFrame(grayLaserOff, "laser off");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    pPreviewWnd->updateFrame(grayLaserOn, "laser onn");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    pPreviewWnd->updateFrame(diffImage, "diff");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    pPreviewWnd->updateFrame(thresholdImage, "threshold");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    pPreviewWnd->updateFrame(rangeImage, "laser range");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    pPreviewWnd->updateFrame(result, "result");
    msleep(LASER_LINE_DETECTION_PREVIEW_DELAY);

    return result;
}

int ScanProc::detectBestLaserRange(LaserRange * ranges, int numRanges, int prevLaserCol)
{
    int bestRange = 0;
    int distanceOfBest = abs(ranges[0].centerCol - prevLaserCol);

    // TODO: instead of just looking at the last laser position, this should probably be a sliding window
    // Select based off of minimum distance to last laser position
    for (int i = 1; i < numRanges; i++)
    {
        int dist = abs(ranges[i].centerCol - prevLaserCol);
        if (dist < distanceOfBest)
        {
            bestRange = i;
            distanceOfBest = dist;
        }
    }

    return bestRange;
}

int ScanProc::detectLaserRangeCenter(const int row, const LaserRange& range, const Mat& diffImage)
{
    int startCol = range.startCol;
    int centerCol = startCol;
    int endCol = range.endCol;
    int components = 3;

    int totalSum = 0;
    int weightedSum = 0;
    int cCol = 0;
    for (int bCol = startCol; bCol < endCol; bCol++)
    {
        int mag = diffImage.at<uchar>(row, bCol);

        totalSum += mag;
        weightedSum += mag * cCol;

        cCol++;
    }

    // Compute the center of mass
    centerCol = startCol + int((float)weightedSum / totalSum + 0.5f);

    return centerCol;
}

void ScanProc::MapLaserPointToGlobalPoint(Mat &laserLine, Mat &laserOff)
{
    configuration* config = Edit3DScanPlugin::getConfiguration();
    Turntable* turntable = Edit3DScanPlugin::getTurntable();

    //calculate position of laser on the back plane in cv frame
    GlobalPoint gLaserLinePosition = config->getLaserPositionOnBackPlane();
    CvPoint cvLaserLinePosition = convertGlobalPointToCvPoint(gLaserLinePosition);
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
                    static int c = 0;
                    vcg::tri::Allocator<CMeshO>::AddVertices(mesh->cm, 1);
                    mesh->cm.vert[c].P() = vcg::Point3f(fsNewPoint.x, fsNewPoint.y, fsNewPoint.z);
                    mesh->cm.vert[c].C() = vcg::Color4b(255, 0, 0, 255);
                    c++;
                    mesh->meshModified() = true;
                    gla->update();
                    //eliminate points from the grounds, that are not part of the model
                    //qDebug("adding point");
                    //model->addPointToPointCloud(fsNewPoint);
                }
                break;
            }
        }
    }
}

CvPoint ScanProc::convertGlobalPointToCvPoint(GlobalPoint fsPoint)
{
    configuration* config = Edit3DScanPlugin::getConfiguration();

    CvSize cvImageSize = cvSize(config->CAM_IMAGE_WIDTH,
                                config->CAM_IMAGE_HEIGHT);
    GlobalSizeFS fsImageSize = MakeGlobalSize(config->FRAME_WIDTH,
                                            (double)config->FRAME_WIDTH * ((double)config->CAM_IMAGE_HEIGHT / (double)config->CAM_IMAGE_WIDTH),
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
