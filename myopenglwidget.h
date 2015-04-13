#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QGLWidget>

class MyOpenGLWidget : public QGLWidget
{
	Q_OBJECT

public:
	MyOpenGLWidget(QObject *parent);
	~MyOpenGLWidget();

	void updateFrame(QImage& image);

protected:
	virtual void paintGL();
	virtual void resizeGL(int width, int height);

private:
	
};

#endif // MYOPENGLWIDGET_H
