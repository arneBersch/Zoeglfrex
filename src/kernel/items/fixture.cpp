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
    if (label.isEmpty()) {
        return Item::name() + model->channels + " (" + QString::number(address) + ")";
    }
    return Item::name();
}
