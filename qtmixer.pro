QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    colourpanel.cpp \
    main.cpp \
    mainwindow.cpp \
    outputwindow.cpp \
    runpanel.cpp \
    runthread.cpp

HEADERS += \
    colourpanel.h \
    graph.h \
    mainwindow.h \
    outputwindow.h \
    runpanel.h \
    runthread.h

FORMS += \
    colourpanel.ui \
    mainwindow.ui \
    outputwindow.ui \
    runpanel.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
