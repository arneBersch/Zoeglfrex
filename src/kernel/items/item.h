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
};

#endif // ITEM_H
