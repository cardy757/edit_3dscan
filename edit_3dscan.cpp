/****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2005                                                \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *   
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/
/****************************************************************************
  History
$Log: meshedit.cpp,v $
****************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <meshlab/glarea.h>
#include "edit_3dscan.h"
#include <wrap/gl/pick.h>
#include <wrap/qt/gl_label.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/videoio/videoio_c.h" // need for CV_CAP_PROP... ??

using namespace std;
using namespace vcg;
using namespace cv;

Edit3DScanPlugin::Edit3DScanPlugin() : scanProc(this)
{
	scanDialog = NULL;
#if RENDER_USING_OPENGL
    webCamDlg = NULL;
#else
    cameraPreviewDlg = NULL;
#endif
    gla = NULL;
	md = NULL;
	mesh = NULL;
	cvcap = NULL;
	timer = NULL;
}

Edit3DScanPlugin::~Edit3DScanPlugin() //TODO: close meshlab without stop edit will not call destructor?
{
	releaseResource();
}

void Edit3DScanPlugin::releaseResource()
{
	if (timer != NULL)
	{
		timer->stop();
		delete timer;
		timer = NULL;
	}
	if (cvcap != NULL)
	{
		cvcap->release();
		delete cvcap;
		cvcap = NULL;
	}
#if RENDER_USING_OPENGL
    if (webCamDlg != NULL)
	{
		delete webCamDlg;
		webCamDlg = NULL;
    }
#else
    if (cameraPreviewDlg != NULL)
    {
        delete cameraPreviewDlg;
        cameraPreviewDlg = NULL;
    }
#endif
	if (scanDialog != NULL)
	{
		delete scanDialog;
		scanDialog = NULL;
	}
}

const QString Edit3DScanPlugin::Info() 
{
	return tr("Execute 3D scanning process.");
}

void Edit3DScanPlugin::mousePressEvent(QMouseEvent *event, MeshModel &m, GLArea *gla)
{
	if (Qt::LeftButton | event->buttons())
	{
		gla->suspendedEditor = true;
		QCoreApplication::sendEvent(gla, event);
		gla->suspendedEditor = false;
	}
}

void Edit3DScanPlugin::mouseMoveEvent(QMouseEvent *event, MeshModel &m, GLArea *gla)
{
	if (Qt::LeftButton | event->buttons())
	{
		gla->suspendedEditor = true;
		QCoreApplication::sendEvent(gla, event);
		gla->suspendedEditor = false;
	}
}

void Edit3DScanPlugin::mouseReleaseEvent(QMouseEvent *event, MeshModel &m, GLArea *gla)
{
	if (Qt::LeftButton | event->buttons())
	{
		gla->suspendedEditor = true;
		QCoreApplication::sendEvent(gla, event);
		gla->suspendedEditor = false;
	}
}

void Edit3DScanPlugin::wheelEvent(QWheelEvent *event, MeshModel &m, GLArea *gla)
{
	gla->suspendedEditor = true;
	QCoreApplication::sendEvent(gla, event);
	gla->suspendedEditor = false;
}

bool Edit3DScanPlugin::StartEdit(MeshDocument &m, GLArea *parent)
{
	this->md = &m;
	this->gla = parent;
	if (md->mm() == NULL)
	{
		RenderMode rm;
		rm.drawMode = GLW::DMPoints;
		md->addNewMesh("", "Scanned Mesh", true, rm);
	}

	if (this->mesh != this->md->mm())
		this->mesh = this->md->mm();

	//Create GUI window if we dont already have one
	if (scanDialog == NULL)
	{
		scanDialog = new ScanDialog(gla->window());
		connect(scanDialog->ui.procScan, SIGNAL(clicked()), this, SLOT(procScan()));
		connect(scanDialog->ui.webCam, SIGNAL(stateChanged(int)), this, SLOT(webCam(int)));
		connect(scanDialog, SIGNAL(SGN_Closing()), gla, SLOT(endEdit()));
	}
	scanDialog->show();

	//Initialize opencv video capture device
	cvcap = new VideoCapture(0); // open the video camera no. 0
	if (!cvcap->isOpened()) false;

    cvcap->set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    cvcap->set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	//timer to get new frame from camera
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateFrame()));

	return true;
}

void Edit3DScanPlugin::EndEdit(MeshModel &m, GLArea *parent)
{
	releaseResource();
}

void Edit3DScanPlugin::procScan()
{
	QPushButton *b = qobject_cast<QPushButton *>(sender());

	if (!scanProc.isRunning()) //start scan process
	{
		scanProc.SetMesh(mesh);
		scanProc.SetGLArea(gla);
		scanProc.start();
		b->setText("Stop Scan");
	}
	else //stop scan process
	{
		scanProc.stop();
		b->setText("Start Scan");
	}
}

void Edit3DScanPlugin::webCam(int checkState)
{
	if (checkState == Qt::Checked)
	{
#if RENDER_USING_OPENGL
        if (webCamDlg == NULL)
		{
			webCamDlg = new WebCamDlg;
            connect(webCamDlg, SIGNAL(SGN_Closing()), this, SLOT(camWndClosed()));
		}
        webCamDlg->show();
#else
        if (cameraPreviewDlg == NULL)
        {
            cameraPreviewDlg = new CameraPreviewDlg;
            connect(cameraPreviewDlg, SIGNAL(SGN_Closing()), this, SLOT(camWndClosed()));

        }
        cameraPreviewDlg->show();
#endif

        timer->start(30);
	}
	else
	{
		timer->stop();
#if RENDER_USING_OPENGL
        webCamDlg->hide();
#else
        cameraPreviewDlg->hide();
#endif
    }
}

void Edit3DScanPlugin::camWndClosed()
{
	scanDialog->ui.webCam->setCheckState(Qt::Unchecked);
}

void Edit3DScanPlugin::updateFrame()
{
    Mat frame;
	bool bSuccess = cvcap->read(frame); // read a new frame from video

	QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
	image = image.rgbSwapped();

    static bool bTest = true;
    if (bTest)
    {
        QString str = "test.jpg";
        bool bResult = image.save(str,"JPEG");
        bTest = false;
    }

#if RENDER_USING_OPENGL
    webCamDlg->updateFrame(image);
#else
    cameraPreviewDlg->updateFrame(image);
#endif
	//send the captured frame to ScanProc for futher processing
}
