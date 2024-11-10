#ifndef ITEM_H
#define ITEM_H

#include <QtWidgets>

class Kernel;

class Item
{
public:
    Item(Kernel* core);
    virtual ~Item();
    Kernel *kernel;
    QString id;
    QString label;
    virtual QString name();
};

#endif // ITEM_H
