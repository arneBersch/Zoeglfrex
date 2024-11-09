#include "fixture.h"

Fixture::Fixture() {}

QString Fixture::name() {
    if (label.isEmpty()) {
        return Item::name() + model->channels;
    }
    return Item::name();
}

Fixture* Fixture::copy() {
    Fixture* fixture = new Fixture();
    fixture->id = id;
    fixture->label = label;
    fixture->model = model;
    fixture->address = address;
    return fixture;
}
