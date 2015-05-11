#include "configuration.h"

configuration::configuration()
{
    this->setDefaultValues();
}

void configuration::setDefaultValues()
{
    //to make the scanning process faster we ommit the lower and hight part of the cvImage
    //as there is no object anyway.  The lower limit is defined by the turning table lower bound
    //units are pixels, seen from the top, resp from the bottom
    UPPER_ANALYZING_FRAME_LIMIT = 0;
    LOWER_ANALYZING_FRAME_LIMIT = 30;

    //as the actual position in the frame differs a little from calculated laserline we stop a little befor as we might catch the real non reflected laser line which creates noise
    ANALYZING_LASER_OFFSET = 90;

    //defining the origin in the cvFrame
    //the position of intersection of back plane with ground plane in cvFrame in percent
    //check the yellow laser line to calibrate, the yellow laser line should touch the bottom plane
    //#define ORIGIN_Y 0.825
    ORIGIN_Y= 0.75;

    /********************************/
    /*       CAMERA DEFINES         */
    /********************************/

    //logitech c270
    FRAME_WIDTH = 30.0f; //in cm. the width of what the camera sees, ie place a measure tool at the back-plane and see how many cm the camera sees.
    CAM_IMAGE_WIDTH = 1280.0f;
    CAM_IMAGE_HEIGHT = 960.0f; //here I am not sure, i think it is 960, (old=853.0f)

    /********************************/
    /*    HARDWARE SETUP DEFINES    */
    /********************************/

    //position of the laser
    LASER_POS_X = 14.0f; //precise by construction
    LASER_POS_Y = 10.0f;  //not needed/used for calculations
    LASER_POS_Z = 28.8f; //precise by construction

    LASER_POS_ON_BACK_PLANE_X = -7.0f;
    LASER_POS_ON_BACK_PLANE_Y = 6.4f;
    LASER_POS_ON_BACK_PLANE_Z = 0.0f;

    LASER_SWIPE_MIN = 18.0f;
    LASER_SWIPE_MAX = 52.0f;

    //position of the c270
    CAM_POS_X = 0.0f; //precise by construction
    CAM_POS_Y = 11.0f;
    CAM_POS_Z = 26.0f;

    //position of the turntable
    TURNTABLE_POS_X = 0.0f; //not used by calculations
    TURNTABLE_POS_Y = 0.0f; //not used by calculations
    TURNTABLE_POS_Z = 11.0f; //precise by construction
}

void configuration::read(const FileNode& node)                          //Read serialization for this class
{
    node["UPPER_ANALYZING_FRAME_LIMIT"] >> UPPER_ANALYZING_FRAME_LIMIT;
    node["LOWER_ANALYZING_FRAME_LIMIT"] >> LOWER_ANALYZING_FRAME_LIMIT;
    node["ANALYZING_LASER_OFFSET"] >> ANALYZING_LASER_OFFSET;
    node["ORIGIN_Y"] >> ORIGIN_Y;
    node["FRAME_WIDTH"] >> FRAME_WIDTH;
    node["CAM_IMAGE_WIDTH"] >> CAM_IMAGE_WIDTH;
    node["CAM_IMAGE_HEIGHT"] >> CAM_IMAGE_HEIGHT;
    node["LASER_POS_X"] >> LASER_POS_X;
    node["LASER_POS_Y"] >> LASER_POS_Y;
    node["LASER_POS_Z"] >> LASER_POS_Z;
    node["LASER_SWIPE_MIN"] >> LASER_SWIPE_MIN;
    node["LASER_SWIPE_MAX"] >> LASER_SWIPE_MAX;
    node["CAM_POS_Y"] >> CAM_POS_Y;
    node["CAM_POS_Z"] >> CAM_POS_Z;
    node["TURNTABLE_POS_X"] >> TURNTABLE_POS_X;
    node["TURNTABLE_POS_Y"] >> TURNTABLE_POS_Y;
    node["TURNTABLE_POS_Z"] >> TURNTABLE_POS_Z;

    interprate();
}

bool configuration::readConfiguration()
{
    string resPath;
    // todo: where to load configuration file?
    resPath = "/Users/justin/MeshLabSrc/configuration.xml";

    FileStorage fs(resPath, FileStorage::READ);
    if(!fs.isOpened())
    {
        return false;
    }

    fs["Configuration"] >> *this;
    fs.release();

    if(!this->goodInput)
    {
        return false;
    }

    return true;
}

void configuration::interprate()
{
    this->goodInput = true;
}

GlobalPoint configuration::getCameraPosition()
{
    return MakeGlobalPoint(CAM_POS_X,
                           CAM_POS_Y,
                           CAM_POS_Z);
}

GlobalPoint configuration::getLaserPosition()
{
    return MakeGlobalPoint(LASER_POS_X,
                           LASER_POS_Y,
                           LASER_POS_Z);
}

GlobalPoint configuration::getLaserPositionOnBackPlane()
{
    return MakeGlobalPoint(LASER_POS_ON_BACK_PLANE_X,
                           LASER_POS_ON_BACK_PLANE_Y,
                           LASER_POS_ON_BACK_PLANE_Z);
}

