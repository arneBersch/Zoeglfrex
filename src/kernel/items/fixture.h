#ifndef FIXTURE_H
#define FIXTURE_H

#include <QtWidgets>

#include "item.h"
#include "model.h"

class Fixture : public Item {
public:
    Fixture(Kernel* core);
    Fixture(const Fixture* item);
    ~Fixture();
    Model* model = nullptr;
    int address = 0;
    QString name() override;
};

#endif // FIXTURE_H
