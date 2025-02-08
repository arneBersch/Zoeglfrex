#include "model.h"
#include "kernel/kernel.h"

Model::Model(Kernel* core) : Item(core) {}

Model::Model(const Model* item) : Item(item->kernel) {
    label = item->label;
    channels = item->channels;
}

Model::~Model() {
    kernel->fixtures->deleteModel(this);
}

QString Model::name() {
    if (label.isEmpty()) {
        return Item::name() + channels;
    }
    return Item::name();
}

QString Model::info() {
    QString info = Item::info();
    info += "\n2 Channels: " + channels;
    return info;
}
