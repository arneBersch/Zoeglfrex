QT += core
QT += gui
QT += sql
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutwindow/aboutwindow.cpp \
    cuelistview/cuelistview.cpp \
    inspector/inspector.cpp \
    main.cpp \
    mainwindow/mainwindow.cpp \
    sacnserver/sacnserver.cpp \
    terminal/terminal.cpp

HEADERS += \
    aboutwindow/aboutwindow.h \
    constants.h \
    cuelistview/cuelistview.h \
    inspector/inspector.h \
    mainwindow/mainwindow.h \
    sacnserver/sacnserver.h \
    terminal/terminal.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    zoeglfrex.qrc
