#include "group.h"
#include "kernel/kernel.h"

Group::Group(Kernel* core) : Item(core) {}

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

Group* Group::copy() {
    Group* group = new Group(kernel);
    group->id = id;
    group->label = label;
    group->fixtures = fixtures;
    return group;
}
