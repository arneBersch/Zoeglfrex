#ifndef GROUP_H
#define GROUP_H

#include <QtWidgets>

#include "item.h"
#include "fixture.h"

class Group : public Item {
public:
    Group();
    QList<Fixture*> fixtures;
    QString name() override;
};

#endif // GROUP_H
