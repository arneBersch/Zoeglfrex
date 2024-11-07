#include "model.h"

Model::Model() {}

QString Model::name() {
    if (label.isEmpty()) {
        return Item::name() + channels;
    }
    return Item::name();
}
