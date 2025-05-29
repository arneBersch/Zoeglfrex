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
    controlpanel/controlpanel.cpp \
    cuelistview/cuelistview.cpp \
    cuelistview/cuemodel.cpp \
    cuelistview/groupmodel.cpp \
    dmxengine/dmxengine.cpp \
    dmxengine/sacnserver.cpp \
    inspector/inspector.cpp \
    kernel/items/color.cpp \
    kernel/items/cue.cpp \
    kernel/items/cuelist.cpp \
    kernel/items/effect.cpp \
    kernel/items/fixture.cpp \
    kernel/items/group.cpp \
    kernel/items/intensity.cpp \
    kernel/items/item.cpp \
    kernel/items/model.cpp \
    kernel/items/position.cpp \
    kernel/items/raw.cpp \
    kernel/kernel.cpp \
    kernel/tables/colortable.cpp \
    kernel/tables/cuelisttable.cpp \
    kernel/tables/cuetable.cpp \
    kernel/tables/effecttable.cpp \
    kernel/tables/fixturetable.cpp \
    kernel/tables/grouptable.cpp \
    kernel/tables/intensitytable.cpp \
    kernel/tables/itemtable.cpp \
    kernel/tables/modeltable.cpp \
    kernel/tables/positiontable.cpp \
    kernel/tables/rawtable.cpp \
    main.cpp \
    mainwindow/mainwindow.cpp \
    preview2d/fixturegraphicsitem.cpp \
    preview2d/preview2d.cpp \
    terminal/terminal.cpp

HEADERS += \
    aboutwindow/aboutwindow.h \
    controlpanel/controlpanel.h \
    cuelistview/cuelistview.h \
    cuelistview/cuemodel.h \
    cuelistview/groupmodel.h \
    dmxengine/dmxengine.h \
    dmxengine/sacnserver.h \
    inspector/inspector.h \
    kernel/items/color.h \
    kernel/items/cue.h \
    kernel/items/cuelist.h \
    kernel/items/effect.h \
    kernel/items/fixture.h \
    kernel/items/group.h \
    kernel/items/intensity.h \
    kernel/items/item.h \
    kernel/items/model.h \
    kernel/items/position.h \
    kernel/items/raw.h \
    kernel/kernel.h \
    kernel/tables/colortable.h \
    kernel/tables/cuelisttable.h \
    kernel/tables/cuetable.h \
    kernel/tables/effecttable.h \
    kernel/tables/fixturetable.h \
    kernel/tables/grouptable.h \
    kernel/tables/intensitytable.h \
    kernel/tables/itemtable.h \
    kernel/tables/modeltable.h \
    kernel/tables/positiontable.h \
    kernel/tables/rawtable.h \
    mainwindow/mainwindow.h \
    preview2d/fixturegraphicsitem.h \
    preview2d/preview2d.h \
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
