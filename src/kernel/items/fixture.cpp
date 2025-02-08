#include "fixture.h"
#include "kernel/kernel.h"

Fixture::Fixture(Kernel* core) : Item(core) {}

Fixture::Fixture(const Fixture* item) : Item(item->kernel) {
    id = item->id;
    label = item->label;
    model = item->model;
    address = 0;
}

Fixture::~Fixture() {
    kernel->groups->deleteFixture(this);
}

QString Fixture::name() {
    QString channels = "Channel";
    if (model != nullptr) {
        channels = model->channels;
    }
    if (label.isEmpty()) {
        return Item::name() + channels + " (" + QString::number(address) + ")";
    }
    return Item::name();
}

QString Fixture::info() {
    QString info = Item::info();
    info += "\n2 Model: ";
    if (model == nullptr) {
        info += "None (Dimmer)";
    } else {
        info += model->name();
    }
    info += "\n3 Address: " + QString::number(address);
    return info;
}
