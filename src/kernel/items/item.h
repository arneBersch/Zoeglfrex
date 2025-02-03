#ifndef ITEM_H
#define ITEM_H

#include <QtWidgets>

class Kernel;

class Item
{
public:
    Item(Kernel* core);
    virtual ~Item();
    QString id;
    QString label = QString();
    virtual QString name();
    virtual QString info();
protected:
    Kernel *kernel;
};

#endif // ITEM_H
