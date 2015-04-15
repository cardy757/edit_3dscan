#ifndef SCANPROC_H
#define SCANPROC_H

#include <QThread>
#include <common/interfaces.h>
#include <meshlab/glarea.h>

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

private:
    QMutex mutex;
    bool fstop;

    MeshModel *mesh;
    GLArea *gla;
};

#endif // SCANPROC_H
