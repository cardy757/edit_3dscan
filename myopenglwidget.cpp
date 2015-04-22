#include "myopenglwidget.h"
#include <QtOpenGL/qgl.h>

MyOpenGLWidget::MyOpenGLWidget(QObject *parent)
    : QGLWidget((QWidget *) parent)
{

}


MyOpenGLWidget::~MyOpenGLWidget()
{

}

void MyOpenGLWidget::paintGL()
{	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 1);

    glBegin(GL_QUADS);
    {
        glTexCoord2f(0, 1); glVertex3f(-1, -1, 0);
        glTexCoord2f(1, 1); glVertex3f(1, -1, 0);
        glTexCoord2f(1, 0); glVertex3f(1, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-1, 1, 0);
    }
    glEnd();
}

void MyOpenGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void MyOpenGLWidget::updateFrame(QImage& image)
{
    deleteTexture(1);
    bindTexture(image, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption);
    updateGL();
}