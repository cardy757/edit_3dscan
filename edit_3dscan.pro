include (../../shared.pri)


HEADERS       = edit_3dscan_factory.h \
				edit_3dscan.h \
				scanDialog.h \
				ScanProc.h \
				camera.h \
				imagesettings.h \
				videosettings.h
				 
SOURCES       = edit_3dscan_factory.cpp \
				edit_3dscan.cpp \
				scanDialog.cpp \
				ScanProc.cpp \
				camera.cpp \
				imagesettings.cpp \
				videosettings.cpp
				

TARGET        = edit_3dscan

RESOURCES     = edit_3dscan.qrc

FORMS		  = scanDialog.ui \
				camera.ui \
				imagesettings.ui \
				videosettings.ui
