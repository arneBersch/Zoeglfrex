#ifndef FIXTURE_H
#define FIXTURE_H

#include <QtWidgets>

#include "item.h"
#include "model.h"

class Fixture : public Item {
public:
    Fixture();
    Model* model;
    int address;
    QString name() override;
    Fixture* copy() override;
};

#endif // FIXTURE_H
