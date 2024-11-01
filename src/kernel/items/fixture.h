#ifndef FIXTURE_H
#define FIXTURE_H

#include <QtWidgets>

#include "model.h"

class Fixture {
public:
    Fixture();
    QString id;
    QString label;
    Model* model;
    int address;
};

#endif // FIXTURE_H
