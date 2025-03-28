QT += core
QT += gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cuelistview/cuelistview.cpp \
    cuelistview/cuemodel.cpp \
    cuelistview/dmxengine.cpp \
    cuelistview/groupmodel.cpp \
    cuelistview/sacnserver.cpp \
    inspector/inspector.cpp \
    kernel/items/color.cpp \
    kernel/items/cue.cpp \
    kernel/items/effect.cpp \
    kernel/items/fixture.cpp \
    kernel/items/group.cpp \
    kernel/items/intensity.cpp \
    kernel/items/item.cpp \
    kernel/items/model.cpp \
    kernel/items/raw.cpp \
    kernel/kernel.cpp \
    kernel/lists/colorlist.cpp \
    kernel/lists/cuelist.cpp \
    kernel/lists/effectlist.cpp \
    kernel/lists/fixturelist.cpp \
    kernel/lists/grouplist.cpp \
    kernel/lists/intensitylist.cpp \
    kernel/lists/itemlist.cpp \
    kernel/lists/modellist.cpp \
    kernel/lists/rawlist.cpp \
    main.cpp \
    mainwindow/aboutwindow.cpp \
    mainwindow/mainwindow.cpp \
    terminal/terminal.cpp

HEADERS += \
    cuelistview/cuelistview.h \
    cuelistview/cuemodel.h \
    cuelistview/dmxengine.h \
    cuelistview/groupmodel.h \
    cuelistview/sacnserver.h \
    inspector/inspector.h \
    kernel/items/color.h \
    kernel/items/cue.h \
    kernel/items/effect.h \
    kernel/items/fixture.h \
    kernel/items/group.h \
    kernel/items/intensity.h \
    kernel/items/item.h \
    kernel/items/model.h \
    kernel/items/raw.h \
    kernel/kernel.h \
    kernel/lists/colorlist.h \
    kernel/lists/cuelist.h \
    kernel/lists/effectlist.h \
    kernel/lists/fixturelist.h \
    kernel/lists/grouplist.h \
    kernel/lists/intensitylist.h \
    kernel/lists/itemlist.h \
    kernel/lists/modellist.h \
    kernel/lists/rawlist.h \
    mainwindow/aboutwindow.h \
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
