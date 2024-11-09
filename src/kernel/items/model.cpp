#include "model.h"
#include "kernel/kernel.h"

Model::Model(Kernel* core) : Item(core) {}

Model::~Model() {
    kernel->fixtures->deleteModel(this);
}

QString Model::name() {
    if (label.isEmpty()) {
        return Item::name() + channels;
    }
    return Item::name();
}

Model* Model::copy() {
    Model* model = new Model(kernel);
    model->id = id;
    model->label = label;
    model->channels = channels;
    return model;
}
