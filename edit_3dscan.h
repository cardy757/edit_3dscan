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

#ifndef EDIT3DSCANPLUGIN_H
#define EDIT3DSCANPLUGIN_H

#include <QObject>
#include <common/interfaces.h>
#include <opencv2/highgui/highgui.hpp>
#include "ScanProc.h"
#include "scanDialog.h"
#include "webcamdlg.h"
#include "camerapreviewdlg.h"
#include "webcam.h"
#include "configuration.h"

#define RENDER_USING_OPENGL 0

using namespace cv;

class Edit3DScanPlugin : public QObject, public MeshEditInterface
{
    Q_OBJECT
    Q_INTERFACES(MeshEditInterface)
        
public:
    Edit3DScanPlugin();
    virtual ~Edit3DScanPlugin();

    static const QString Info();

    virtual bool isSingleMeshEdit() const { return false; }
    virtual void LayerChanged(MeshDocument &md, MeshModel &oldMeshModel, GLArea *parent) {}

    virtual bool StartEdit(MeshDocument &/*m*/, GLArea * /*parent*/);
    virtual void EndEdit(MeshModel &/*m*/, GLArea * /*parent*/);
    virtual void Decorate (MeshModel &/*m*/, GLArea * ){};
    virtual void mousePressEvent(QMouseEvent *, MeshModel &, GLArea * );
    virtual void mouseMoveEvent(QMouseEvent *, MeshModel &, GLArea * );
    virtual void mouseReleaseEvent(QMouseEvent *event, MeshModel &/*m*/, GLArea * );
    virtual void wheelEvent(QWheelEvent*, MeshModel &/*m*/, GLArea *);

    static configuration* config;
        
private:
    ScanDialog *scanDialog;
#if RENDER_USING_OPENGL
    WebCamDlg *webCamDlg;
#else
    CameraPreviewDlg *cameraPreviewDlg;
#endif
    GLArea *gla;
    MeshDocument *md;
    MeshModel *mesh;
    ScanProc scanProc;
    webcam m_webcam;

    void releaseResource();

private Q_SLOTS:
    void procScan(); //click Start/Stop Scan button
    void webCam(int checkState); //change Web Camera Preivew checkbox
    void camWndClosed();
    void updateFrame();
};

#endif
