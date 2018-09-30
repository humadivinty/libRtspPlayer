TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    libtest.h

unix:!macx|win32: LIBS += -L$$PWD/lib/ -llibTest

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

unix:!macx|win32: LIBS += -L$$PWD/lib/ -lrtsp_play

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
