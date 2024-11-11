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
    QString label = QString();
    virtual QString name();
    QString idString();
};

#endif // ITEM_H
