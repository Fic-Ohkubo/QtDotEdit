#-------------------------------------------------
#
# Project created by QtCreator 2018-10-22T11:55:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DotViewEdit
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mydialog.cpp \
    0_initUI1.cpp \
    0_initUI2.cpp \
    1_mydialog.cpp \
    2_mydialog.cpp \
    2_table.cpp \
    3_DotEdit.cpp \
    B0_uiAction.cpp \
    B1_uiAction.cpp \
    MouseEvent.cpp \
    tool1.cpp \
    tool2.cpp \
    useless.cpp \
    mydialog.cpp \
    CorrectXY.cpp

HEADERS += \
        mydialog.h \
    basedata.h \
    ErrorDef.h \
    MyHead.h \
    Tool.h

FORMS += \
        mydialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -LD:/Libtiff/lib/ -ltiff
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/Libtiff/lib/ -ltiffd

INCLUDEPATH += D:/Libtiff/include
DEPENDPATH += D:/Libtiff/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../OpenCV/ocv2410/build/x64/vc15/lib/ -lopencv_core2410 -lopencv_imgproc2410 -lopencv_highgui2410
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../OpenCV/ocv2410/build/x64/vc15/lib/ -lopencv_core2410d -lopencv_imgproc2410d -lopencv_highgui2410d
else:unix: LIBS += -L$$PWD/../../../../../OpenCV/ocv2410/build/x64/vc15/lib/ -lopencv_core2410

INCLUDEPATH += $$PWD/../../../../../OpenCV/ocv2410/build/include
DEPENDPATH += $$PWD/../../../../../OpenCV/ocv2410/build/include
