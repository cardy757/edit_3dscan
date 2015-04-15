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
