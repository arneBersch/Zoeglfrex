#ifndef GROUP_H
#define GROUP_H

#include <QtWidgets>

#include "item.h"
#include "fixture.h"

class Group : public Item {
public:
    Group();
    QSet<Fixture*> fixtures;
};

#endif // GROUP_H
