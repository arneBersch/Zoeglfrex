#ifndef GROUP_H
#define GROUP_H

#include <QtWidgets>

#include "item.h"
#include "fixture.h"

class Group : public Item {
public:
    Group(Kernel* core);
    Group(const Group* item);
    ~Group();
    QList<Fixture*> fixtures = QList<Fixture*>();
    QString name() override;
    QString info() override;
};

#endif // GROUP_H
