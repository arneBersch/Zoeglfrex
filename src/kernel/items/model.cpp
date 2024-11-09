#include "model.h"

Model::Model() {}

QString Model::name() {
    if (label.isEmpty()) {
        return Item::name() + channels;
    }
    return Item::name();
}

Model* Model::copy() {
    Model* model = new Model();
    model->id = id;
    model->label = label;
    model->channels = channels;
    return model;
}
