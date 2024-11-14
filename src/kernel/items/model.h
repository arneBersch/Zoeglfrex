#ifndef MODEL_H
#define MODEL_H

#include <QtWidgets>

#include "item.h"

class Model : public Item {
public:
    Model(Kernel* core);
    Model(const Model* item);
    ~Model();
    QString channels = "D";
    QString name() override;
    QString info() override;
};

#endif // MODEL_H
