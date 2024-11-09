#ifndef MODEL_H
#define MODEL_H

#include <QtWidgets>

#include "item.h"

class Model : public Item {
public:
    Model(Kernel* core);
    ~Model();
    QString channels;
    QString name() override;
    Model* copy() override;
};

#endif // MODEL_H
