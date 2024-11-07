#ifndef MODEL_H
#define MODEL_H

#include <QtWidgets>

#include "item.h"

class Model : public Item {
public:
    Model();
    QString channels;
    QString name() override;
};

#endif // MODEL_H
