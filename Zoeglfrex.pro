QT += core
QT += gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutwindow.cpp \
    cuelistview/cuelistview.cpp \
    cuelistview/cuemodel.cpp \
    cuelistview/dmxengine.cpp \
    cuelistview/sacnserver.cpp \
    inspector/inspector.cpp \
    kernel/kernel.cpp \
    kernel/lists/colorlist.cpp \
    kernel/lists/cuelist.cpp \
    kernel/lists/fixturelist.cpp \
    kernel/lists/grouplist.cpp \
    kernel/lists/intensitylist.cpp \
    kernel/lists/modellist.cpp \
    kernel/lists/rowlist.cpp \
    kernel/lists/transitionlist.cpp \
    main.cpp \
    mainwindow.cpp \
    terminal/terminal.cpp

HEADERS += \
    aboutwindow.h \
    cuelistview/cuelistview.h \
    cuelistview/cuemodel.h \
    cuelistview/dmxengine.h \
    cuelistview/sacnserver.h \
    inspector/inspector.h \
    kernel/kernel.h \
    kernel/lists/colorlist.h \
    kernel/lists/cuelist.h \
    kernel/lists/fixturelist.h \
    kernel/lists/grouplist.h \
    kernel/lists/intensitylist.h \
    kernel/lists/modellist.h \
    kernel/lists/rowlist.h \
    kernel/lists/transitionlist.h \
    mainwindow.h \
    terminal/terminal.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    resources/style.qss \
    LICENSE

RESOURCES += \
    zoeglfrex.qrc
