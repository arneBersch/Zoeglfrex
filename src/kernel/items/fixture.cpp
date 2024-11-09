#include "fixture.h"
#include "kernel/kernel.h"

Fixture::Fixture(Kernel* core) : Item(core) {}

Fixture::~Fixture() {
    kernel->groups->deleteFixture(this);
}

QString Fixture::name() {
    if (label.isEmpty()) {
        return Item::name() + model->channels;
    }
    return Item::name();
}

Fixture* Fixture::copy() {
    Fixture* fixture = new Fixture(kernel);
    fixture->id = id;
    fixture->label = label;
    fixture->model = model;
    fixture->address = 0;
    return fixture;
}
