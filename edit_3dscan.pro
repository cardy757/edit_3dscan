include (../../shared.pri)

QT += multimedia multimediawidgets

HEADERS       = edit_3dscan_factory.h \
                scanDialog.h \
                ScanProc.h \
                myopenglwidget.h \
                webcamdlg.h \
                edit_3dscan.h \
                camerapreviewdlg.h

SOURCES       = edit_3dscan_factory.cpp \
                edit_3dscan.cpp \
                scanDialog.cpp \
                ScanProc.cpp \
                myopenglwidget.cpp \
                webcamdlg.cpp \
                camerapreviewdlg.cpp
				

TARGET        = edit_3dscan

FORMS	      = webcamdlg.ui \
                scandialog.ui


macx {
    message("Buildng for Mac.")

    INCLUDEPATH   += /usr/local/include/

    LIBS += -L/usr/local/lib/ \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_videoio
}
