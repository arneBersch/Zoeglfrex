#ifndef ITEM_H
#define ITEM_H

#include <QtWidgets>

class Item
{
public:
    Item();
    QString id;
    QString label;
    virtual QString name();
    virtual Item* copy() = 0;
};

#endif // ITEM_H
