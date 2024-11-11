#include "item.h"

Item::Item(Kernel* core) {
    kernel = core;
}

Item::~Item() {}

QString Item::name() {
    return idString() + " " + label;
}

QString Item::idString() {
    QString idString = id;
    while (idString.endsWith(".0")) {
        idString.chop(2);
    }
    return idString;
}
