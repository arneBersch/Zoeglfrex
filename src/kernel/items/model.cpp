#include "model.h"
#include "kernel/kernel.h"

Model::Model(Kernel* core) : Item(core) {}

Model::Model(const Model* item) : Item(item->kernel) {
    label = item->label;
    channels = item->channels;
}

Model::~Model() {
    for (Fixture* fixture : kernel->fixtures->items) {
        if (fixture->model == this) {
            fixture->model = nullptr;
        }
    }
}

QString Model::name() {
    if (label.isEmpty()) {
        return Item::name() + channels;
    }
    return Item::name();
}

QString Model::info() {
    QString info = Item::info();
    info += "\n" + kernel->models->CHANNELSATTRIBUTEID + " Channels: " + channels;
    return info;
}
