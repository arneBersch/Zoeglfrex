#include "item.h"

Item::Item() {}

QString Item::name() {
    return id + " " + label;
}
