#include "group.h"

Group::Group() {}

QString Group::name() {
    if (label.isEmpty()) {
        QString response = Item::name();
        for (Fixture* fixture : fixtures) {
            response += fixture->name() + "; ";
        }
        return response;
    }
    return Item::name();
}
