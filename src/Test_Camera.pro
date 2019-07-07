#-------------------------------------------------
#
# Project created by QtCreator 2016-04-08T11:39:08
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Test_Camera
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH +=C:\\OpenCV-3.1.0\\opencv\\build\\include \

LIBS +=-LC:\\OpenCV-3.1.0\\mybuild\\lib\\Debug \
    -lopencv_videostab310d \
    -lopencv_stitching310d \
    -lopencv_calib3d310d \
    -lopencv_ts310d \
    -lopencv_features2d310d \
    -lopencv_objdetect310d \
    -lopencv_highgui310d \
    -lopencv_superres310d \
    -lopencv_videoio310d \
    -lopencv_shape310d \
    -lopencv_imgcodecs310d \
    -lopencv_photo310d \
    -lopencv_video310d \
    -lopencv_imgproc310d \
    -lopencv_flann310d \
    -lopencv_ml310d \
    -lopencv_core310d

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../Downloads/build-SMTPEmail-Desktop_Qt_5_6_0_MSVC2015_32bit-Debug/release/ -lSMTPEmail
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../Downloads/build-SMTPEmail-Desktop_Qt_5_6_0_MSVC2015_32bit-Debug/debug/ -lSMTPEmail
else:unix: LIBS += -L$$PWD/../../../Downloads/build-SMTPEmail-Desktop_Qt_5_6_0_MSVC2015_32bit-Debug/ -lSMTPEmail

INCLUDEPATH += $$PWD/../../../Downloads/build-SMTPEmail-Desktop_Qt_5_6_0_MSVC2015_32bit-Debug/debug
DEPENDPATH += $$PWD/../../../Downloads/build-SMTPEmail-Desktop_Qt_5_6_0_MSVC2015_32bit-Debug/debug
