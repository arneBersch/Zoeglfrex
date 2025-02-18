#include "item.h"
#include "kernel/kernel.h"

Item::Item(Kernel* core) {
    kernel = core;
}

Item::~Item() {}

QString Item::name() {
    return id + " " + label;
}

QString Item::info() {
    QString info = kernel->models->IDATTRIBUTEID + " ID: " + id;
    info += "\n" + kernel->models->LABELATTRIBUTEID + " Label: \"" + label + "\"";
    return info;
}
