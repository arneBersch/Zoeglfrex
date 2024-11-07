#include "fixture.h"

Fixture::Fixture() {}

QString Fixture::name() {
    if (label.isEmpty()) {
        return Item::name() + model->name();
    }
    return Item::name();
}
