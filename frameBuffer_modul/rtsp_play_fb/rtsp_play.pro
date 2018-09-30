#-------------------------------------------------
#
# Project created by QtCreator 2017-12-20T16:45:11
#
#-------------------------------------------------

QT       -= core gui

TARGET = rtsp_play
TEMPLATE = lib

DEFINES += RTSP_PLAY_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        rtsp_play.cpp \
    RtspPlayer.cpp \
    imageFunction.cpp \
    MyImgList.cpp \
    log4z.cpp \
    fbshow.cpp

HEADERS += \
        rtsp_play.h \
        rtsp_play_global.h \ 
    RtspPlayer.h \
    imageFunction.h \
    MyImgList.h \
    log4z.h \
    fbshow.h \
    mybmphead.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!macx|win32: LIBS += -L$$PWD/ffmpeg-3.4/lib/ -lavcodec

INCLUDEPATH += $$PWD/ffmpeg-3.4/include
DEPENDPATH += $$PWD/ffmpeg-3.4/include

#unix:!macx|win32: LIBS += -L$$PWD/ffmpeg-3.4/lib/ -lavdevice

#INCLUDEPATH += $$PWD/ffmpeg-3.4/include
#DEPENDPATH += $$PWD/ffmpeg-3.4/include

unix:!macx|win32: LIBS += -L$$PWD/ffmpeg-3.4/lib/ -lavformat

INCLUDEPATH += $$PWD/ffmpeg-3.4/include
DEPENDPATH += $$PWD/ffmpeg-3.4/include

unix:!macx|win32: LIBS += -L$$PWD/ffmpeg-3.4/lib/ -lavutil

INCLUDEPATH += $$PWD/ffmpeg-3.4/include
DEPENDPATH += $$PWD/ffmpeg-3.4/include

unix:!macx|win32: LIBS += -L$$PWD/ffmpeg-3.4/lib/ -lswresample

INCLUDEPATH += $$PWD/ffmpeg-3.4/include
DEPENDPATH += $$PWD/ffmpeg-3.4/include

unix:!macx|win32: LIBS += -L$$PWD/ffmpeg-3.4/lib/ -lswscale

INCLUDEPATH += $$PWD/ffmpeg-3.4/include
DEPENDPATH += $$PWD/ffmpeg-3.4/include

unix:!macx|win32: LIBS += -L$$PWD/SDL2-2.0.7/lib/x86/ -lSDL2

INCLUDEPATH += $$PWD/SDL2-2.0.7/include
DEPENDPATH += $$PWD/SDL2-2.0.7/include
