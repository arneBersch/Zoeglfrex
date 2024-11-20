#include "group.h"
#include "kernel/kernel.h"

Group::Group(Kernel* core) : Item(core) {}

Group::Group(const Group* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    fixtures = item->fixtures;
}

Group::~Group() {
    kernel->cues->deleteGroup(this);
}

QString Group::name() {
    if (label.isEmpty()) {
        QString response = Item::name();
        for (Fixture* fixture : fixtures) {
            response += fixture->name() + "; ";
        }
        return response;
    }
    return Item::name();
}

QString Group::info() {
    QString info = Item::info();
    QString fixtureNames;
    for (Fixture* fixture : fixtures) {
        fixtureNames += fixture->name() + ", ";
    }
    fixtureNames.chop(2);
    info += "\nFixtures: " + fixtureNames;
    return info;
}

