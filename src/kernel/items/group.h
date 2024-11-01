#ifndef GROUP_H
#define GROUP_H

#include <QtWidgets>

#include "fixture.h"

class Group {
public:
    Group();
    QString id;
    QString label;
    QSet<Fixture*> fixtures;
};

#endif // GROUP_H
