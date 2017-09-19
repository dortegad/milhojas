#-------------------------------------------------
#
# Project created by QtCreator 2015-05-24T08:25:34
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = milHojas
TEMPLATE = app


SOURCES += main.cpp\
        mwmilhojas.cpp

HEADERS  += mwmilhojas.h

FORMS    += mwmilhojas.ui

#QT_CONFIG -= no-pkg-config
#CONFIG += link_pkgconfig
#PKGCONFIG += opencv


INCLUDEPATH += D:\opencv-3.2.0\build\install\include
LIBS += -LD:\opencv-3.2.0\build\install\x86\vc12\lib \
-lopencv_aruco320d \
-lopencv_bgsegm320d \
-lopencv_bioinspired320d \
-lopencv_calib3d320d \
-lopencv_ccalib320d \
-lopencv_core320d \
-lopencv_datasets320d \
-lopencv_dnn320d \
-lopencv_dpm320d \
-lopencv_face320d \
-lopencv_features2d320d \
-lopencv_flann320d \
-lopencv_fuzzy320d \
-lopencv_hdf320d \
-lopencv_highgui320d \
-lopencv_imgcodecs320d \
-lopencv_imgproc320d \
-lopencv_line_descriptor320d \
-lopencv_ml320d \
-lopencv_objdetect320d \
-lopencv_optflow320d \
-lopencv_phase_unwrapping320d \
-lopencv_photo320d \
-lopencv_plot320d \
-lopencv_reg320d \
-lopencv_rgbd320d \
-lopencv_saliency320d \
-lopencv_shape320d \
-lopencv_stereo320d \
-lopencv_stitching320d \
-lopencv_structured_light320d \
-lopencv_superres320d \
-lopencv_surface_matching320d \
-lopencv_text320d \
-lopencv_tracking320d \
-lopencv_video320d \
-lopencv_videoio320d \
-lopencv_videostab320d \
-lopencv_xfeatures2d320d \
-lopencv_ximgproc320d \
-lopencv_xobjdetect320d \
-lopencv_xphoto320d





