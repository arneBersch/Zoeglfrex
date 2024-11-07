#include "group.h"

Group::Group() {}

QString Group::name() {
    if (label.isEmpty()) {
        if (fixtures.length() == 1) {
            return Item::name() + fixtures[0]->name();
        }
    }
    return Item::name();
}
