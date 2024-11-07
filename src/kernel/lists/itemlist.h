#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QtWidgets>

class Kernel;
#include "kernel/items/item.h"

template <class T> class ItemList : public QAbstractTableModel {
public:
    ItemList();
    virtual T* getItem(QString id) const = 0;
    virtual int getItemRow(QString id) const = 0;
    virtual QList<QString> getIds() const = 0;
    virtual bool copyItems(QList<QString> ids, QString targetId) = 0;
    virtual bool deleteItems(QList<QString> ids) = 0;
    bool labelItems(QList<QString> ids, QString label);
    virtual bool moveItems(QList<QString> ids, QString targetId) = 0;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, const int role) const override;
    bool greaterId(QString firstId, QString secondId);
    Kernel *kernel;
    QList<T*> items;
};

#include "kernel/items/model.h"
template class ItemList<Model>;
#include "kernel/items/fixture.h"
template class ItemList<Fixture>;
#include "kernel/items/group.h"
template class ItemList<Group>;
#include "kernel/items/intensity.h"
template class ItemList<Intensity>;
#include "kernel/items/color.h"
template class ItemList<Color>;
#include "kernel/items/transition.h"
template class ItemList<Transition>;
#include "kernel/items/cue.h"
template class ItemList<Cue>;

#endif // ITEMLIST_H
