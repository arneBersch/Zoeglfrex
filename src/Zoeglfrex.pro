QT += core
QT += gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutwindow/aboutwindow.cpp \
    cuelistview/cuelistview.cpp \
    cuelistview/cuemodel.cpp \
    cuelistview/dmxengine.cpp \
    cuelistview/sacnserver.cpp \
    inspector/inspector.cpp \
    kernel/items/color.cpp \
    kernel/items/cue.cpp \
    kernel/items/fixture.cpp \
    kernel/items/group.cpp \
    kernel/items/intensity.cpp \
    kernel/items/model.cpp \
    kernel/items/transition.cpp \
    kernel/kernel.cpp \
    kernel/lists/colorlist.cpp \
    kernel/lists/cuelist.cpp \
    kernel/lists/fixturelist.cpp \
    kernel/lists/grouplist.cpp \
    kernel/lists/intensitylist.cpp \
    kernel/lists/itemlist.cpp \
    kernel/lists/modellist.cpp \
    kernel/lists/transitionlist.cpp \
    main.cpp \
    mainwindow/mainwindow.cpp \
    terminal/terminal.cpp

HEADERS += \
    aboutwindow/aboutwindow.h \
    cuelistview/cuelistview.h \
    cuelistview/cuemodel.h \
    cuelistview/dmxengine.h \
    cuelistview/sacnserver.h \
    inspector/inspector.h \
    kernel/items/color.h \
    kernel/items/cue.h \
    kernel/items/fixture.h \
    kernel/items/group.h \
    kernel/items/intensity.h \
    kernel/items/model.h \
    kernel/items/transition.h \
    kernel/kernel.h \
    kernel/lists/colorlist.h \
    kernel/lists/cuelist.h \
    kernel/lists/fixturelist.h \
    kernel/lists/grouplist.h \
    kernel/lists/intensitylist.h \
    kernel/lists/itemlist.h \
    kernel/lists/modellist.h \
    kernel/lists/transitionlist.h \
    mainwindow/mainwindow.h \
    terminal/terminal.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    resources/style.qss \
    resources/licenses.txt

RESOURCES += \
    zoeglfrex.qrc
