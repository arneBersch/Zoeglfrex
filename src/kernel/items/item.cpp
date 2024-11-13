#include "item.h"

Item::Item(Kernel* core) {
    kernel = core;
}

Item::~Item() {}

QString Item::name() {
    return id + " " + label;
}

QString Item::info() {
    QString info = "ID: " + id + "\n";
    info += "Label: \"" + label + "\"";
    return info;
}
