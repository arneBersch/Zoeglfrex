#include "item.h"

Item::Item(Kernel* core) {
    kernel = core;
}

Item::~Item() {}

QString Item::name() {
    return id + " " + label;
}

QString Item::info() {
    QString info = "0 ID: " + id;
    info += "\n1 Label: \"" + label + "\"";
    return info;
}
